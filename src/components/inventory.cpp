#include <components/inventory.hpp>

inventory::~inventory() {};

void inventory::insert(entityManager *manager, entity *ent) {
	manager->deactivate(ent);
	items[ent->typeString()].insert(ent);
}

entity *inventory::findType(const std::string& type) {
	if (items.find(type) == items.end() || items[type].size() == 0) {
		return nullptr;
	}

	return *items[type].begin();
}

entity *inventory::remove(entityManager *manager, entity *ent) {
	if (!ent || !items.count(ent->typeString())) {
		return ent;
	}

	manager->activate(ent);
	items[ent->typeString()].erase(ent);

	return ent;
}
