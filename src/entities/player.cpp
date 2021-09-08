#include <grend/gameEditor.hpp>
#include "player.hpp"
#include <components/playerInfo.hpp>

player::~player() {};

// TODO: only returns one collection, object tree could have any number
animationCollection::ptr findAnimations(gameObject::ptr obj) {
	if (!obj) {
		return nullptr;
	}

	for (auto& chan : obj->animations) {
		return chan->group->collection;
	}

	for (auto& [name, ptr] : obj->nodes) {
		auto ret = findAnimations(ptr);

		if (ret) {
			return ret;
		}
	}

	return nullptr;
}

void animatedCharacter::setAnimation(std::string animation, float weight) {
	if (animations) {
		for (auto& [name, ptr] : animations->groups) {
			if (auto group = ptr.lock()) {
				group->weight = (name == animation)? weight : 0.0;
			}
		}
	}
}

animatedCharacter::animatedCharacter(gameObject::ptr objs) {
	if ((animations = findAnimations(objs)) == nullptr) {
		// TODO: proper error
		throw "asdf";
	}

	objects = objs;
}

gameObject::ptr animatedCharacter::getObject(void) {
	return objects;
}

// TODO: might as well have a resource component
static gameObject::ptr playerModel = nullptr;

player::player(entityManager *manager, gameMain *game, glm::vec3 position)
	: entity(manager)
{

	//new boxSpawner(manager, this);
	new wieldedHandler(manager, this);
	new movementHandler(manager, this);
	new projectileCollision(manager, this);
	new syncRigidBodyPosition(manager, this);
	rigidBody *body = new rigidBodySphere(manager, this, position, 10.0, 0.75);

	manager->registerComponent(this, "player", this);
	manager->registerComponent(this, "updatable", this);

	if (!playerModel) {
		// TODO: resource cache
		//playerModel = loadScene(GR_PREFIX "assets/obj/TestGuy/rigged-lowpolyguy.glb");
		SDL_Log("Loading player model...");
		playerModel = loadSceneCompiled(DEMO_PREFIX "assets/obj/buff-dude-testanim.glb");
		//playerModel = loadSceneCompiled(DEMO_PREFIX "assets/obj/ld48/player-cursor.glb");
		//playerModel = loadSceneCompiled("/home/flux/blender/objects/lowpoly-cc0-guy/low-poly-cc0-guy-fixedimport.gltf");

		TRS transform = playerModel->getTransformTRS();
		//transform.rotation = glm::quat(glm::vec3(0, -M_PI/2, 0));
		//transform.scale = glm::vec3(0.16f);
		//transform.scale = glm::vec3(2.0f);
		transform.position = glm::vec3(0, -0.75, 0);
		playerModel->setTransform(transform);

		assert(playerModel != nullptr);
		SDL_Log("got player model");
	}

	TRS transform = node->getTransformTRS();
	transform.position = position;
	node->setTransform(transform);
	setNode("model", node, playerModel);
	//setNode("light", node, std::make_shared<gameLightPoint>());
	//setNode("light", node, std::make_shared<gameLightPoint>());
	//auto lit = std::make_shared<gameLightPoint>();

	auto lit = std::make_shared<gameLightSpot>();
	lit->setTransform((TRS) {
		.position = glm::vec3(0, 1, 1),
		//.rotation = glm::quat(glm::vec3(0, -M_PI/2, 0)),
	});

	lit->intensity = 125;
	lit->is_static = false;
	lit->casts_shadows = true;
	//lit->angle = cos(glm::degrees(35.f));
	//lit->casts_shadows = false;

	auto plit = std::make_shared<gameLightPoint>();
	//plit->diffuse = glm::vec4(0.0, 0.17, 0.46, 1.0);
	plit->diffuse = glm::vec4(1.0);
	plit->setTransform((TRS) { .position = glm::vec3(0, 1.5, 0), });
	plit->intensity = 50;
	plit->radius = 0.75;
	plit->is_static = false;
	plit->casts_shadows = false;

	setNode("spotlight", node, lit);
	setNode("pointlight", node, plit);
	character = std::make_shared<animatedCharacter>(playerModel);
	//character->setAnimation("idle");

	body->registerCollisionQueue(manager->collisions);
}

player::player(entityManager *manager,
               entity *ent,
               nlohmann::json properties)
	: entity(manager, properties)
{
	manager->registerComponent(this, "player", this);

	if (!playerModel) {
		// TODO: resource cache
		//playerModel = loadScene(GR_PREFIX "assets/obj/TestGuy/rigged-lowpolyguy.glb");
		SDL_Log("Loading player model...");
		playerModel = loadSceneCompiled(DEMO_PREFIX "assets/obj/buff-dude-testanim.glb");

		TRS transform = playerModel->getTransformTRS();
		transform.rotation = glm::quat(glm::vec3(0, -M_PI/2, 0));
		//transform.scale = glm::vec3(0.16f);
		transform.position = glm::vec3(0, -0.5, 0);
		playerModel->setTransform(transform);

		//bindCookedMeshes();
		assert(playerModel != nullptr);
		SDL_Log("got player model");
	}

	setNode("model", node, playerModel);
	//setNode("light", node, std::make_shared<gameLightPoint>());
	//setNode("light", node, std::make_shared<gameLightPoint>());
	//auto lit = std::make_shared<gameLightPoint>();
	auto lit = std::make_shared<gameLightSpot>();
	lit->setTransform((TRS) {
		.position = glm::vec3(0, 0, 1),
		.rotation = glm::quat(glm::vec3(0, -M_PI/2, 0)),
	});

	lit->intensity = 200;
	lit->is_static = false;
	lit->casts_shadows = true;
	setNode("light", node, lit);
	character = std::make_shared<animatedCharacter>(playerModel);
	//character->setAnimation("idle");
}

nlohmann::json player::serialize(entityManager *manager) {
	return entity::serialize(manager);
}

void player::update(entityManager *manager, float delta) {
	rigidBody *body = castEntityComponent<rigidBody*>(manager, this, "rigidBody");
	if (!body) return;

	body->phys->setAngularFactor(0.f);

	glm::vec3 vel = body->phys->getVelocity();
	if (glm::length(vel) < 2.0) {
		character->setAnimation("idle");
	} else {
		character->setAnimation("walking");
	}
}
