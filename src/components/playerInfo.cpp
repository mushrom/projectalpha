#include <components/playerInfo.hpp>
#include <components/inventory.hpp>
#include <entities/items/items.hpp>
#include <logic/globalMessages.hpp>
#include <entities/player.hpp>

playerInfo::~playerInfo() {};
wieldedHandler::~wieldedHandler() {};

playerInfo::stats playerInfo::calcFullStats(void) {
	// TODO: look for (de)buff components on player and wielded items 
	//       that alter stats
	return base;
}

/*
template <typename... U>
std::vector<const char *> typeList() {
	return { getTypeName<U>()... };
}
*/

void wieldedHandler::handleInput(entityManager *manager,
                                 entity *ent,
                                 inputEvent& ev)
{
	// XXX: for now, only handle action events
	if (ev.type != inputEvent::types::primaryAction
	 && ev.type != inputEvent::types::secondaryAction
	 && ev.type != inputEvent::types::tertiaryAction)
	{
		return;
	}

	//entity *playerEnt = findFirst(manager, {"player", "inventory", "playerInfo"});
	entity *playerEnt = findFirstTypes<player, inventory, playerInfo>(manager);
	if (!playerEnt) return;

	auto inv = getComponent<inventory>(manager, playerEnt);
	auto stats = getComponent<playerInfo>(manager, playerEnt);

	//auto inv = castEntityComponent<inventory*>(manager, playerEnt, "inventory");
	//auto stats = castEntityComponent<playerInfo*>(manager, playerEnt, "playerInfo");

	if (!inv || !stats) return;

	if (ev.active) {
		std::cerr << "wieldedHandler::handleInput(): got here" << std::endl;

		entity *prim = inv->findType(stats->primaryWeapon);
		entity *sec  = inv->findType(stats->secondaryWeapon);
		entity *acc  = inv->findType(stats->accessory);

		std::string pstr = (prim? "" : "(empty) ") + stats->primaryWeapon;
		std::string sstr = (sec?  "" : "(empty) ") + stats->secondaryWeapon;
		std::string astr = (acc?  "" : "(empty) ") + stats->accessory;

		auto useItem = [&] (entity *item) {
			Wieldable *w = getComponent<Wieldable>(manager, item);
			//Wieldable *w;
			//castEntityComponent(w, manager, item, "Wieldable");

			if (w) {
				inv->remove(manager, item);
				manager->engine->entities->activate(item);
				w->action(manager, playerEnt);

				Messages()->publish({
					.type = "itemDropped",
					.ent  = item,
					.comp = playerEnt,
				});
			}
		};

		if (ev.type == inputEvent::types::primaryAction && prim) {
			SDL_Log("Primary!");
			useItem(prim);
		}

		if (ev.type == inputEvent::types::secondaryAction && sec) {
			SDL_Log("Secondary!");
			useItem(sec);
		}

		if (ev.type == inputEvent::types::tertiaryAction && acc) {
			SDL_Log("A button!");
			useItem(acc);
		}
	}
}

nlohmann::json wieldedHandler::serialize(entityManager *manager) {
	// TODO: actually serialize
	return defaultProperties();
}
