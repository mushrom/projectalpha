#include <grend/interpolation.hpp>

#include "UI.hpp"
#include <components/inputHandler.hpp>
#include <components/healthbar.hpp>
#include <entities/player.hpp>

#include <nuklear/canvas.h>

// virtual destructors for rtti
uiComponent::~uiComponent() {};
dialogPrompt::~dialogPrompt() {};
uiSystem::~uiSystem() {};

void drawPlayerHealthbar(entityManager *manager,
                         struct nk_context *nk_ctx,
                         health *playerHealth)
{
	static float slowAmount = 0.0;
	static float fastAmount = 0.0;
	fastAmount = interp::average(playerHealth->amount, fastAmount,  4.f, 1.f/60);
	slowAmount = interp::average(fastAmount,           slowAmount, 32.f, 1.f/60);
	float diff = slowAmount - fastAmount;

	int wx = manager->engine->rend->screen_x;
	int wy = manager->engine->rend->screen_y;

	struct nk_canvas canvas;
	if (nk_canvas_begin(nk_ctx, &canvas, "player healthbar", 0, 90, 48, 256, 16,
	                    nk_rgba(32, 32, 32, 127)))
	{
		float k = 252*fastAmount;
		struct nk_rect rect  = nk_rect(93, 50, k, 12);
		struct nk_rect extra = nk_rect(93 + k, 50, (252 - k)*diff, 12);

		nk_fill_rect(canvas.painter, rect, 0, nk_rgb(192, 64, 32));

		if (diff > 0.001) {
			float k = 252*fastAmount;

			//nk_fill_rect(canvas.painter, extra, 0, nk_rgb(160, 84, 48));
			nk_fill_rect(canvas.painter, extra, 0, nk_rgb(48, 85, 160));
		}
	}
	nk_canvas_end(nk_ctx, &canvas);

	if (nk_begin(nk_ctx, "Inventory info", nk_rect(90, 72, 192, 32), 0)) {
		nk_layout_row_dynamic(nk_ctx, 14, 1);
		nk_label(nk_ctx, "[Tab/Start] Open inventory", NK_TEXT_LEFT);
	}
	nk_end(nk_ctx);

	if (nk_begin(nk_ctx, "FPS", nk_rect(wx - 144, 48, 112, 32), 0)) {
		double fps = manager->engine->frame_timer.average();
		std::string fpsstr = std::to_string(fps) + "fps";

		nk_layout_row_dynamic(nk_ctx, 14, 1);
		nk_label(nk_ctx, fpsstr.c_str(), NK_TEXT_LEFT);
	}
	nk_end(nk_ctx);
}

	/*
void renderObjectives(entityManager *manager,
                      levelController *level,
                      vecGUI& vgui)
{
	int wx = manager->engine->rend->screen_x;
	int wy = manager->engine->rend->screen_y;

	nvgBeginPath(vgui.nvg);
	nvgRoundedRect(vgui.nvg, wx - 250, 50, 200, 100, 10);
	nvgFillColor(vgui.nvg, nvgRGBA(28, 30, 34, 192));
	nvgFill(vgui.nvg);

	nvgFontSize(vgui.nvg, 16.f);
	nvgFontFace(vgui.nvg, "sans-bold");
	nvgFontBlur(vgui.nvg, 0);
	nvgTextAlign(vgui.nvg, NVG_ALIGN_LEFT);
	nvgFillColor(vgui.nvg, nvgRGBA(0xf0, 0x60, 0x60, 160));
	nvgText(vgui.nvg, wx - 82, 80, "❎", NULL);
	nvgFillColor(vgui.nvg, nvgRGBA(220, 220, 220, 160));
	nvgText(vgui.nvg, wx - 235, 80, "💚 Objectives: ", NULL);

	unsigned i = 1;
	for (auto& [desc, completed] : level->objectivesCompleted) {
		std::string foo = (completed? "➡" : "❎") + desc;

		nvgText(vgui.nvg, wx - 235, 80 + (i++ * 16), foo.c_str(), NULL);
	}
}
*/

void renderHealthbars(entityManager *manager,
                      struct nk_context *nk_ctx,
                      camera::ptr cam)
{
	//std::set<entity*> ents = searchEntities(manager, {"health", "healthbar"});
	//std::set<entity*> players = searchEntities(manager, {"player", "health"});
	std::set<entity*> ents = searchEntities(manager, {getTypeName<health>(), getTypeName<healthbar>()});
	std::set<entity*> players = searchEntities(manager, {getTypeName<player>(), getTypeName<health>()});

	for (auto& ent : ents) {
		healthbar *bar = getComponent<healthbar>(manager, ent);
		//healthbar *bar;
		//castEntityComponent(bar, manager, ent, "healthbar");

		if (bar) {
			bar->draw(manager, ent, nk_ctx, cam);
		}
	}

	if (players.size() > 0) {
		entity *ent = *players.begin();
		health *playerHealth = getComponent<health>(manager, ent);
		//health *playerHealth;

		//castEntityComponent(playerHealth, manager, ent, "health");

		if (playerHealth) {
			drawPlayerHealthbar(manager, nk_ctx, playerHealth);
		}
	}
}

// XXX
void dialogPrompt::onEvent(entityManager *manager, entity *ent, entity *other) {
	//isActive = true;
}

bool dialogPrompt::active(entityManager *manager, entity *ent) {
	if (!ent || !ent->active) {
		return false;
	}

	// inherited from areaEvent
	// TODO: some way to specify onScreen, currentlyInside, etc
	//return currentlyInside;
	//bool isActive;
	entity *playerEnt = findFirst<player>(manager);

	if (!playerEnt || !playerEnt->active) {
		return false;
	}

	glm::vec3 ownPos = ent->node->getTransformTRS().position;
	glm::vec3 playerPos = playerEnt->node->getTransformTRS().position;

	return glm::distance(ownPos, playerPos) < 2.f;
}

void dialogPrompt::draw(entityManager *manager,
                        entity *ent,
                        struct nk_context *nk_ctx,
                        camera::ptr cam)
{
	glm::vec3 pos = ent->node->getTransformTRS().position;
	glm::vec4 screenpos = cam->worldToScreenPosition(pos);

	if (cam->onScreen(screenpos)) {
		std::string name = "dialog" + std::to_string(pos.x) + std::to_string(pos.y) + std::to_string(pos.z);

		screenpos.y = 1.0 - screenpos.y;
		screenpos.x *= manager->engine->rend->screen_x;
		screenpos.y *= manager->engine->rend->screen_y;

		int width = /*XXX: not fixed-width font*/8*(prompt.size());
		if (nk_begin(nk_ctx, name.c_str(), nk_rect(screenpos.x, screenpos.y, width, 32), 0)) {
			nk_layout_row_dynamic(nk_ctx, 14, 1);
			nk_label(nk_ctx, prompt.c_str(), NK_TEXT_LEFT);
		}
		nk_end(nk_ctx);
	}


	// if we're still inside the target area next frame, then this will be
	// true again... bit of a hack, might be a good idea to some sort of level
	// signal component for being inside an area......
	isActive = false;
}

void uiSystem::update(entityManager *manager, float delta) {
	auto components = manager->getComponents<uiComponent>();

	for (auto& cit : components) {
		uiComponent *comp = dynamic_cast<uiComponent*>(cit);
		entity *ent = manager->getEntity(comp);

		if (!comp || !ent) {
			continue;
		}

		comp->update();

		if (comp->active(manager, ent)) {
			comp->draw(manager, ent, nk_ctx, cam);
		}
	}
}

/*
void renderControls(gameMain *game, vecGUI& vgui) {
	// TODO: should have a generic "pad" component
	// assume first instances of these components are the ones we want,
	// since there should only be 1 of each
	auto& movepads   = game->entities->getComponents("touchMovementHandler");
	auto& actionpads = game->entities->getComponents("touchRotationHandler");
	touchMovementHandler *movepad;
	touchRotationHandler *actionpad;

	if (movepads.size() == 0 || actionpads.size() == 0) {
		return;
	}

	movepad   = dynamic_cast<touchMovementHandler*>(*movepads.begin());
	actionpad = dynamic_cast<touchRotationHandler*>(*actionpads.begin());

	// left movement pad
	nvgStrokeWidth(vgui.nvg, 2.0);
	nvgBeginPath(vgui.nvg);
	nvgCircle(vgui.nvg, movepad->center.x + movepad->touchpos.x,
	                    movepad->center.y + movepad->touchpos.y,
	                    movepad->radius / 3.f);
	nvgFillColor(vgui.nvg, nvgRGBA(0x60, 0x60, 0x60, 0x80));
	nvgStrokeColor(vgui.nvg, nvgRGBA(255, 255, 255, 192));
	nvgStroke(vgui.nvg);
	nvgFill(vgui.nvg);

	nvgStrokeWidth(vgui.nvg, 2.0);
	nvgBeginPath(vgui.nvg);
	nvgCircle(vgui.nvg, movepad->center.x, movepad->center.y, movepad->radius);
	nvgStrokeColor(vgui.nvg, nvgRGBA(0x60, 0x60, 0x60, 0x40));
	nvgStroke(vgui.nvg);

	// right action pad
	nvgStrokeWidth(vgui.nvg, 2.0);
	nvgBeginPath(vgui.nvg);
	nvgCircle(vgui.nvg, actionpad->center.x + actionpad->touchpos.x,
	                    actionpad->center.y + actionpad->touchpos.y,
	                    actionpad->radius / 3.f);
	nvgFillColor(vgui.nvg, nvgRGBA(0x60, 0x60, 0x60, 0x80));
	nvgStrokeColor(vgui.nvg, nvgRGBA(255, 255, 255, 192));
	nvgStroke(vgui.nvg);
	nvgFill(vgui.nvg);

	nvgStrokeWidth(vgui.nvg, 2.0);
	nvgBeginPath(vgui.nvg);
	nvgCircle(vgui.nvg, actionpad->center.x, actionpad->center.y, actionpad->radius);
	nvgStrokeColor(vgui.nvg, nvgRGBA(0x60, 0x60, 0x60, 0x40));
	nvgStroke(vgui.nvg);
}
*/
