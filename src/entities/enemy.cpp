#include <grend/geometryGeneration.hpp>
#include <grend/gameEditor.hpp>

#include <components/health.hpp>
#include <components/healthbar.hpp>
#include <entities/projectile.hpp>
#include "enemy.hpp"

static channelBuffers_ptr sfx = nullptr;
static uint32_t counter = 0;

enemy::~enemy() {};

enemy::enemy(entityManager *manager, gameMain *game, glm::vec3 position)
	: entity(manager)
{
	static gameObject::ptr enemyModel = nullptr;

	TRS transform = node->getTransformTRS();
	transform.position = position;
	node->setTransform(transform);

	new health(manager, this);
	new worldHealthbar(manager, this);
	new projectileCollision(manager, this);
	new syncRigidBodyXZVelocity(manager, this);
	//auto body = new rigidBodySphere(manager, this, transform.position, 1.0, 0.5);
	auto body = new rigidBodyCapsule(manager, this, transform.position, 1.0, 1.0, 2.0);

	manager->registerComponent(this, "enemy", this);

	// TODO:
	if (!enemyModel) {
		//enemyModel = loadSceneAsyncCompiled(manager->engine, DEMO_PREFIX "assets/obj/test-enemy.glb");
		//enemyModel = loadSceneCompiled(DEMO_PREFIX "assets/obj/test-enemy.glb");
		enemyModel = loadSceneCompiled(DEMO_PREFIX "assets/obj/noodler.glb");
		//enemyModel = loadSceneAsyncCompiled(manager->engine, DEMO_PREFIX "assets/obj/ld48/enemy-cube.glb");
		//enemyModel = loadSceneCompiled(DEMO_PREFIX "assets/obj/ld48/enemy-cube.glb");
		//enemyModel->transform.scale = glm::vec3(0.25);
		sfx = openAudio(DEMO_PREFIX "assets/sfx/meh/emeny.wav.ogg");
	}

	setNode("model", node, enemyModel);
	body->registerCollisionQueue(manager->collisions);
	body->phys->setAngularFactor(0.0);

	xxxid = counter++;

	//lastSound = 100*node->id;
}

enemy::enemy(entityManager *manager, entity *ent, nlohmann::json properties)
	: entity(manager, properties)
{
	static gameObject::ptr enemyModel = nullptr;

	new health(manager, this);
	new worldHealthbar(manager, this);
	new projectileCollision(manager, this);
	new syncRigidBodyXZVelocity(manager, this);
	auto body = new rigidBodyCapsule(manager, this, node->getTransformTRS().position, 1.0, 1.0, 2.0);
	/*
	auto body = new rigidBodySphere(manager, this,
	                                node->getTransformTRS().position,
	                                1.0, 0.5);
									*/

	manager->registerComponent(this, "enemy", this);

	// TODO:
	if (!enemyModel) {
		enemyModel = loadSceneAsyncCompiled(manager->engine, DEMO_PREFIX "assets/obj/test-enemy.glb");

		TRS transform = enemyModel->getTransformTRS();
		transform.scale = glm::vec3(0.25);
		enemyModel->setTransform(transform);
	}

	setNode("model", node, enemyModel);
	body->registerCollisionQueue(manager->collisions);
	body->phys->setAngularFactor(0.0);
}

void enemy::update(entityManager *manager, float delta) {
	glm::vec3 playerPos;

	entity *playerEnt =
		findNearest(manager, node->getTransformTRS().position, {"player"});

	if (playerEnt) {
		playerPos = playerEnt->getNode()->getTransformTRS().position;
	}

	// TODO: should this be a component, a generic chase implementation?
	glm::vec3 diff = playerPos - node->getTransformTRS().position;
	glm::vec3 vel =  glm::normalize(glm::vec3(diff.x, 0, diff.z));


	rigidBody *body = castEntityComponent<rigidBody*>(manager, this, "rigidBody");

	if (body) {
		body->phys->setAcceleration(10.f*vel);
	}

	uint32_t k = SDL_GetTicks();
	if (k - lastSound > 3000 + 50*xxxid) {
		auto ch = std::make_shared<spatialAudioChannel>(sfx);
		ch->worldPosition = node->getTransformTRS().position;
		manager->engine->audio->add(ch);
		lastSound = k;
	}
}

nlohmann::json enemy::serialize(entityManager *manager) {
	return entity::serialize(manager);
}
