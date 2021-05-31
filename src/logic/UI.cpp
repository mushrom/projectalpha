#include "UI.hpp"
#include <components/inputHandler.hpp>
#include <components/healthbar.hpp>
#include <grend/interpolation.hpp>

#include <nuklear/canvas.h>

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
	std::set<entity*> ents = searchEntities(manager, {"health", "healthbar"});
	std::set<entity*> players = searchEntities(manager, {"player", "health"});

	for (auto& ent : ents) {
		healthbar *bar;
		castEntityComponent(bar, manager, ent, "healthbar");

		if (bar) {
			bar->draw(manager, ent, nk_ctx, cam);
		}
	}

	if (players.size() > 0) {
		entity *ent = *players.begin();
		health *playerHealth;

		castEntityComponent(playerHealth, manager, ent, "health");

		if (playerHealth) {
			drawPlayerHealthbar(manager, nk_ctx, playerHealth);
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
