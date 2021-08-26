#pragma once

#include <grend/sdlContext.hpp>

using namespace grendx;

inline SDL_GameController *curController = nullptr;
inline Sint32 controllerJoyID = -1;

static inline
SDL_GameController *Controller(void) {
	return curController;
}

static inline
void initController(void) {
	SDL_Log("Scanning for controllers...");
	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		if (SDL_IsGameController(i)) {
			if ((curController = SDL_GameControllerOpen(i))) {
				const char *name = SDL_GameControllerNameForIndex(i);
				SDL_Log("Opened controller %d: %s", i, name);
				break;
			} else {
				SDL_Log("Couldn't open controller %d!", i);
			}
		}
	}

	if (!curController) {
		SDL_Log("No controller found!");
	}
}

static inline
void updateController(SDL_Event& ev) {
	if (ev.type == SDL_CONTROLLERDEVICEADDED && !curController) {
		curController = SDL_GameControllerOpen(ev.cdevice.which);
		controllerJoyID = ev.cdevice.which;
	}

	if (ev.type == SDL_CONTROLLERDEVICEREMOVED && ev.cdevice.which == controllerJoyID) {
		SDL_GameControllerClose(curController);
		curController = nullptr;
		controllerJoyID = -1;
	}
}
