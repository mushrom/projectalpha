#include <components/itemPickup.hpp>
#include <components/inventory.hpp>
#include <grend/gameEditor.hpp>

pickupAction::~pickupAction() {};
hasItem::~hasItem() {};
pickup::~pickup() {};

void pickupAction::onEvent(entityManager *manager, entity *ent, entity *other) {
	inventory *inv;
	SDL_Log("Picking up an item!");

	castEntityComponent(inv, manager, ent, "inventory");

	if (inv) {
		// XXX: manager->engine
		inv->insert(manager, other);
	}

	manager->remove(other);
	new hasItem(manager, ent, tags);
}


template <class T>
static inline nlohmann::json setSerializedPosition(glm::vec3 position) {
	nlohmann::json asdf = T::defaultProperties();
	asdf["node"]["position"] = { position[0], position[1], position[2] };
	return asdf;
}

pickup::pickup(entityManager *manager, glm::vec3 position)
	: pickup(manager, this, setSerializedPosition<pickup>(position)) {};

pickup::pickup(entityManager *manager, entity *ent, nlohmann::json properties)
	: entity(manager, properties)
{
	new areaSphere(manager, this, 2.f);

	manager->registerComponent(this, "pickup", this);
}
