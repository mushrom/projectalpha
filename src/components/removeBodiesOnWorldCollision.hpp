#pragma once

#include <grend/ecs/ecs.hpp>
#include <grend/ecs/collision.hpp>

#include <components/actions/action.hpp>

class removeBodiesOnWorldCollision : public collisionHandler {
	public:
		removeBodiesOnWorldCollision(entityManager *manager, entity *ent)
			: collisionHandler(manager, ent, {})
		{
			manager->registerComponent(ent, this);
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

			auto bodies = comps.equal_range(getTypeName<rigidBody>());
			auto syncs = comps.equal_range(getTypeName<syncRigidBody>());
			auto rms = comps.equal_range(getTypeName<removeBodiesOnWorldCollision>());

			//auto bodies = comps.equal_range("rigidBody");
			//auto syncs = comps.equal_range("syncRigidBody");
			//auto rms = comps.equal_range("removeBodiesOnWorldCollision");

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

