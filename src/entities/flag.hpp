#pragma once

#include <grend/gameObject.hpp>
#include <grend/ecs/ecs.hpp>
#include <components/area.hpp>

using namespace grendx;
using namespace grendx::ecs;

class flag : public entity {
	public:
		flag(entityManager *manager, gameMain *game,
		     glm::vec3 position, std::string color);

		virtual ~flag();
		virtual void update(entityManager *manager, float delta);
};

class flagPickup : public areaEnter {
	public:
		flagPickup(entityManager *manager, entity *ent)
			//: areaEnter(manager, ent, {"flag", "area"})
			: areaEnter(manager, ent, {getTypeName<flag>(), getTypeName<area>()})
		{
			manager->registerComponent(ent, this);
		}

		virtual ~flagPickup();
		virtual void onEvent(entityManager *manager, entity *ent, entity *other);
};
