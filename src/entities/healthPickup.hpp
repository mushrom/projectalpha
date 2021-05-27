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

		virtual void apply(entityManager *manager, entity *ent) const {
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
		}
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

		virtual void apply(entityManager *manager, entity *ent) const { }

		/*
		virtual void apply(entityManager *manager, entity *ent) const {
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
		}
		*/
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
			std::cerr << "health pickup collision!" << std::endl;
			healthPickup *pickup = dynamic_cast<healthPickup*>(other);

			if (pickup) {
				pickup->apply(manager, ent);
				manager->remove(pickup);
			}
		};
};
