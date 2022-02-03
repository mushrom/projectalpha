#include <entities/items/items.hpp>
#include <entities/items/loot.hpp>

#include <components/actions/action.hpp>
#include <components/actions/Throwable.hpp>
#include <components/actions/Wieldable.hpp>
#include <components/removeBodiesOnWorldCollision.hpp>

#include <entities/player.hpp>

// non-pure destructors for rtti
healthPickup::~healthPickup() {};
healthPickupCollision::~healthPickupCollision() {};
coinPickup::~coinPickup() {};
chestItem::~chestItem() {};
flareItem::~flareItem() {};
explodyBarrel::~explodyBarrel() {};

ammoLoot::~ammoLoot() {};
healthLoot::~healthLoot() {};

chasePlayer::~chasePlayer() {};

chasePlayer::chasePlayer(entityManager *manager, entity *ent)
	: component(manager, ent)
{
	manager->registerComponent(ent, this);
	manager->registerInterface<updatable>(ent, this);
}

void chasePlayer::update(entityManager *manager, float delta) {
	entity *self = manager->getEntity(this);
	glm::vec3 pos = self->node->getTransformTRS().position;

	//entity *p = findNearest(manager, pos, {"player"});
	entity *p = findNearest(manager, pos, {getTypeName<player>()});

	if (p) {
		glm::vec3 playerPos = p->node->getTransformTRS().position;
		glm::vec3 diff = playerPos - pos;
		float dist = glm::length(diff);

		if (dist < 7.0) {
			float f = 7.f / dist;
			glm::vec3 dir = diff / dist;
			glm::vec3 newpos = pos + dir*f*f*delta;

			TRS foo = self->node->getTransformTRS();
			foo.position = newpos;
			self->node->setTransform(foo);
		}
	}
};

