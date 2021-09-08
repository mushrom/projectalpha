#include <grend/geometryGeneration.hpp>
#include <grend/gameEditor.hpp>

#include <components/health.hpp>
#include <components/healthbar.hpp>
#include <entities/projectile.hpp>
#include "enemy.hpp"

static channelBuffers_ptr sfx = nullptr;
static uint32_t counter = 0;

// TODO: seriously for real just do a resource manager already
static std::map<std::string, gameObject::ptr> enemyModels;

enemy::~enemy() {};
noodler::~noodler() {};
bat::~bat() {};

enemy::enemy(entityManager *manager,
		gameMain *game,
		glm::vec3 position,
		std::string modelPath,
		float radius,
		float height,
		float mass)
	: entity(manager)
{
	gameObject::ptr model = nullptr;

	TRS transform = node->getTransformTRS();
	transform.position = position;
	node->setTransform(transform);

	new health(manager, this);
	new worldHealthbar(manager, this);
	new projectileCollision(manager, this);
	new syncRigidBodyXZVelocity(manager, this);
	auto body = new rigidBodyCapsule(manager, this, transform.position, mass, radius, height);

	manager->registerComponent(this, "enemy", this);
	manager->registerComponent(this, "updatable", this);

	auto it = enemyModels.find(modelPath);

	// TODO:
	if (it == enemyModels.end()) {
		auto [data, _] = loadSceneAsyncCompiled(manager->engine, modelPath);

		model = data;
		enemyModels.insert({modelPath, data});

	} else {
		model = it->second;
	}

	if (!sfx) {
		sfx = openAudio(DEMO_PREFIX "assets/sfx/mnstr7.ogg");
	}

	setNode("model", node, model);
	body->registerCollisionQueue(manager->collisions);
	body->phys->setAngularFactor(0.0);

	xxxid = counter++;

	//lastSound = 100*node->id;
}

// TODO: sync this constructor with the above
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
		//enemyModel = loadSceneAsyncCompiled(manager->engine, DEMO_PREFIX "assets/obj/test-enemy.glb");
		auto [data, _] = loadSceneAsyncCompiled(manager->engine, DEMO_PREFIX "assets/obj/noodler.glb");
		enemyModel = data;
		sfx = openAudio(DEMO_PREFIX "assets/sfx/mnstr7.ogg");

		TRS transform = enemyModel->getTransformTRS();
		transform.scale = glm::vec3(0.25);
		enemyModel->setTransform(transform);
	}

	setNode("model", node, enemyModel);
	body->registerCollisionQueue(manager->collisions);
	body->phys->setAngularFactor(0.0);
}

#include <logic/projalphaView.hpp>
void enemy::update(entityManager *manager, float delta) {
	glm::vec3 playerPos;
	glm::vec3 selfPos = node->getTransformTRS().position;

	entity *playerEnt =
		findNearest(manager, node->getTransformTRS().position, {"player"});

	if (playerEnt) {
		playerPos = playerEnt->getNode()->getTransformTRS().position;
	}

	/*
	// TODO: should this be a component, a generic chase implementation?
	//       wrapping things in generic "behavior" components could be pretty handy...
	glm::vec3 diff = playerPos - node->getTransformTRS().position;
	glm::vec3 vel =  glm::normalize(glm::vec3(diff.x, 0, diff.z));


	rigidBody *body = castEntityComponent<rigidBody*>(manager, this, "rigidBody");

	if (body) {
		body->phys->setAcceleration(10.f*vel);
	}
	*/
	rigidBody *body = castEntityComponent<rigidBody*>(manager, this, "rigidBody");
	health *hp = castEntityComponent<health*>(manager, this, "health");

	if (!body || !hp) {
		SDL_Log("No body/health!");
		return;
	}

	// BIG XXX
	auto v = std::dynamic_pointer_cast<projalphaView>(manager->engine->view);

	if (!v) {
		//std::cerr << "No view!" << std::endl;
		SDL_Log("enemy::update(): No view!");
		return;
	}

	// STILL XXX
	auto wfcgen = v->getGenerator();
	if (!wfcgen) {
		return;
	}

	gameObject::ptr wfcroot = wfcgen->getNode()->getNode("nodes");

	if (!wfcroot) {
		return;
	}

	if (hp->amount < 0.5) {
		// flee
		glm::vec3 dir = wfcgen->pathfindAway(selfPos, playerPos);
		body->phys->setAcceleration(10.f*dir);

	} else if (hp->amount < 1.0 || glm::distance(selfPos, playerPos) < 12.f) {
		glm::vec3 dir = wfcgen->pathfindDirection(selfPos, playerPos);
		body->phys->setAcceleration(10.f*dir);
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
