#include <grend/ecs/rigidBody.hpp>
#include "projectile.hpp"
#include <components/timedLifetime.hpp>

projectile::~projectile() {};
projectileCollision::~projectileCollision() {};
projectileDestruct::~projectileDestruct() {};

projectileCollision::projectileCollision(entityManager *manager,
                                         entity *ent,
                                         nlohmann::json properties)
	//: collisionHandler(manager, ent, {"projectile"})
	: collisionHandler(manager, ent, {getTypeName<projectile>()})
{
	manager->registerComponent(ent, this);
}

nlohmann::json projectileCollision::serialize(entityManager *manager) {
	return {};
}

projectile::projectile(entityManager *manager, gameMain *game, glm::vec3 position)
	: entity(manager)
{
	manager->registerComponent(this, this);

	// TODO: configurable projectile attributes
	//new rigidBodySphere(manager, this, position, 1.0, 0.25);
	new projectileDestruct(manager, this);
	//new syncRigidBodyTransform(manager, this);

	TRS transform = node->getTransformTRS();
	transform.position = position;
	node->setTransform(transform);
}

void projectile::update(entityManager *manager, float delta) {
	// TODO:
}

