#include <components/inventory.hpp>

inventory::~inventory() {};

void inventory::insert(entityManager *manager, entity *ent) {
	auto& facts = manager->engine->factories;

	if (facts->has(ent->typeString())) {
		items.push_back(ent->serialize(manager));

	} else {
		SDL_Log("Inventory error: Have no serializer for entity type \"%s\"!",
		        ent->typeString());
	}
}

entity *inventory::remove(entityManager *manager, size_t idx) {
	auto& facts = manager->engine->factories;

	if (idx < items.size()) {
		entity *ent = facts->build(manager, items[idx]);
		items.erase(items.begin() + idx);

		return ent;
	}

	return nullptr;
}
