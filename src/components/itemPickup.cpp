#include <components/itemPickup.hpp>
#include <grend/gameEditor.hpp>

itemPickup::~itemPickup() {};
hasItem::~hasItem() {};

void itemPickup::onEvent(entityManager *manager, entity *ent, entity *other) {
	SDL_Log("Picking up an item!");

	manager->remove(other);
	new hasItem(manager, ent, tags);
}
