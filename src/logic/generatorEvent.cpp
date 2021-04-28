#include "generatorEvent.hpp"

// non-pure virtual destructor for rtti
worldGenerator::~worldGenerator() {};

void worldGenerator::setEventQueue(generatorEventQueue::ptr q) {
	eventQueue = q;
}

void worldGenerator::emit(generatorEvent ev) {
	if (eventQueue) {
		eventQueue->emit(ev);
	}
}

