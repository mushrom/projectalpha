#pragma once

#include <grend/gameObject.hpp>
#include <grend/ecs/ecs.hpp>
#include <components/area.hpp>

using namespace grendx;
using namespace grendx::ecs;

class pickupAction : public areaEnter {
	public:
		pickupAction(entityManager *manager, entity *ent,
		           std::vector<std::string> _tags)
			: areaEnter(manager, ent, _tags)
		{
			manager->registerComponent(ent, "pickupAction", this);
		}

		virtual ~pickupAction();
		virtual void onEvent(entityManager *manager, entity *ent, entity *other);
};

class hasItem : public component {
	public:
		hasItem(entityManager *manager, entity *ent, std::vector<std::string> _tags)
			: component(manager, ent),
			  tags(_tags)
		{
			manager->registerComponent(ent, "hasItem", this);

			for (auto& tag : tags) {
				// XXX: bit of a hack, should probably have a seperate query system
				//      for this... a way to organize indexers and searching
				//      for different component, entity types will be important,
				//      not sure how to approach that yet though
				manager->registerComponent(ent, "hasItem:" + tag, this);
			}
		}

		virtual ~hasItem();
		std::vector<std::string> tags;
};

class pickup : public entity {
	public:
		pickup(entityManager *manager, glm::vec3 position);
		pickup(entityManager *manager, entity *ent, nlohmann::json properties);
		virtual ~pickup();

		virtual void update(entityManager *manager, float delta) {};
};

