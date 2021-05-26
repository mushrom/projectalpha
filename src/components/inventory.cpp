#include <components/inventory.hpp>

inventory::~inventory() {};

void inventory::insert(entityManager *manager, entity *ent) {
	/*
	auto& facts = manager->engine->factories;

	if (facts->has(ent->typeString())) {
		items.push_back(ent->serialize(manager));

	} else {
		SDL_Log("Inventory error: Have no serializer for entity type \"%s\"!",
		        ent->typeString());
	}
	*/
	manager->deactivate(ent);
	items.push_back(ent);
}

entity *inventory::remove(entityManager *manager, size_t idx) {
	/*
	auto& facts = manager->engine->factories;

	if (idx < items.size()) {
		entity *ent = facts->build(manager, items[idx]);
		items.erase(items.begin() + idx);

		return ent;
	}
	*/

	if (idx < items.size()) {
		entity *ent = items[idx];
		// TODO: should the entity need to be explicitly reactivated?
		//       common use for remove() here would be simply deleting an
		//       item after use, and if it's activated here the node will
		//       briefly pop back into the active set...
		//
		//       could have two different methods, remove() and... something
		manager->activate(ent);
		items.erase(items.begin() + idx);

		return ent;
	}

	return nullptr;
}
