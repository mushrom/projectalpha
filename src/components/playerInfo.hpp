#pragma once

#include <grend/gameObject.hpp>
#include <grend/ecs/ecs.hpp>
#include <grend/ecs/message.hpp>

#include <components/inputHandler.hpp>

using namespace grendx;

class playerInfo : public component {
	public:
		playerInfo(entityManager *manager,
		           entity *ent,
		           nlohmann::json properties)
			: component(manager, ent)
		{
			manager->registerComponent(ent, "playerInfo", this);
		}

		struct stats {
			float strength;
			float dexterity;
			float stealth;
			float special;
			float experience;
			int level;
		};

		virtual ~playerInfo();
		stats calcFullStats(void);

		stats base;

		std::string primaryWeapon = "Empty";
		std::string secondaryWeapon = "Empty";
		std::string accessory = "Empty";
};

// calls the Wieldable actions for whatever components are mapped to
// input slots
class wieldedHandler : public inputHandler {
	public:
		// TODO: could set firing rate, bullet amount, etc in properties
		wieldedHandler(entityManager *manager,
		               entity *ent,
		               nlohmann::json properties={})
			: inputHandler(manager, ent)
		{
			manager->registerComponent(ent, "wieldedHandler", this);
		}
		virtual ~wieldedHandler();

		virtual void
		handleInput(entityManager *manager, entity *ent, inputEvent& ev);

		// serialization stuff
		constexpr static const char *serializedType = "wieldedHandler";

		virtual const char *typeString(void) const { return serializedType; };
		virtual nlohmann::json serialize(entityManager *manager);
};
