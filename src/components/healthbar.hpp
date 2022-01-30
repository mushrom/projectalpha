#pragma once

#include <grend/ecs/ecs.hpp>
#include <grend/camera.hpp>

#include <nuklear/nuklear.h>

using namespace grendx;
using namespace grendx::ecs;

class healthbar : public component {
	public:
		healthbar(entityManager *manager, entity *ent)
			: component(manager, ent)
		{
			manager->registerComponent(ent, this);
		};
		virtual ~healthbar();

		virtual void draw(entityManager *manager, entity *ent,
		                  struct nk_context *nk_ctx, camera::ptr cam) = 0;
};

// health bar drawn aligned and scaled with the 3D world position of
// the underlying entity
class worldHealthbar : public healthbar {
	public:
		worldHealthbar(entityManager *manager, entity *ent)
			: healthbar(manager, ent)
		{
			manager->registerComponent(ent, this);
		}

		virtual ~worldHealthbar();

		float lastAmount = 1.0;
		virtual void draw(entityManager *manager, entity *ent,
		                  struct nk_context *nk_ctx, camera::ptr cam);
};
