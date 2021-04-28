#pragma once

#include <grend/gameObject.hpp>
#include <grend/gameMain.hpp>
#include <thread>
#include <mutex>
#include <vector>

using namespace grendx;

struct generatorEvent {
	// event interface for ECS stuff, this way entities can be
	// spawned/despawned/deactivated/etc as the player moves around
	enum types {
		generatorStarted,
		generated,
		deleted,
	} type;

	glm::vec3 position;
	glm::vec3 extent;
};

class generatorEventQueue {
	public:
		typedef std::shared_ptr<generatorEventQueue> ptr;
		typedef std::weak_ptr<generatorEventQueue>   weakptr;

		std::lock_guard<std::mutex> lock(void) {
			return std::lock_guard<std::mutex>(mtx);
		}

		std::vector<generatorEvent>& getQueue(void) {
			return queue;
		}

		void emit(generatorEvent ev) {
			std::lock_guard<std::mutex> g(mtx);
			queue.push_back(ev);
		}

	private:
		std::mutex mtx;
		std::vector<generatorEvent> queue;
};

class worldGenerator {
	public:
		virtual ~worldGenerator();

		virtual gameObject::ptr getNode(void) { return root; };
		virtual void setPosition(gameMain *game, glm::vec3 position) = 0;
		virtual void setEventQueue(generatorEventQueue::ptr q);

	protected:
		// TODO: generic event class
		virtual void emit(generatorEvent ev);

		gameObject::ptr root = std::make_shared<gameObject>();
		glm::vec3 lastPosition = glm::vec3(INT_MAX);
		generatorEventQueue::ptr eventQueue = nullptr;
};
