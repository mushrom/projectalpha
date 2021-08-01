#include <logic/tests/tests.hpp>

using namespace grendx;
using namespace grendx::ecs;

bool tests::defaultTest(gameMain *game, projalphaView::ptr view) {
	SDL_Log("Default tester!");
	game->running = true;
	//view->load(game, mapfile);
	view->level->reset();
	view->input.setMode(projalphaView::modes::Loading);

	for (unsigned i = 0; i < 256; i++) {
		if (!game->running) {
			SDL_Log("ERROR: game stopped running!");
			return false;
		}

		try {
			game->step();

		} catch (const std::exception& ex) {
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Exception! %s", ex.what());
			return false;

		} catch (const char* ex) {
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Exception! %s", ex);
			return false;
		}
	}

	return true;
}
