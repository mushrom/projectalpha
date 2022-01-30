#pragma once

#include <grend/gameObject.hpp>
#include <grend/animation.hpp>
#include <grend/modalSDLInput.hpp>

#include <grend/ecs/ecs.hpp>
#include <grend/ecs/rigidBody.hpp>

using namespace grendx;
using namespace grendx::ecs;

struct inputEvent {
	enum types {
		move,
		look,
		crouch,
		// left, right, middle click
		primaryAction,
		secondaryAction,
		tertiaryAction,
	} type;

	glm::vec3 data;
	bool active;
};

typedef std::shared_ptr<std::vector<inputEvent>> inputQueue;

class inputHandler : public component {
	public:
		inputHandler(entityManager *manager, entity *ent)
			: component(manager, ent)
		{
			manager->registerComponent(ent, this);
		}
		virtual ~inputHandler();

		virtual void
		handleInput(entityManager *manager, entity *ent, inputEvent& ev) {
			std::cerr << "handleInput(): got here" << std::endl;
		}
};

class rawEventHandler : public component {
	public:
		rawEventHandler(entityManager *manager, entity *ent)
			: component(manager, ent)
		{
			manager->registerComponent(ent, this);
		}
		virtual ~rawEventHandler();

		virtual void
		handleEvent(entityManager *manager, entity *ent, SDL_Event& ev) {
			std::cerr << "handleInput(): got here" << std::endl;
		}
};

class inputPoller : public component {
	public:
		inputPoller(entityManager *manager, entity *ent)
			: component(manager, ent)
		{
			manager->registerComponent(ent, this);
		}
		virtual ~inputPoller();

		virtual void update(entityManager *manager, entity *ent) = 0;
};

class inputHandlerSystem : public entitySystem {
	public:
		typedef std::shared_ptr<inputHandlerSystem> ptr;
		typedef std::weak_ptr<inputHandlerSystem>   weakptr;

		virtual ~inputHandlerSystem();

		virtual void update(entityManager *manager, float delta);
		virtual void handleEvent(entityManager *manager, SDL_Event& ev);

		inputQueue inputs = std::make_shared<std::vector<inputEvent>>();
};

class controllable : public component {
	public: 
		controllable(entityManager *manager, entity *ent)
			: component(manager, ent)
		{
			manager->registerComponent(ent, this);
		}
		virtual ~controllable();

		void handleInput(inputEvent& ev);
};

// XXX: having basically tag components, waste of space?
class isControlled : public component {
	public: 
		isControlled(entityManager *manager, entity *ent)
			: component(manager, ent)
		{
			manager->registerComponent(ent, this);
		}
		virtual ~isControlled();
};

class movementHandler : public inputHandler {
	public:
		movementHandler(entityManager *manager, entity *ent)
			: inputHandler(manager, ent)
		{
			manager->registerComponent(ent, this);
		}

		// nothing special for deserializing construction
		movementHandler(entityManager *manager, entity *ent, nlohmann::json _)
			: inputHandler(manager, ent)
		{
			manager->registerComponent(ent, this);
		}
		virtual ~movementHandler();

		virtual void
		handleInput(entityManager *manager, entity *ent, inputEvent& ev) {
			if (ev.type == inputEvent::types::move) {
				rigidBody *body = getComponent<rigidBody>(manager, ent);

				if (body) {
					body->phys->setVelocity(ev.data * 20.f);
					lastvel = ev.data;
				}
			}
		}

		glm::vec3 lastvel;

		// serialization stuff
		constexpr static const char *serializedType = "movementHandler";
		virtual const char *typeString(void) const { return serializedType; };
};

class mouseRotationPoller : public inputPoller {
	public:
		mouseRotationPoller(entityManager *manager, entity *ent, camera::ptr _cam)
			: inputPoller(manager, ent),
			  cam(_cam)
		{
			manager->registerComponent(ent, this);
		}
		virtual ~mouseRotationPoller();

		virtual void update(entityManager *manager, entity *ent);

	private:
		camera::ptr cam;
};

class touchMovementHandler : public rawEventHandler {
	public:
		glm::vec2 touchpos;
		glm::vec2 center;
		float radius;

		touchMovementHandler(entityManager *manager, entity *ent, camera::ptr _cam,
		                     inputQueue q, glm::vec2 _center, float _radius)
			: rawEventHandler(manager, ent),
			  center(_center), radius(_radius), inputs(q), cam(_cam)
		{
			manager->registerComponent(ent, this);
		}
		virtual ~touchMovementHandler();

		virtual void handleEvent(entityManager *manager, entity *ent, SDL_Event& ev);

	private:
		inputQueue inputs;
		camera::ptr cam;
};

class touchRotationHandler : public rawEventHandler {
	public:
		glm::vec2 touchpos;
		glm::vec2 center;
		float radius;

		touchRotationHandler(entityManager *manager, entity *ent, camera::ptr _cam,
		                     inputQueue q, glm::vec2 _center, float _radius)
			: rawEventHandler(manager, ent),
			  center(_center), radius(_radius), inputs(q), cam(_cam)
		{
			manager->registerComponent(ent, this);
		}
		virtual ~touchRotationHandler();

		virtual void handleEvent(entityManager *manager, entity *ent, SDL_Event& ev);

	private:
		inputQueue inputs;
		camera::ptr cam;
};

bindFunc inputMapper(inputQueue q, camera::ptr cam);
bindFunc camMovement2D(inputQueue q, camera::ptr cam, float accel);
bindFunc touchMapper(inputQueue q, camera::ptr cam);
