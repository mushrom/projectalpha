#pragma once

#include <grend/gameObject.hpp>
#include <grend/animation.hpp>
#include <grend/modalSDLInput.hpp>

#include <grend/ecs/ecs.hpp>
#include <grend/ecs/rigidBody.hpp>
#include <grend/ecs/collision.hpp>

using namespace grendx;
using namespace grendx::ecs;

class enemy : public entity, public updatable {
	public:
		//enemy(entityManager *manager, gameMain *game, glm::vec3 position);
		enemy(entityManager *manager,
		      gameMain *game,
		      glm::vec3 position,
		      std::string modelpath,
		      float radius,
		      float height,
		      float mass);
		enemy(entityManager *manager, entity *ent, nlohmann::json properties);

		virtual ~enemy();
		virtual void update(entityManager *manager, float delta);
		virtual gameObject::ptr getNode(void) { return node; };

		// serialization stuff
		constexpr static const char *serializedType = "enemy";
		static const nlohmann::json defaultProperties(void) {
			return entity::defaultProperties();
		}

		virtual const char *typeString(void) const { return serializedType; };
		virtual nlohmann::json serialize(entityManager *manager); 

		uint32_t lastSound = 0;
		uint32_t xxxid = 0;
};

class noodler : public enemy /* #1 */ {
	public:
		noodler(entityManager *manager, gameMain *game, glm::vec3 position)
			: enemy(manager, game, position, DEMO_PREFIX "assets/obj/noodler.glb",
			        1.0, 2.0, 1.0) {};

		virtual ~noodler();
};

class bat : public enemy {
	public:
		bat(entityManager *manager, gameMain *game, glm::vec3 position)
			: enemy(manager, game, position, DEMO_PREFIX "assets/obj/bat-test.glb",
			        1.0, 2.5, 0.5) {};

		virtual ~bat();
};
