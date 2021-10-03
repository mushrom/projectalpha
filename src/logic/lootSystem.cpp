#include <grend/geometryGeneration.hpp>
#include <grend/gameEditor.hpp>
#include <stdlib.h>

#include "lootSystem.hpp"
#include <components/boxSpawner.hpp>
#include <components/timedLifetime.hpp>
#include <entities/player.hpp>
#include <entities/items/healthPickup.hpp>
#include <entities/items/loot.hpp>
#include <logic/globalMessages.hpp>

// destructor as key function for rtti
lootSystem::~lootSystem() {};

void lootSystem::onEvent(entityManager *manager, entity *ent, float delta) {
	unsigned drops = rand()%10 + 1;

	for (unsigned i = 0; i < drops; i++) {
		bool ammo = rand()&1; // XXX: true for ammo, false for health
		glm::vec3 pos = ent->node->getTransformTRS().position;

		pos.x += 2.f*sin(1.618 * float(i));
		pos.z += 2.f*cos(1.618 * float(i));

		if (ammo) {
			auto am = new ammoLoot(manager, pos);
			manager->add(am);

			Messages()->publish({
				.type = "itemDropped",
				.ent  = am,
				.comp = ent,
			});

		} else {
			auto am = new healthLoot(manager, pos);
			manager->add(am);

			Messages()->publish({
				.type = "itemDropped",
				.ent  = am,
				.comp = ent,
			});

		}
	}
}
