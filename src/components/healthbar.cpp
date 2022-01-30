#include "healthbar.hpp"
#include "health.hpp"
#include <grend/interpolation.hpp>

#include <nuklear/canvas.h>

using namespace grendx;
using namespace grendx::ecs;

// non-virtual destructors for rtti
healthbar::~healthbar() {};
worldHealthbar::~worldHealthbar() {};

void worldHealthbar::draw(entityManager *manager, entity *ent,
                          struct nk_context *nk_ctx, camera::ptr cam)
{
	health *entHealth = getComponent<health>(manager, ent);
	// TODO: maybe an error message or something here
	if (!entHealth)
		return;

	float ticks = SDL_GetTicks() / 1000.f;
	glm::vec3 entpos = ent->getNode()->getTransformTRS().position + glm::vec3(0, 3, 0);
	glm::vec4 screenpos = cam->worldToScreenPosition(entpos);

	if (entHealth->amount < 1.0 && cam->onScreen(screenpos)) {
		// TODO: some sort of grid editor wouldn't be too hard,
		//       probably worthwhile for quick UIs
		float depth = 16*max(0.f, screenpos.w);
		float pad = depth*8.f;

		float width  = 8*pad;
		float height = 3*pad;

		screenpos.y  = 1.0 - screenpos.y;
		screenpos.x *= manager->engine->rend->screen_x;
		screenpos.y *= manager->engine->rend->screen_y;

		glm::vec2 outer    = glm::vec2(screenpos) - glm::vec2(width, height)*0.5f;
		glm::vec2 innermin = outer + pad;
		glm::vec2 innermax = outer + glm::vec2(width, height) - 2*pad;

		std::string name = std::string("enemy healthbar:");
		name += std::to_string(ent->node->id);

		struct nk_canvas canvas;
		if (nk_canvas_begin(nk_ctx, &canvas, name.c_str(), 0,
		                    outer.x, outer.y, width, height,
		                    nk_rgba(32, 32, 32, 127)));
		{
			lastAmount = interp::average(entHealth->amount, lastAmount, 16.f, 1.f/60);
			struct nk_rect rect = nk_rect(innermin.x, innermin.y,
			                              lastAmount*(width - 2*pad), pad);
			struct nk_rect extra = nk_rect(
				innermin.x + lastAmount*(width - 2*pad), innermin.y,
				(1.f - lastAmount)*(width - 2*pad), pad
			);

			nk_fill_rect(canvas.painter, rect, 0, nk_rgb(48,  192, 48));
			nk_fill_rect(canvas.painter, extra, 0, nk_rgb(160, 48,  48));
		}
		nk_canvas_end(nk_ctx, &canvas);
	}
}
