#pragma once

#include <grend/ecs/ecs.hpp>
#include <grend/ecs/rigidBody.hpp>

#include <components/actions/action.hpp>
// TODO: less enterprise name
#include <components/removeBodiesOnWorldCollision.hpp>

class Throwable : public Action {
	public:
		Throwable(entityManager *manager, entity *ent)
			: Action(manager, ent)
		{
			manager->registerComponent(ent, this);
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
