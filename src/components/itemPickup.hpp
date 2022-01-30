#pragma once

#include <grend/gameObject.hpp>
#include <grend/ecs/ecs.hpp>
#include <grend/ecs/message.hpp>

#include <components/area.hpp>
#include <logic/globalMessages.hpp>

using namespace grendx;
using namespace grendx::ecs;

class pickupAction : public areaInside {
	public:
		pickupAction(entityManager *manager,
		             entity *ent,
		             std::vector<const char *> _tags)
			: areaInside(manager, ent, _tags)
		{
			manager->registerComponent(ent, this);
			Messages()->publish({
				.type = "actionCreated",
				.ent  = ent,
				.comp = this,
			});
		}

		virtual ~pickupAction();
		virtual void onEvent(entityManager *manager, entity *ent, entity *other);
};

class autopickupAction : public areaInside {
	public:
		autopickupAction(entityManager *manager,
		                 entity *ent,
		                 std::vector<const char *> _tags)
			: areaInside(manager, ent, _tags)
		{
			manager->registerComponent(ent, this);
			Messages()->publish({
				.type = "actionCreated",
				.ent  = ent,
				.comp = this,
			});
		}

		virtual ~autopickupAction();
		virtual void onEvent(entityManager *manager, entity *ent, entity *other);
};

// TODO: remove this
class hasItem : public component {
	public:
		hasItem(entityManager *manager, entity *ent, std::vector<const char *> _tags)
			: component(manager, ent),
			  tags(_tags)
		{
			manager->registerComponent(ent, this);

			/*
			for (auto& tag : tags) {
				// XXX: bit of a hack, should probably have a seperate query system
				//      for this... a way to organize indexers and searching
				//      for different component, entity types will be important,
				//      not sure how to approach that yet though
				//manager->registerComponent(ent, "hasItem:" + tag, this);
			}
			*/
		}

		virtual ~hasItem();
		std::vector<const char *> tags;
};

class pickup : public entity {
	public:
		pickup(entityManager *manager, glm::vec3 position);
		pickup(entityManager *manager, entity *ent, nlohmann::json properties);
		virtual ~pickup();
};

class autopickup : public entity {
	public:
		autopickup(entityManager *manager, glm::vec3 position);
		autopickup(entityManager *manager, entity *ent, nlohmann::json properties);
		virtual ~autopickup();

		virtual void onEvent(entityManager *manager, entity *ent, entity *other);
};

