#pragma once

#include <grend/gameObject.hpp>
#include <grend/animation.hpp>
#include <grend/gameEditor.hpp>
#include <grend/ecs/ecs.hpp>
#include <grend/ecs/collision.hpp>
#include <grend/ecs/rigidBody.hpp>

#include <components/health.hpp>
#include <components/itemPickup.hpp>

#include <components/explosionParticles.hpp>

#include <components/actions/Wieldable.hpp>
#include <components/actions/Throwable.hpp>

#include <entities/projectile.hpp>

using namespace grendx;
using namespace grendx::ecs;

class explodyBarrel : public pickup {
	public:
		constexpr static const char *serializedType = "explodyBarrel";

		// TODO: creating a json object just for initialization might not
		//       be the best for performance...
		//       might be worth looking into other strategies for serialization,
		//       but this is probably the simplest
		explodyBarrel(entityManager *manager, glm::vec3 position)
			: explodyBarrel(manager, this,
			                setSerializedPosition<chestItem>(position)) {};

		explodyBarrel(entityManager *manager, entity *ent, nlohmann::json properties)
			: pickup(manager, ent, properties)
		{
			AABBExtent box = {
				.center = {0.0, 0.5, 0.0},
				.extent = {0.275, 0.45, 0.275},
			};

			new health(manager, this);
			new projectileCollision(manager, this);
			new syncRigidBodyTransform(manager, this);
			new explosionParticles(manager, this);

			rigidBody *body = new rigidBodyCylinder(manager, this, node->getTransformTRS().position, 5.0, box);

			manager->registerComponent(this, this);
			body->registerCollisionQueue(manager->collisions);

			static gameObject::ptr model = nullptr;
			// XXX: really need resource manager
			if (model == nullptr) {
				model = loadSceneCompiled(DEMO_PREFIX "assets/obj/Barrel_01/Barrel_01_2k.gltf");

				TRS transform = model->getTransformTRS();
				transform.position -= glm::vec3(0, 0.45, 0);
				model->setTransform(transform);
			}

			{
				//TRS transform = model->getTransformTRS();
				//transform.scale = glm::vec3(2.0);
				//model->setTransform(transform);
			}

			setNode("model", node, model);

			gameLightPoint::ptr lit = std::make_shared<gameLightPoint>();

			lit->diffuse = glm::vec4(1.0, 0.7, 0.4, 1); // orangish
			lit->intensity = 50;
			lit->setTransform({ .position = {0, 1, 0} });
			setNode("light", node, lit);
		}

		virtual ~explodyBarrel();
		virtual const char *typeString() const { return serializedType; };
};

