#include <grend/gameMain.hpp>
#include <grend/gameMainDevWindow.hpp>
#include <grend/gameObject.hpp>
//#include <grend/playerView.hpp>
#include <grend/geometryGeneration.hpp>
#include <grend/gameEditor.hpp>
#include <grend/controllers.hpp>

#include <grend/ecs/ecs.hpp>
#include <grend/ecs/rigidBody.hpp>
#include <grend/ecs/collision.hpp>
#include <grend/ecs/serializer.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>

#include <memory>
#include <chrono>
#include <map>
#include <vector>
#include <set>
#include <functional>

#include <initializer_list>

// XXX:  toggle using textures/models I have locally, don't want to
//       bloat the assets folder again
#define LOCAL_BUILD 0

using namespace grendx;
using namespace grendx::ecs;

// TODO: should include less stuff
#include <components/inputHandler.hpp>
#include <components/health.hpp>
#include <components/healthbar.hpp>
#include <components/timedLifetime.hpp>
#include <components/team.hpp>
#include <components/itemPickup.hpp>
#include <components/inventory.hpp>
#include <components/playerInfo.hpp>

#include <entities/player.hpp>
#include <entities/enemy.hpp>
#include <entities/projectile.hpp>
#include <entities/enemyCollision.hpp>
#include <entities/items/items.hpp>
#include <entities/flag.hpp>
#include <entities/enemySpawner.hpp>
#include <entities/killedParticles.hpp>
#include <entities/targetArea.hpp>
#include <entities/amulet.hpp>

#include <logic/projalphaView.hpp>
#include <logic/wfcGenerator.hpp>
#include <logic/UI.hpp>
#include <logic/levelController.hpp>

#include <nuklear/nuklear.h>

// TODO: move to utilities header
struct grendDirEnt {
	std::string name;
	bool isFile;
};

static std::vector<grendDirEnt> listdir(std::string path) {
	std::vector<grendDirEnt> ret;

// XXX: older debian raspi is built on doesn't include c++17 filesystem functions,
//      so leave the old posix code in for that... aaaaa
#if defined(_WIN32)
	if (fs::exists(currentDir) && fs::is_directory(path)) {
		for (auto& p : fs::directory_iterator(currentDir)) {
			ret.push_back({
				p.path().filename(),
				!fs::is_directory(p.path())
			});
		}

	} else {
		SDL_Log("listdir: Invalid directory %s", path.c_str());
	}

#else
	DIR *dirp;

	if ((dirp = opendir(path.c_str()))) {
		struct dirent *dent;

		while ((dent = readdir(dirp))) {
			ret.push_back({
				std::string(dent->d_name),
				dent->d_type != DT_DIR
			});
		}
	}
#endif

	return ret;
}

void initEntitiesFromNodes(gameObject::ptr node,
                           std::function<void(const std::string&, gameObject::ptr&)> init)
{
	if (node == nullptr) {
		return;
	}

	for (auto& [name, ptr] : node->nodes) {
		init(name, ptr);
	}
}

#if defined(_WIN32)
extern "C" {
//int WinMain(int argc, char *argv[]);
int WinMain(void);
}

int WinMain(void) try {
	int argc = 1;
	const char *argv[] = {"asdf"};
#else
int main(int argc, char *argv[]) try {
#endif
	const char *mapfile = DEMO_PREFIX "assets/maps/level-test.map";

	if (argc > 1) {
		mapfile = argv[1];
	}

	SDL_Log("entering main()");
	SDL_Log("started SDL context");
	SDL_Log("have game state");

	// include editor in debug builds, use main game view for release
#if defined(GAME_BUILD_DEBUG)
	gameMain *game = new gameMainDevWindow();
#else
	gameMain *game = new gameMain();
#endif

	/*
	game->jobs->addAsync([=] {
		auto foo = openSpatialLoop(GR_PREFIX "assets/sfx/Bit Bit Loop.ogg");
		foo->worldPosition = glm::vec3(-10, 0, -5);
		game->audio->add(foo);
		return true;
	});
	*/

	game->jobs->addAsync([=] {
		auto hum = openAudio(DEMO_PREFIX "assets/sfx/cave themeb4.ogg");
		auto water = openAudio(DEMO_PREFIX "assets/sfx/atmosbasement.mp3_.ogg");

		for (auto& sfx : {hum, water}) {
			for (int i = 0; i < 5; i++) {
				auto bar = std::make_shared<spatialAudioChannel>(sfx);
				glm::vec3 r(rand() / (float)RAND_MAX, 0, rand() / (float)RAND_MAX);
				bar->worldPosition = glm::vec3(4*32.f * r.x, 2.f, 4*32.f * r.z);
				bar->loopMode = audioChannel::mode::Loop;
				game->audio->add(bar);
			}
		}

		//auto bar = openSpatialLoop(GR_PREFIX "assets/sfx/Meditating Beat.ogg");
		//auto bar = openSpatialLoop(DEMO_PREFIX "assets/sfx/cave themeb4.ogg");
		//bar->worldPosition = glm::vec3(34, 0, 34);
		//game->audio->add(bar);
		return true;
	});

	projalphaView::ptr view = std::make_shared<projalphaView>(game);
	view->cam->setFar(1000.0);
	view->cam->setFovx(70.0);
	game->setView(view);
	game->rend->lightThreshold = 0.2;

	view->level->addInit([=] () {
		//view->wfcgen->generate(game, {});
	});

	view->level->addInit([=] () {
		entity *playerEnt;
		glm::vec3 pos(-5, 20, -5);

		/*
		auto floor = view->getFloor(game, view->currentFloor);
		if (floor) {
			pos = floor->entrance;
		}
		*/

		playerEnt = new player(game->entities.get(), game, pos);
		game->entities->add(playerEnt);
		//new generatorEventHandler(game->entities.get(), playerEnt);
		new health(game->entities.get(), playerEnt);
		new enemyCollision(game->entities.get(), playerEnt);
		new healthPickupCollision(game->entities.get(), playerEnt);
		new flagPickup(game->entities.get(), playerEnt);
		new team(game->entities.get(), playerEnt, "blue");
		new areaAddScore(game->entities.get(), playerEnt, {});
		new playerInfo(game->entities.get(), playerEnt, {});
		inventory *inv = new inventory(game->entities.get(), playerEnt, {});

		// start with 5 flares
		for (int i = 0; i < 5; i++) {
			entity *flare = new flareItem(game->entities.get(), glm::vec3(0));
			game->entities->add(flare);
			inv->insert(game->entities.get(), flare);
		}

		// 20 bullets
		for (int i = 0; i < 50; i++) {
			entity *bullet = new boxBullet(game->entities.get(), game, glm::vec3(0));
			game->entities->add(bullet);
			inv->insert(game->entities.get(), bullet);
		}

		new pickupAction(game->entities.get(), playerEnt, {"amuletPickup"});
		new pickupAction(game->entities.get(), playerEnt, {"pickup"});

#if defined(__ANDROID__)
		int wx = game->rend->screen_x;
		int wy = game->rend->screen_y;
		glm::vec2 movepad  ( 2*wx/16.f, 7*wy/9.f);
		glm::vec2 actionpad(14*wx/16.f, 7*wy/9.f);

		new touchMovementHandler(game->entities.get(), playerEnt, cam,
								 view->inputSystem->inputs, movepad, 150.f);
		new touchRotationHandler(game->entities.get(), playerEnt, cam,
								 view->inputSystem->inputs, actionpad, 150.f);

#else
		new mouseRotationPoller(game->entities.get(), playerEnt, view->cam);
#endif
	});

	view->level->addInit([=] () {
		view->currentFloor = -1;
		view->incrementFloor(game, 1);
	});

	view->level->addDestructor([=] () {
		// TODO: should just have reset function in entity manager
		for (auto& ent : game->entities->entities) {
			game->entities->remove(ent);
		}

		game->entities->clearFreedEntities();
		view->floors.clear();
	});

	view->level->addObjective("Reach exit",
		[=] () {
			auto players
				= searchEntities(game->entities.get(), {"player", "hasItem:amuletPickup"});
			// TODO: check for goal item (amulet?) and current floor == 0 (exit)
			return view->currentFloor == -1 && players.size() != 0;
		});

	view->level->addLoseCondition(
		[=] () {
			std::set<entity*> players
				= searchEntities(game->entities.get(), {"player"});

			return std::pair<bool, std::string>(players.size() == 0, "lol u died");
		});

	SDL_Log("Got to game->run()! mapfile: %s\n", mapfile);
	//view->load(game, mapfile);
	auto mapdata = loadMapCompiled(game, mapfile);
	game->state->rootnode = mapdata;
	//setNode("asdf", game->state->rootnode, mapdata);
	setNode("entities", game->state->rootnode, game->entities->root);

	std::vector<physicsObject::ptr> mapPhysics;
	game->phys->addStaticModels(nullptr,
								mapdata,
								TRS(),
								mapPhysics);

	if (char *target = getenv("GREND_TEST_TARGET")) {
		SDL_Log("Got a test target!");

		if (strcmp(target, "default") == 0) {
			SDL_Log("Default tester!");
			game->running = true;
			view->load(game, mapfile);
			view->input.setMode(projalphaView::modes::Move);

			for (unsigned i = 0; i < 256; i++) {
				if (!game->running) {
					SDL_Log("ERROR: game stopped running!");
					return 1;
				}

				try {
					game->step();
				} catch (const std::exception& ex) {
					SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Exception! %s", ex.what());
					return 1;

				} catch (const char* ex) {
					SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Exception! %s", ex);
					return 1;
				}
			}

			SDL_Log("Test '%s' passed.", target);

		} else {
			SDL_Log("Unknown test '%s', counting that as an error...", target);
			return 1;
		}


	} else {
		SDL_Log("No test configured, running normally");
		game->run();
	}

	return 0;

} catch (const std::exception& ex) {
	SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Exception! %s", ex.what());
	return 1;

} catch (const char* ex) {
	SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Exception! %s", ex);
	return 1;
}
