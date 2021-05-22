#pragma once

#include <grend/gameObject.hpp>
#include <grend/ecs/ecs.hpp>
#include <components/area.hpp>

using namespace grendx;
using namespace grendx::ecs;

class amuletPickup : public entity {
	public:
		amuletPickup(entityManager *manager, gameMain *game, glm::vec3 position);
		virtual ~amuletPickup();
		virtual void update(entityManager *manager, float delta);
};

