#pragma once

#include <grend/gameObject.hpp>
#include <grend/animation.hpp>
#include <grend/gameEditor.hpp>
#include <grend/ecs/ecs.hpp>
#include <grend/ecs/collision.hpp>
#include <grend/ecs/rigidBody.hpp>

#include <components/health.hpp>
#include <components/itemPickup.hpp>

using namespace grendx;
using namespace grendx::ecs;

template <class T>
static inline nlohmann::json setSerializedPosition(glm::vec3 position) {
	nlohmann::json asdf = T::defaultProperties();
	asdf["node"]["position"] = { position[0], position[1], position[2] };
	return asdf;
}

class Action /*lawsuit :O*/ : public component {
	public:
		Action(entityManager *manager, entity *ent)
			: component(manager, ent)
		{
			manager->registerComponent(ent, "Action", this);
		};

		virtual ~Action();
		virtual void action(entityManager *manager, entity *ent) const = 0;
		// TODO: maybe a tester to see if the entity can currently do this action
};

class Consumable : public Action {
	public:
		Consumable(entityManager *manager, entity *ent)
			: Action(manager, ent)
		{
			manager->registerComponent(ent, "Consumable", this);
		}

		virtual ~Consumable();
};

class removeBodiesOnWorldCollision : public collisionHandler {
	public:
		removeBodiesOnWorldCollision(entityManager *manager, entity *ent)
			: collisionHandler(manager, ent, {})
		{
			manager->registerComponent(ent, "removeBodiesOnWorldCollision", this);
		}
		virtual ~removeBodiesOnWorldCollision();

		virtual void
		onCollision(entityManager *manager, entity *ent,
		            entity *other, collision& col)
		{
			if (other != nullptr) {
				// world bodies have no attached entity, for now
				return;
			}

			if (glm::dot(col.normal, glm::vec3(0, -1, 0)) < 0.9) {
				return;
			}

			SDL_Log("Got here, at removeBodiesOnWorldCollision::action()!");

			auto comps = manager->getEntityComponents(ent);
			auto bodies = comps.equal_range("rigidBody");
			auto syncs = comps.equal_range("syncRigidBody");
			auto rms = comps.equal_range("removeBodiesOnWorldCollision");

			std::set<component*> toRemove;

			for (auto& range : {bodies, syncs, rms}) {
				for (auto it = range.first; it != range.second; it++) {
					auto& [_, comp] = *it;
					toRemove.insert(comp);
				}
			}

			for (auto& comp : toRemove) {
				manager->unregisterComponent(ent, comp);
			}
		};
};

class Throwable : public Action {
	public:
		Throwable(entityManager *manager, entity *ent)
			: Action(manager, ent)
		{
			manager->registerComponent(ent, "Throwable", this);
		}
		virtual ~Throwable();

		virtual void action(entityManager *manager, entity *ent) const {
			entity *self = manager->getEntity((component*)this);

			TRS transform = ent->node->getTransformTRS();
			glm::vec3 dir;
			dir = glm::mat3_cast(transform.rotation) * glm::vec3(1, 0, 0);
			dir = glm::normalize(dir + glm::vec3(0, 1, 0));

			glm::vec3 pos = transform.position + dir*3.f;

			self->node->setTransform({
				.position = pos,
				.rotation = transform.rotation,
			});

			rigidBody *body = new rigidBodySphere(manager, self, pos, 5.0, 0.5);
			new syncRigidBodyPosition(manager, self);
			new removeBodiesOnWorldCollision(manager, self);
			// TODO: component to remove rigid bodies after a collision, or some
			//       number of collisions

			body->phys->setVelocity(dir*10.f);
			body->registerCollisionQueue(manager->collisions);
		}
};

class Wieldable : public Action {
	// name of the action component to use when this thing is wielded
	std::string wieldAction;

	public:
		Wieldable(entityManager *manager, entity *ent, std::string action)
			: Action(manager, ent),
			  wieldAction(action)
		{
			manager->registerComponent(ent, "Wieldable", this);
		}
		virtual ~Wieldable();

		virtual void action(entityManager *manager, entity *ent) const {
			SDL_Log("Got here, at Wieldable::action()!");
			entity *self = manager->getEntity((component*)this);
			Action *act = castEntityComponent<Action*>(manager, self, wieldAction);

			if (act) {
				SDL_Log("Wieldable::action() calling subaction %s...", wieldAction.c_str());
				act->action(manager, ent);

			} else {
				SDL_Log("Wieldable::action(): couldn't find action %s", wieldAction.c_str());
			}
		}
};

class healingConsumable : public Consumable {
	float heals;

	public:
		healingConsumable(entityManager *manager, entity *ent, float amount = 30.f)
			: Consumable(manager, ent),
			  heals(amount)
		{
			manager->registerComponent(ent, "healingConsumable", this);
		}
		virtual ~healingConsumable();

		virtual void action(entityManager *manager, entity *ent) const {
			SDL_Log("Got here, at healingConsumable::action()!");
			//health *enthealth = manager->getEnt
			auto comps = manager->getEntityComponents(ent);
			auto range = comps.equal_range("health");

			for (auto it = range.first; it != range.second; it++) {
				auto& [key, comp] = *it;
				health *entHealth = dynamic_cast<health*>(comp);

				if (entHealth) {
					entHealth->heal(heals);
				}
			}

			// single-use consumable
			entity *self = manager->getEntity((component*)this);
			manager->remove(self);
		}
};

class healthPickup : public pickup {
	float heals = 30.f;

	public:
		constexpr static const char *serializedType = "healthPickup";

		// TODO: creating a json object just for initialization might not
		//       be the best for performance...
		//       might be worth looking into other strategies for serialization,
		//       but this is probably the simplest
		healthPickup(entityManager *manager, glm::vec3 position)
			: healthPickup(manager, this,
			               setSerializedPosition<healthPickup>(position)) {};

		healthPickup(entityManager *manager, entity *ent, nlohmann::json properties)
			: pickup(manager, ent, properties)
		{
			gameLightPoint::ptr lit = std::make_shared<gameLightPoint>();

			new healingConsumable(manager, ent);
			new Wieldable(manager, ent, "healingConsumable");
			manager->registerComponent(this, "healthPickup", this);

			static gameModel::ptr model = nullptr;
			// XXX: really need resource manager
			if (model == nullptr) {
				model = load_object(GR_PREFIX "assets/obj/smoothsphere.obj");
				compileModel("healthmodel", model);
				bindCookedMeshes();
			}

			lit->diffuse = glm::vec4(1, 0, 0, 1); // red

			{
				TRS transform = model->getTransformTRS();
				transform.scale = glm::vec3(0.5);
				model->setTransform(transform);
			}

			setNode("model", node, model);
			setNode("light", node, lit);
		}

		virtual ~healthPickup();
		virtual const char *typeString() const { return serializedType; };
};

class coinPickup : public pickup {
	public:
		constexpr static const char *serializedType = "coinPickup";

		// TODO: creating a json object just for initialization might not
		//       be the best for performance...
		//       might be worth looking into other strategies for serialization,
		//       but this is probably the simplest
		coinPickup(entityManager *manager, glm::vec3 position)
			: coinPickup(manager, this,
			             setSerializedPosition<coinPickup>(position)) {};

		coinPickup(entityManager *manager, entity *ent, nlohmann::json properties)
			: pickup(manager, ent, properties)
		{
			new Throwable(manager, ent);
			new Wieldable(manager, ent, "Throwable");
			manager->registerComponent(this, "coinPickup", this);

			static gameObject::ptr model = nullptr;
			// XXX: really need resource manager
			if (model == nullptr) {
				model = loadSceneCompiled(DEMO_PREFIX "assets/obj/coins.glb");
			}

			{
				TRS transform = model->getTransformTRS();
				transform.scale = glm::vec3(2.0);
				model->setTransform(transform);
			}

			setNode("model", node, model);

			gameLightPoint::ptr lit = std::make_shared<gameLightPoint>();

			lit->diffuse = glm::vec4(0.8, 0.8, 0.5, 1); // yellowish
			lit->setTransform({ .position = {0, 1, 0} });
			setNode("light", node, lit);
		}

		virtual ~coinPickup();
		virtual const char *typeString() const { return serializedType; };
};

class flareItem : public pickup {
	public:
		constexpr static const char *serializedType = "flareItem";

		// TODO: creating a json object just for initialization might not
		//       be the best for performance...
		//       might be worth looking into other strategies for serialization,
		//       but this is probably the simplest
		flareItem(entityManager *manager, glm::vec3 position)
			: flareItem(manager, this,
			             setSerializedPosition<flareItem>(position)) {};

		flareItem(entityManager *manager, entity *ent, nlohmann::json properties)
			: pickup(manager, ent, properties)
		{
			new Throwable(manager, ent);
			new Wieldable(manager, ent, "Throwable");
			manager->registerComponent(this, "flareItem", this);

			static gameObject::ptr model = nullptr;
			// XXX: really need resource manager
			if (model == nullptr) {
				model = loadSceneCompiled(DEMO_PREFIX "assets/obj/flare.glb");
			}

			{
				TRS transform = model->getTransformTRS();
				transform.scale = glm::vec3(2.0);
				model->setTransform(transform);
			}

			setNode("model", node, model);

			gameLightPoint::ptr lit = std::make_shared<gameLightPoint>();

			lit->diffuse = glm::vec4(1.0, 0.3, 0.1, 1); // bright red
			lit->intensity = 150;
			lit->setTransform({ .position = {0, 1, 0} });
			setNode("light", node, lit);
		}

		virtual ~flareItem();
		virtual const char *typeString() const { return serializedType; };
};

class healthPickupCollision : public collisionHandler {
	float damage;
	float lastCollision = 0;

	public:
		healthPickupCollision(entityManager *manager, entity *ent, float _damage = 2.5f)
			: collisionHandler(manager, ent, {"healthPickup"})
		{
			damage = _damage;
			manager->registerComponent(ent, "healthPickupCollision", this);
		}
		virtual ~healthPickupCollision();

		virtual void
		onCollision(entityManager *manager, entity *ent,
		            entity *other, collision& col)
		{
			/*
			std::cerr << "health pickup collision!" << std::endl;
			healthPickup *pickup = dynamic_cast<healthPickup*>(other);

			if (pickup) {
				pickup->apply(manager, ent);
				manager->remove(pickup);
			}
			*/
		};
};
