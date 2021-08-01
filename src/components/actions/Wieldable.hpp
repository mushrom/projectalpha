#pragma once

#include <grend/ecs/ecs.hpp>
#include <grend/ecs/rigidBody.hpp>

#include <components/actions/action.hpp>

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

