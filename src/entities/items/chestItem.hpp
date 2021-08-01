#pragma once

#include <grend/gameObject.hpp>
#include <grend/animation.hpp>
#include <grend/gameEditor.hpp>
#include <grend/ecs/ecs.hpp>
#include <grend/ecs/collision.hpp>
#include <grend/ecs/rigidBody.hpp>

#include <components/health.hpp>
#include <components/itemPickup.hpp>

#include <components/actions/Wieldable.hpp>
#include <components/actions/Throwable.hpp>

using namespace grendx;
using namespace grendx::ecs;

class chestItem : public pickup {
	public:
		constexpr static const char *serializedType = "chestItem";

		// TODO: creating a json object just for initialization might not
		//       be the best for performance...
		//       might be worth looking into other strategies for serialization,
		//       but this is probably the simplest
		chestItem(entityManager *manager, glm::vec3 position)
			: chestItem(manager, this,
			             setSerializedPosition<chestItem>(position)) {};

		chestItem(entityManager *manager, entity *ent, nlohmann::json properties)
			: pickup(manager, ent, properties)
		{
			manager->registerComponent(this, "chestItem", this);

			static gameObject::ptr model = nullptr;
			// XXX: really need resource manager
			if (model == nullptr) {
				model = loadSceneCompiled(DEMO_PREFIX "assets/obj/treasure-chest.glb");
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

		virtual ~chestItem();
		virtual const char *typeString() const { return serializedType; };
};

