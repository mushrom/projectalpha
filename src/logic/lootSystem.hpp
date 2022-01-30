#pragma once

#include <grend/gameObject.hpp>
#include <grend/ecs/ecs.hpp>

using namespace grendx;
using namespace grendx::ecs;

class lootSystem : public entityEventSystem {
	public:
		lootSystem(std::vector<const char *> _tags)
			: entityEventSystem(_tags) {};

		virtual ~lootSystem();
		virtual void onEvent(entityManager *manager, entity *ent, float delta);
};
