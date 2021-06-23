#include <components/inventory.hpp>

inventory::~inventory() {};

void inventory::insert(entityManager *manager, entity *ent) {
	manager->deactivate(ent);
	items[ent->typeString()].insert(ent);
}

entity *inventory::remove(entityManager *manager, entity *ent) {
	if (!ent || !items.count(ent->typeString())) {
		return ent;
	}

	manager->activate(ent);
	items[ent->typeString()].erase(ent);

	return ent;
}
