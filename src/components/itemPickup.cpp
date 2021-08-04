#include <components/itemPickup.hpp>
#include <components/inventory.hpp>
#include <utility/serializer.hpp>
#include <grend/gameEditor.hpp>

#include <logic/UI.hpp>

pickupAction::~pickupAction() {};
hasItem::~hasItem() {};
pickup::~pickup() {};

void pickupAction::onEvent(entityManager *manager, entity *ent, entity *other) {
	inventory *inv;
	castEntityComponent(inv, manager, ent, "inventory");

	if (!inv) {
		return;
	}

	const uint8_t *keystates = SDL_GetKeyboardState(NULL);

	// big-ish XXX: ideally input events should all be dispatched through handlers,
	//              but that would be much messier... being able to poll for state
	//              here is conceptually much easier to work with
	if (keystates[SDL_SCANCODE_X]) {
		// XXX: manager->engine
		SDL_Log("Picking up an item!");
		inv->insert(manager, other);
		//manager->remove(other);
		new hasItem(manager, ent, tags);

		Messages()->publish({
			.type = "itemPickedUp",
			.ent  = other,
			.comp = ent,
			// TODO: tag with level
		});
	}
}

pickup::pickup(entityManager *manager, glm::vec3 position)
	: pickup(manager, this, setSerializedPosition<pickup>(position)) {};

pickup::pickup(entityManager *manager, entity *ent, nlohmann::json properties)
	: entity(manager, properties)
{
	new areaSphere(manager, this, 2.f);
	new dialogPrompt(manager, this, "[X] Pick up the item here");

	manager->registerComponent(this, "pickup", this);
}
