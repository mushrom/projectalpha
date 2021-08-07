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
	manager->registerComponent(this, "updatable", this);

	// TODO:
	if (!enemyModel) {
		//enemyModel = loadSceneAsyncCompiled(manager->engine, DEMO_PREFIX "assets/obj/test-enemy.glb");
		//enemyModel = loadSceneCompiled(DEMO_PREFIX "assets/obj/test-enemy.glb");
		//enemyModel = loadSceneCompiled(DEMO_PREFIX "assets/obj/noodler.glb");
		// TODO: resource manager
		auto [data, _] = loadSceneAsyncCompiled(manager->engine, DEMO_PREFIX "assets/obj/noodler.glb");
		enemyModel = data;
		sfx = openAudio(DEMO_PREFIX "assets/sfx/mnstr7.ogg");
		//enemyModel = loadSceneAsyncCompiled(manager->engine, DEMO_PREFIX "assets/obj/ld48/enemy-cube.glb");
		//enemyModel = loadSceneCompiled(DEMO_PREFIX "assets/obj/ld48/enemy-cube.glb");
		//enemyModel->transform.scale = glm::vec3(0.25);
		//sfx = openAudio(DEMO_PREFIX "assets/sfx/meh/emeny.wav.ogg");
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
		std::cerr << "No body/health!" << std::endl;
		return;
	}

#if 0
	std::pair<int, int> playerTile = {
		int(playerPos.x/4 + 0.5),
		int(playerPos.z/4 + 0.5)
	};

	std::pair<int, int> currentTile = {
		int(selfPos.x/4 + 0.5),
		int(selfPos.z/4 + 0.5)
	};
#endif

	// BIG XXX
	auto v = std::dynamic_pointer_cast<projalphaView>(manager->engine->view);

	if (!v) {
		std::cerr << "No view!" << std::endl;
	}

	if (v) {
		//auto& wfcgen = v->wfcgen;
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

		} else if (hp->amount < 1.0) {
			glm::vec3 dir = wfcgen->pathfindDirection(selfPos, playerPos);
			body->phys->setAcceleration(10.f*dir);
		}
#if 0
		using Coord = std::pair<int, int>;

		Coord bestDir = {0, 0};
		float score = HUGE_VALF;
		int fleeing = hp->amount <= 0.5;
		wfcGenerator::Array* tilemap = nullptr;
		wfcGenerator::Array* farmap = nullptr;

		/*
		if (fleeing) {
			auto mit = wfcgen->maxDistances.find(playerTile);
			if (mit == wfcgen->maxDistances.end()) {
				// no map
				std::cerr << "No map!" << std::endl;
				return;
			}

			auto it = wfcgen->omnidijkstra.find(mit->second);
			if (it == wfcgen->omnidijkstra.end()) {
				// also no map
				std::cerr << "No map!" << std::endl;
				return;
			}

			tilemap = &it->second;

		} else
		*/
		{
			auto mit = wfcgen->maxDistances.find(playerTile);
			if (mit == wfcgen->maxDistances.end()) {
				// no map
				std::cerr << "No map!" << std::endl;
				return;
			}

			auto fit = wfcgen->omnidijkstra.find(mit->second);
			if (fit == wfcgen->omnidijkstra.end()) {
				// also no map
				std::cerr << "No map!" << std::endl;
				return;
			}

			farmap = &fit->second;

			auto it = wfcgen->omnidijkstra.find(playerTile);
			if (it == wfcgen->omnidijkstra.end()) {
				// no map
				std::cerr << "No map!" << std::endl;
				return;
			}

			tilemap = &it->second;
		}

		float dist = tilemap->get(currentTile);

		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				if (x == 0 && y == 0) continue;

				Coord c = {currentTile.first + x, currentTile.second + y};
				float dist = tilemap->valid(c)? tilemap->get(c) : HUGE_VALF;
				float fdist = farmap->valid(c)? farmap->get(c) : HUGE_VALF;

				if (dist < HUGE_VALF) {
					if (abs(x) == abs(y)) {
						Coord adjx = {currentTile.first + x, currentTile.second};
						Coord adjy = {currentTile.first, currentTile.second + y};
						float m = max(tilemap->get(adjx), tilemap->get(adjy));

						if (m == HUGE_VALF) {
							// avoid hugging (and possibly getting stuck
							// on) corners
							continue;
						}
					}

					if (fleeing && fdist < HUGE_VALF) {
						dist *= (fleeing)? -1.414 : 1.0;
						dist += fdist;
					}

					if (dist < score) {
						score = dist;
						bestDir = {x, y};
					}
				}
			}
		}

		if (hp->amount < 1.0 && bestDir != Coord {0, 0}) {
			//glm::vec3 vel = glm::normalize(glm::vec3((lowest.first, 0, lowest.second)));
			glm::vec2 dir = glm::normalize(glm::vec2(bestDir.first, bestDir.second));
			glm::vec3 vel(dir.x, 0, dir.y);
			body->phys->setAcceleration(10.f*vel);
		}
#endif

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
