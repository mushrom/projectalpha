#pragma once

#include <grend/gameObject.hpp>
#include <grend/animation.hpp>
#include <grend/gameEditor.hpp>
#include <grend/ecs/ecs.hpp>
#include <grend/ecs/collision.hpp>
#include <grend/ecs/rigidBody.hpp>

#include <components/health.hpp>
#include <components/inventory.hpp>
#include <components/itemPickup.hpp>
#include <components/boxSpawner.hpp>

class chasePlayer : public component, public updatable {
	public:
		chasePlayer(entityManager *manager, entity *ent);
		virtual ~chasePlayer();
		virtual void update(entityManager *manager, float delta);
};

class ammoLoot : public autopickup {
	public:
		ammoLoot(entityManager *manager, glm::vec3 position)
			: autopickup(manager, position)
		{
			manager->registerComponent(this, this);

			new chasePlayer(manager, this);

			// TODO: resource manager
			static gameObject::ptr model = nullptr;

			if (!model) {
				model = loadSceneCompiled(DEMO_PREFIX "assets/obj/emissive-cube.glb");

				TRS transform = model->getTransformTRS();
				transform.scale = glm::vec3(0.25);
				model->setTransform(transform);
			}


			gameLightPoint::ptr lit = std::make_shared<gameLightPoint>();
			lit->diffuse = glm::vec4(0.2, 0.2, 1.0, 1.0);
			lit->intensity = 200;
			lit->radius = 0.2;

			setNode("light", node, lit);
			setNode("model", node, model);
		}

		virtual void onEvent(entityManager *manager, entity *ent, entity *other) {
			inventory *inv = getComponent<inventory>(manager, other);

			if (!inv) {
				return;
			}

			// 20 bullets
			for (int i = 0; i < 5; i++) {
				entity *bullet = new boxBullet(manager, manager->engine, glm::vec3(0));
				manager->add(bullet);
				inv->insert(manager, bullet);
			}
		}

		virtual ~ammoLoot();
};

class healthLoot : public autopickup {
	public:
		healthLoot(entityManager *manager, glm::vec3 position)
			: autopickup(manager, position)
		{
			manager->registerComponent(this, this);

			new chasePlayer(manager, this);

			// TODO: resource manager
			static gameObject::ptr model = nullptr;

			if (!model) {
				model = loadSceneCompiled(DEMO_PREFIX "assets/obj/emissive-cube.glb");

				TRS transform = model->getTransformTRS();
				transform.scale = glm::vec3(0.25);
				model->setTransform(transform);
			}

			gameLightPoint::ptr lit = std::make_shared<gameLightPoint>();
			lit->diffuse = glm::vec4(1.0, 0.2, 0.2, 1.0);
			lit->intensity = 200;
			lit->radius = 0.2;

			setNode("light", node, lit);
			setNode("model", node, model);
		}

		virtual void onEvent(entityManager *manager, entity *ent, entity *other) {
			health *hp = getComponent<health>(manager, other);

			if (hp) {
				hp->heal(10);
			}
		}

		virtual ~healthLoot();
};
