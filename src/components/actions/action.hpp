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

class Action /*lawsuit :O*/ : public component {
	public:
		Action(entityManager *manager, entity *ent)
			: component(manager, ent)
		{
			manager->registerComponent(ent, this);
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
			manager->registerComponent(ent, this);
		}

		virtual ~Consumable();
};

class healingConsumable : public Consumable {
	float heals;

	public:
		healingConsumable(entityManager *manager, entity *ent, float amount = 30.f)
			: Consumable(manager, ent),
			  heals(amount)
		{
			manager->registerComponent(ent, this);
		}
		virtual ~healingConsumable();

		virtual void action(entityManager *manager, entity *ent) const {
			SDL_Log("Got here, at healingConsumable::action()!");
			//health *enthealth = manager->getEnt
			auto comps = manager->getEntityComponents(ent);
			//auto range = comps.equal_range("health");
			auto range = comps.equal_range(getTypeName<health>());

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
