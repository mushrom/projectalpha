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

#include <entities/player.hpp>
#include <entities/enemy.hpp>
#include <entities/projectile.hpp>
#include <entities/enemyCollision.hpp>
#include <entities/healthPickup.hpp>
#include <entities/flag.hpp>
#include <entities/enemySpawner.hpp>
#include <entities/killedParticles.hpp>
#include <entities/targetArea.hpp>
#include <entities/amulet.hpp>

#include <logic/wfcGenerator.hpp>
#include <logic/UI.hpp>
#include <logic/levelController.hpp>

#include <nuklear/nuklear.h>

class landscapeEventSystem : public entitySystem {
	public:
		virtual void update(entityManager *manager, float delta);

		generatorEventQueue::ptr queue = std::make_shared<generatorEventQueue>();
};

class abyssDeleter : public entitySystem {
	public:
		virtual void update(entityManager *manager, float delta) {
			// XXX: delete entities that fall into the abyss
			//      could be done more efficiently,
			//      ie. some sort of spatial partitioning
			for (auto& ent : manager->entities) {
				if (ent->node->getTransformTRS().position.y < -100.f) {
					manager->remove(ent);
				}
			}
		}
};

class generatorEventHandler : public component {
	public:
		generatorEventHandler(entityManager *manager, entity *ent)
			: component(manager, ent)
		{
			manager->registerComponent(ent, "generatorEventHandler", this);
		}

		virtual void
		handleEvent(entityManager *manager, entity *ent, generatorEvent& ev) {
			const char *typestr;

			switch (ev.type) {
				case generatorEvent::types::generatorStarted:
					typestr =  "started";
					break;

				case generatorEvent::types::generated:
					typestr =  "generated";
					break;

				case generatorEvent::types::deleted:
					typestr =  "deleted";
					break;

				default:
					typestr =  "<unknown>";
					break;
			}

			SDL_Log(
				"handleEvent: got here, %s [+/-%g] [+/-%g] [+/-%g]",
				typestr, ev.extent.x, ev.extent.y, ev.extent.z);

			/*

			std::cerr << std::endl;
			*/
		}
};

class generatorEventActivator : public generatorEventHandler {
	public:
		generatorEventActivator(entityManager *manager, entity *ent)
			: generatorEventHandler(manager, ent)
		{
			manager->registerComponent(ent, "generatorEventActivator", this);
		}

		virtual void
		handleEvent(entityManager *manager, entity *ent, generatorEvent& ev) {
			// TODO: activate/deactivate stuff here
		}
};

void landscapeEventSystem::update(entityManager *manager, float delta) {
	auto handlers = manager->getComponents("generatorEventHandler");
	auto g = queue->lock();
	auto& quevec = queue->getQueue();

	for (auto& ev : quevec) {
		for (auto& it : handlers) {
			generatorEventHandler *handler = dynamic_cast<generatorEventHandler*>(it);
			entity *ent = manager->getEntity(handler);

			if (handler && ent) {
				handler->handleEvent(manager, ent, ev);
			}
		}
	}

	// XXX: should be in queue class
	quevec.clear();
}

// XXX: this sort of makes sense but not really... game entity with no renderable
//      objects, functioning as basically a subsystem? 
//      abstraction here doesn't make sense, needs to be redone
class worldEntityGenerator : public generatorEventHandler {
	public:
		worldEntityGenerator(entityManager *manager, entity *ent)
			: generatorEventHandler(manager, ent)
		{
			manager->registerComponent(ent, "generatorEventHandler", this);
		}

		virtual void
		handleEvent(entityManager *manager, entity *ent, generatorEvent& ev) {
			switch (ev.type) {
				case generatorEvent::types::generated:
					{
						// XXX
						std::tuple<float, float, float> foo =
							{ ev.position.x, ev.position.y, ev.position.z };

						if (positions.count(foo) == 0) {
							positions.insert(foo);
							SDL_Log("worldEntityGenerator(): generating some things");

							//manager->add(new enemy(manager, manager->engine, ev.position + glm::vec3(0, 50.f, 0)));

							// TODO: need a way to know what the general shape of
							//       the generated thing is...
							//manager->add(new healthPickup(manager, ev.position + glm::vec3(0, 10.f, 0)));
						}
					}
					break;

				case generatorEvent::types::deleted:
					break;

				default:
					break;
			}
		}

		std::set<std::tuple<float, float, float>> positions;
};

class worldEntitySpawner : public entity {
	public:
		worldEntitySpawner(entityManager *manager)
			: entity (manager)
		{
			manager->registerComponent(this, "worldEntitySpawner", this);
			new worldEntityGenerator(manager, this);
		}

		virtual void update(entityManager *manager, float delta) { /* nop */ };
};

class projalphaView : public gameView {
	public:
		typedef std::shared_ptr<projalphaView> ptr;
		typedef std::weak_ptr<projalphaView>   weakptr;

		projalphaView(gameMain *game);
		virtual void logic(gameMain *game, float delta);
		virtual void render(gameMain *game);
		void load(gameMain *game, std::string map);
		//void loadPlayer(void);

		enum modes {
			MainMenu,
			Move,
			Pause,
			Loading,
			Won,
			Inventory,
		};

		renderPostChain::ptr post = nullptr;
		//modalSDLInput input;
		//vecGUI vgui;
		struct nk_context *nk_ctx;

		int menuSelect = 0;
		int currentFloor = 0;
		float zoom = 20.f;

		std::unique_ptr<levelController> level;
		//landscapeGenerator landscape;
		std::unique_ptr<wfcGenerator> wfcgen;
		inputHandlerSystem::ptr inputSystem;
		std::string currentMap = "no map!";
		std::string loadedMap = "no map loaded either!";
		std::vector<physicsObject::ptr> mapPhysics;
		std::vector<entity*> levelEntities;

		void nextFloor(gameMain *game);
		void prevFloor(gameMain *game);
		bool nearNode(gameMain *game, const std::string& name, float thresh = 3.f);

	private:
		void drawMainMenu(gameMain *game, int wx, int wy);
		void drawInventory(gameMain *game, int wx, int wy);
		void drawWinScreen(gameMain *game, int wx, int wy);
		void drawPauseMenu(gameMain *game, int wx, int wy);
};

// XXX
static glm::vec2 movepos(0, 0);
static glm::vec2 actionpos(0, 0);

projalphaView::projalphaView(gameMain *game)
	: gameView(),
	  level(new levelController),
	  //wfcgen(new wfcGenerator(game, DEMO_PREFIX "assets/obj/ld48/tiles/wfc-config.json"))
	  wfcgen(new wfcGenerator(game, DEMO_PREFIX "assets/obj/catacomb-tiles/wfc-config.json"))
{
    //ctx = nk_sdl_init(win);
    nk_ctx = nk_sdl_init(game->ctx.window);

	if (!nk_ctx) {
		throw std::logic_error("Couldn't initialize nk_ctx!");
	}

    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
	{
		struct nk_font_atlas *atlas;
		nk_sdl_font_stash_begin(&atlas);
		struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, GR_PREFIX "assets/fonts/Roboto-Regular.ttf", 16, 0);
		/*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
		/*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16, 0);*/
		/*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
		/*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
		/*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
		/*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
		nk_sdl_font_stash_end();
		/*nk_style_load_all_cursors(ctx, atlas->cursors);*/
		if (roboto) {
			nk_style_set_font(nk_ctx, &roboto->handle);
		}
	}

#ifdef NO_FLOATING_FB
	post = renderPostChain::ptr(new renderPostChain(
		{loadPostShader(GR_PREFIX "shaders/baked/texpresent.frag",
		                game->rend->globalShaderOptions)},
		SCREEN_SIZE_X, SCREEN_SIZE_Y));
#else
	post = renderPostChain::ptr(new renderPostChain(
		{
			game->rend->postShaders["fog-depth"],
			//game->rend->postShaders["psaa"],
		},
		//{game->rend->postShaders["tonemap"], game->rend->postShaders["psaa"]},
		SCREEN_SIZE_X, SCREEN_SIZE_Y));
#endif

	// TODO: less redundant way to do this
#define SERIALIZABLE(T) game->factories->add<T>()
	SERIALIZABLE(entity);
	SERIALIZABLE(component);

	SERIALIZABLE(rigidBody);
	SERIALIZABLE(rigidBodySphere);
	SERIALIZABLE(rigidBodyBox);
	SERIALIZABLE(rigidBodyCylinder);
	SERIALIZABLE(rigidBodyCapsule);
	SERIALIZABLE(syncRigidBodyTransform);
	SERIALIZABLE(syncRigidBodyPosition);
	SERIALIZABLE(syncRigidBodyXZVelocity);
	//SERIALIZABLE(collisionHandler);

	SERIALIZABLE(player);
	SERIALIZABLE(enemy);
	SERIALIZABLE(enemySpawner);
	SERIALIZABLE(health);
	SERIALIZABLE(team);
	SERIALIZABLE(boxSpawner);
	SERIALIZABLE(movementHandler);
	SERIALIZABLE(projectileCollision);
	SERIALIZABLE(targetArea);
	SERIALIZABLE(areaAddScore);
#undef SERIALIZABLE

	// TODO: names are kinda pointless here
	// TODO: should systems be a state object in gameMain as well?
	//       they practically are since the entityManager here is, just one
	//       level deep...
	game->entities->systems["lifetime"] = std::make_shared<lifetimeSystem>();
	game->entities->systems["area"]     = std::make_shared<areaSystem>();
	game->entities->systems["abyss"]    = std::make_shared<abyssDeleter>();
	game->entities->systems["collision"] = std::make_shared<entitySystemCollision>();
	game->entities->systems["syncPhysics"] = std::make_shared<syncRigidBodySystem>();

	/*
	game->entities->addEvents["killedParticles"]
		= std::make_shared<killedParticles>();
		*/
	game->entities->removeEvents["enemyParticles"]
		= killedParticles::ptr(new killedParticles({"enemy"}));
	game->entities->removeEvents["spawnerParticles"]
		= killedParticles::ptr(new killedParticles({"enemySpawner"}));
	game->entities->removeEvents["playerParticles"]
		= killedParticles::ptr(new killedParticles({"player"}));

	inputSystem = std::make_shared<inputHandlerSystem>();
	game->entities->systems["input"] = inputSystem;

	auto generatorSys = std::make_shared<landscapeEventSystem>();
	game->entities->systems["landscapeEvents"] = generatorSys;
	wfcgen->setEventQueue(generatorSys->queue);

	bindCookedMeshes();
	input.bind(MODAL_ALL_MODES, resizeInputHandler(game, post));
	input.bind(MODAL_ALL_MODES, [=, this] (SDL_Event& ev, unsigned flags) {
		nk_sdl_handle_event(&ev);
		return MODAL_NO_CHANGE;
	});

#if !defined(__ANDROID__)
	//input.bind(modes::Move, controller::camMovement(cam, 30.f));
	//input.bind(modes::Move, controller::camFPS(cam, game));
	// movement controller needs to be MODAL_ALL_MODES to avoid dropping
	// keystrokes and missing state changes
	input.bind(MODAL_ALL_MODES, controller::camMovement2D(cam, 15.f));
	input.bind(modes::Move, controller::camScrollZoom(cam, &zoom, 3.f));
	input.bind(modes::Move, inputMapper(inputSystem->inputs, cam));
#endif

	//input.bind(modes::Move, controller::camAngled2DFixed(cam, game, -M_PI/4.f));
	input.bind(modes::Move,
		controller::camAngled2DRotatable(cam, game,
		                                 -M_PI/4.f, -M_PI/2.f, 0.f));
	input.bind(modes::Move, [=, this] (SDL_Event& ev, unsigned flags) {
		inputSystem->handleEvent(game->entities.get(), ev);
		return MODAL_NO_CHANGE;
	});

	input.bind(modes::Inventory, [=, this] (SDL_Event& ev, unsigned flags) {
		if (ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_TAB) {
			return (int)modes::Move;
		}

		return MODAL_NO_CHANGE;
	});

	// TODO: configurable keybinds
	input.bind(modes::Move, [=, this] (SDL_Event& ev, unsigned flags) {
		if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_TAB) {
			return (int)modes::Inventory;
		}

		return MODAL_NO_CHANGE;
	});

	input.bind(modes::Move, [=, this] (SDL_Event& ev, unsigned flags) {
		if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) {
			return (int)modes::Pause;
		}

		return MODAL_NO_CHANGE;
	});

	input.bind(modes::Move, [=, this] (SDL_Event& ev, unsigned flags) {
		if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_TAB) {
			return (int)modes::Inventory;
		}

		return MODAL_NO_CHANGE;
	});


	static nlohmann::json testthing = {};

	input.bind(MODAL_ALL_MODES,
		[=] (SDL_Event& ev, unsigned flags) {
			if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_h) {
				nlohmann::json compJson;

				for (auto& ent : game->entities->entities) {
					//std::cerr << ent->typeString() << std::endl;
					if (game->factories->has(ent->typeString())) {
						compJson.push_back(ent->serialize(game->entities.get()));
					}
				}

				std::cerr << compJson.dump(4) << std::endl;
				testthing = compJson;
			}

			return MODAL_NO_CHANGE;
		});

	input.bind(MODAL_ALL_MODES,
		[=] (SDL_Event& ev, unsigned flags) {
			if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_i) {
				for (auto& entprops : testthing) {
					entity *ent =
						game->factories->build(game->entities.get(), entprops);

					std::cerr
						<< "Loading entity, properties: "
						<< entprops.dump(4)
						<< std::endl;

					if (ent) {
						game->entities->add(ent);
					}
				}
			}

			return MODAL_NO_CHANGE;
		});

	input.setMode(modes::MainMenu);
};

void projalphaView::nextFloor(gameMain *game) {
	gameObject::ptr wfcroot = wfcgen->getNode()->getNode("nodes");
	currentFloor++;

	for (auto& ent : levelEntities) {
		if (ent->active) {
			// XXX: items in the inventory are deactivated, and so
			//      won't be removed here...
			//      bit of a hack, but probably ok performance-wise
			game->entities->remove(ent);
		}
	}

	levelEntities.clear();
	wfcgen->generate(game, {});

	if (wfcroot && wfcroot->hasNode("leaves")) {
		gameObject::ptr leafroot = wfcroot->getNode("leaves");
		glm::vec3 amuletPos;

		for (const auto& [name, ptr] : leafroot->nodes) {
			auto en = new enemy(game->entities.get(),
					game,
					ptr->getTransformTRS().position + glm::vec3(4, 2, 0));
			amuletPos = ptr->getTransformTRS().position + glm::vec3(2);

			new team(game->entities.get(), en, "red");
			game->entities->add(en);
			levelEntities.push_back(en);

			if (rand() & 1) {
				auto hen = new healthPickup(game->entities.get(),
				                            ptr->getTransformTRS().position + glm::vec3(0, 0.75, 0));
				
				game->entities->add(hen);
				levelEntities.push_back(hen);

			} else {
				auto xen = new coinPickup(game->entities.get(),
				                          ptr->getTransformTRS().position + glm::vec3(0, 0, 0));
				
				game->entities->add(xen);
				levelEntities.push_back(xen);
			}
		}

		if (currentFloor == 5) {
			auto en = new amuletPickup(game->entities.get(), game, amuletPos);
			game->entities->add(en);
		}
	}
}

void projalphaView::prevFloor(gameMain *game) {
	gameObject::ptr wfcroot = wfcgen->getNode()->getNode("nodes");
	currentFloor--;

	for (auto& ent : levelEntities) {
		game->entities->remove(ent);
	}

	levelEntities.clear();
	wfcgen->generate(game, {});

	if (wfcroot && wfcroot->hasNode("leaves")) {
		gameObject::ptr leafroot = wfcroot->getNode("leaves");

		for (const auto& [name, ptr] : leafroot->nodes) {
			auto en = new enemy(game->entities.get(),
					game,
					ptr->getTransformTRS().position + glm::vec3(4, 2, 0));

			new team(game->entities.get(), en, "red");
			game->entities->add(en);
			levelEntities.push_back(en);
		}
	}
}

bool projalphaView::nearNode(gameMain *game, const std::string& name, float thresh)
{
	gameObject::ptr wfcroot = wfcgen->getNode()->getNode("nodes");
	entity *playerEnt = findFirst(game->entities.get(), {"player"});

	if (wfcroot->hasNode("exit") && playerEnt) {
		TRS nodeTrans = wfcroot->getNode(name)->getTransformTRS();
		TRS playerTrans = playerEnt->getNode()->getTransformTRS();
		// TODO: need a way to calculate the transform from this node
		//glm::vec3 pos = nodeTrans.position - glm::vec3(64, 0, 64);
		//glm::vec3 pos = nodeTrans.position - glm::vec3(64, 0, 64);
		glm::vec3 pos = nodeTrans.position;
		return glm::distance(pos, playerTrans.position) < 3;
	}

	return false;
}

void projalphaView::logic(gameMain *game, float delta) {
	// XXX: handle input from end of frame at render() to logic() here
	//      on the next frame... should probably override handleInput()
	nk_input_end(nk_ctx);

	if (input.mode == modes::MainMenu
		|| input.mode == modes::Pause
		|| input.mode == modes::Won)
	{
		// XXX:
		return;
	}

	// big XXX
	if (currentMap != loadedMap) {
		loadedMap = currentMap;
		load(game, currentMap);
	}

	if (input.mode == modes::Loading) {
		gameObject::ptr wfcroot = wfcgen->getNode()->getNode("nodes");
		//if (!game->state->rootnode->hasNode("wfc")) {
		if (!wfcroot || !wfcroot->hasNode("leaves")) {
			return;

		} else {
			input.setMode(modes::Move);
		}
	}

	static glm::vec3 lastvel = glm::vec3(0);
	static gameObject::ptr retval;

	if (cam->velocity() != lastvel) {
		lastvel = cam->velocity();
	}

	game->phys->stepSimulation(delta);
	game->phys->filterCollisions();;

	entity *playerEnt = findFirst(game->entities.get(), {"player"});

	if (playerEnt) {
		TRS transform = playerEnt->getNode()->getTransformTRS();
		cam->slide(transform.position - zoom*cam->direction(), 16.f, delta);
		wfcgen->setPosition(game, transform.position);
	}

	game->entities->update(delta);

	if (level->won()) {
		SDL_Log("winner winner, a dinner of chicken!");
		level->reset();
		input.setMode(modes::Won);
	} 

	auto lost = level->lost();
	if (lost.first) {
		SDL_Log("lol u died: %s", lost.second.c_str());
		input.setMode(modes::MainMenu);
	}

	if (nearNode(game, "exit")) {
		nextFloor(game);
	}

	if (nearNode(game, "entry")) {
		prevFloor(game);
	}
}

void projalphaView::render(gameMain *game) {
	int winsize_x, winsize_y;
	SDL_GetWindowSize(game->ctx.window, &winsize_x, &winsize_y);
	renderFlags flags = game-> rend->getLightingFlags();

	if (input.mode == modes::MainMenu) {
		renderWorld(game, cam, flags);

		// TODO: need to set post size on resize event..
		//post->setSize(winsize_x, winsize_y);
		post->setUniform("exposure", game->rend->exposure);
		post->setUniform("time_ms",  SDL_GetTicks() * 1.f);
		post->draw(game->rend->framebuffer);
		//input.setMode(modes::Move);

		// TODO: function to do this
		drawMainMenu(game, winsize_x, winsize_y);

	} else if (input.mode == modes::Pause) {
		renderWorld(game, cam, flags);

		// TODO: need to set post size on resize event..
		//post->setSize(winsize_x, winsize_y);
		post->setUniform("exposure", game->rend->exposure);
		post->setUniform("time_ms",  SDL_GetTicks() * 1.f);
		post->draw(game->rend->framebuffer);

		// TODO: function to do this
		drawPauseMenu(game, winsize_x, winsize_y);

	} else if (input.mode == modes::Won) {
		renderWorld(game, cam, flags);

		// TODO: need to set post size on resize event..
		//post->setSize(winsize_x, winsize_y);
		post->setUniform("exposure", game->rend->exposure);
		post->setUniform("time_ms",  SDL_GetTicks() * 1.f);
		post->draw(game->rend->framebuffer);

		// TODO: function to do this
		drawWinScreen(game, winsize_x, winsize_y);

	} else if (input.mode == modes::Loading) {
		// render loading screen
		Framebuffer().bind();
		setDefaultGlFlags();
		disable(GL_DEPTH_TEST);
		disable(GL_SCISSOR_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	} else if (input.mode == modes::Inventory) {
		renderWorld(game, cam, flags);

		// TODO: need to set post size on resize event..
		//post->setSize(winsize_x, winsize_y);
		post->setUniform("exposure", game->rend->exposure);
		post->setUniform("time_ms",  SDL_GetTicks() * 1.f);
		post->draw(game->rend->framebuffer);
		//input.setMode(modes::Move);

		// TODO: function to do this
		//drawMainMenu(game, winsize_x, winsize_y);
		renderHealthbars(game->entities.get(), nk_ctx, cam);
		drawInventory(game, winsize_x, winsize_y);

	} else {
		// main game mode
		renderWorld(game, cam, flags);
		post->setUniform("exposure", game->rend->exposure);
		post->setUniform("time_ms",  SDL_GetTicks() * 1.f);
		post->draw(game->rend->framebuffer);

		Framebuffer().bind();
		setDefaultGlFlags();

		disable(GL_DEPTH_TEST);
		disable(GL_SCISSOR_TEST);
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		renderHealthbars(game->entities.get(), nk_ctx, cam);
	}

	nk_sdl_render(NK_ANTI_ALIASING_ON, 512*1024, 128*1024);
	// XXX: handle input from end of frame here to beginning of logic()
	//      on the next frame... should probably override handleInput()
	nk_input_begin(nk_ctx);
}

static std::vector<std::pair<std::string, bool>> listdir(std::string path) {
	std::vector<std::pair<std::string, bool>> ret;

// XXX: older debian raspi is built on doesn't include c++17 filesystem functions,
//      so leave the old posix code in for that... aaaaa
#if defined(_WIN32)
	if (fs::exists(currentDir) && fs::is_directory(path)) {
		for (auto& p : fs::directory_iterator(currentDir)) {
			ret.push_back({p.path().filename(), !fs::is_directory(p.path())});
		}

	} else {
		SDL_Log("listdir: Invalid directory %s", path.c_str());
	}

#else
	DIR *dirp;

	if ((dirp = opendir(path.c_str()))) {
		struct dirent *dent;

		while ((dent = readdir(dirp))) {
			ret.push_back({std::string(dent->d_name), dent->d_type != DT_DIR});
		}

		/*
		std::sort(dirContents.begin(), dirContents.end(),
			[&] (struct f_dirent& a, struct f_dirent& b) {
				return (a.type != b.type)
					? a.type < b.type
					: a.name < b.name;
			});
			*/
	}
#endif

	return ret;
}

void projalphaView::load(gameMain *game, std::string map) {
	// avoid reloading if the target map is already loaded
	if (true || map != currentMap) {
		TRS staticPosition; // default
		gameObject::ptr newroot
			= game->state->rootnode
			= std::make_shared<gameObject>();

		currentMap = map;
		//game->state->rootnode = loadMapCompiled(game, map);
		game->jobs->addAsync([=, this] () {
			//auto [node, models] = loadMapData(game, map);
			auto mapdata = loadMapData(game, map);
			auto node = mapdata.first;
			auto models = mapdata.second;

			game->jobs->addDeferred([=, this] () {
				// TODO: some sort of world entity
				mapPhysics.clear();
				game->phys->addStaticModels(nullptr,
				                            node,
				                            staticPosition,
				                            mapPhysics);

				compileModels(models);

				level->reset();
				game->state->rootnode = node;
				setNode("asyncLoaded", node, std::make_shared<gameObject>());
				setNode("entities", node, game->entities->root);
				setNode("wfc", node, wfcgen->getNode());

				return true;
			});

			return true;
		});
	}
}

void projalphaView::drawMainMenu(gameMain *game, int wx, int wy) {
	static int selected;
	bool reset = false;

	//input.setMode(modes::Loading);
	if (nk_begin(nk_ctx, "Main menu", nk_rect(50, 50, 220, 220),
	             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE))
	{
		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "New game")) {
			SDL_Log("New game!");
			input.setMode(modes::Loading);
			reset = true;
		}
		
		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "Settings")) {
			SDL_Log("Settings");
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "Quit")) {
			SDL_Log("Quiterino");
		}
	}
	nk_end(nk_ctx);

	if (reset) {
		level->reset();
	}
}

void projalphaView::drawPauseMenu(gameMain *game, int wx, int wy) {
	if (nk_begin(nk_ctx, "Pause", nk_rect(50, 50, 220, 220),
	             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "Resume")) {
			SDL_Log("Resuming");
			input.setMode(modes::Move);
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "Settings")) {
			SDL_Log("Settings");
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "Abandon run")) {
			SDL_Log("Abandon run");
			input.setMode(modes::MainMenu);
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "Quit")) {
			SDL_Log("Quiterino");
		}
	}
	nk_end(nk_ctx);
}

void projalphaView::drawInventory(gameMain *game, int wx, int wy) {
	entity *playerEnt = findFirst(game->entities.get(), {"player", "inventory"});
	if (!playerEnt) return;

	auto inv = castEntityComponent<inventory*>(game->entities.get(), playerEnt, "inventory");

	if (!inv) return;

	static std::map<entity*, std::string> names;

	if (nk_begin(nk_ctx, "Player inventory", nk_rect(50, 50, 270, 270),
	             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE))
	{
		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "a button")) {
			// asdf
			SDL_Log("clicked a button");
		}

		for (auto& [key, items] : inv->items) {
			if (items.size() == 0) {
				// no items of this type, nothing to do
				continue;
			}

			const char *name = key.c_str();
			entity *ent = *items.begin();

			/*
			if (names.count(ent) == 0) {
				std::string foo = std::string(ent->typeString()) + ": ";
				auto comps = game->entities->getEntityComponents(ent);

				for (auto& [name, _] : comps) {
					foo += name + ", ";
				}

				names[ent] = foo;
			}

			const char *name = names[ent].c_str();
			*/

			nk_layout_row_dynamic(nk_ctx, 0, 3);
			nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "%lu : %s", items.size(), name);

			if (nk_button_label(nk_ctx, "Use")) {
				SDL_Log("Using %s", name);

				Wieldable *w;
				castEntityComponent(w, game->entities.get(), ent, "Wieldable");

				// TODO: need a way to safely observe entity pointers in cases
				//       where they may be deleted... don't want to use shared_ptr
				//       here because the entity manager has sole ownership
				//       over the lifetime of the entity, shared_ptr would
				//       result in lingering invalid entities
				if (w) {
					inv->remove(game->entities.get(), ent);
					game->entities->activate(ent);
					w->action(game->entities.get(), playerEnt);
				}
			}

			if (nk_button_label(nk_ctx, "Drop")) {
				inv->remove(game->entities.get(), ent);
				game->entities->activate(ent);
				TRS newtrans = playerEnt->node->getTransformTRS();
				newtrans.position += glm::vec3(3, 0, 3);
				ent->node->setTransform(newtrans);
			}
		}
	}
	nk_end(nk_ctx);
}

void projalphaView::drawWinScreen(gameMain *game, int wx, int wy) {
	static int selected;
	bool reset = false;

	if (nk_begin(nk_ctx, "You are winnar!", nk_rect(wx/2, wy/2, 220, 220),
	             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE))
	{
		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "New game")) {
			SDL_Log("Starting new game");
			input.setMode(modes::Move);
			level->reset();
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "Main menu")) {
			SDL_Log("Going to main menu");
			input.setMode(modes::MainMenu);
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "Quit")) {
			SDL_Log("Quiterino");
		}
	}
	nk_end(nk_ctx);
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
		auto bar = openSpatialLoop(GR_PREFIX "assets/sfx/Meditating Beat.ogg");
		bar->worldPosition = glm::vec3(0, 0, -5);
		game->audio->add(bar);
		return true;
	});

	projalphaView::ptr view = std::make_shared<projalphaView>(game);
	view->cam->setFar(1000.0);
	view->cam->setFovx(70.0);
	game->setView(view);
	game->rend->lightThreshold = 0.2;

	/*
	// JS flashbacks
	view->level->addInit([=] () {
		gameObject::ptr flagnodes = game->state->rootnode->getNode("flags");

		initEntitiesFromNodes(flagnodes,
			[=] (const std::string& name, gameObject::ptr& ptr) {
				std::cerr << "have flag node " << name << std::endl;

				auto f = new flag(game->entities.get(), game,
								  ptr->getTransformTRS().position, name);
				game->entities->add(f);
			});
	});

	view->level->addInit([=] () {
		gameObject::ptr spawners = game->state->rootnode->getNode("spawners");

		initEntitiesFromNodes(spawners,
			[&] (const std::string& name, gameObject::ptr& ptr) {
				std::cerr << "have spawner node " << name << std::endl;

				auto en = new enemySpawner(game->entities.get(), game,
										   ptr->getTransformTRS().position);
				new team(game->entities.get(), en, "red");
				game->entities->add(en);
			});
	});
	*/

	view->level->addInit([=] () {
		view->wfcgen->generate(game, {});
	});

	view->level->addInit([=] () {
		entity *playerEnt;
		glm::vec3 pos(-5, 20, -5);
		gameObject::ptr wfcroot = view->wfcgen->getNode()->getNode("nodes");

		if (wfcroot->hasNode("entry")) {
			TRS transform = wfcroot->getNode("entry")->getTransformTRS();
			// TODO: need a way to calculate the transform from this node
			//pos = transform.position + glm::vec3(0, 20, 0) - glm::vec3(64, 0, 64);
			pos = transform.position + glm::vec3(0, 2, 0);
			//pos = transform.position;
		}

		playerEnt = new player(game->entities.get(), game, pos);
		game->entities->add(playerEnt);
		new generatorEventHandler(game->entities.get(), playerEnt);
		new health(game->entities.get(), playerEnt);
		new enemyCollision(game->entities.get(), playerEnt);
		new healthPickupCollision(game->entities.get(), playerEnt);
		new flagPickup(game->entities.get(), playerEnt);
		new team(game->entities.get(), playerEnt, "blue");
		new areaAddScore(game->entities.get(), playerEnt, {});
		inventory *inv = new inventory(game->entities.get(), playerEnt, {});

		// start with 5 flares
		for (int i = 0; i < 5; i++) {
			entity *flare = new flareItem(game->entities.get(), glm::vec3(0));
			game->entities->add(flare);
			inv->insert(game->entities.get(), flare);
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
		view->currentFloor = 1;
		view->nextFloor(game);
	});

	view->level->addDestructor([=] () {
		// TODO: should just have reset function in entity manager
		for (auto& ent : game->entities->entities) {
			game->entities->remove(ent);
		}

		game->entities->clearFreedEntities();
	});

	view->level->addObjective("Reach exit",
		[=] () {
			auto players
				= searchEntities(game->entities.get(), {"player", "hasItem:amuletPickup"});
			// TODO: check for goal item (amulet?) and current floor == 0 (exit)
			return view->currentFloor == 0 && players.size() != 0;
		});

	view->level->addLoseCondition(
		[=] () {
			std::set<entity*> players
				= searchEntities(game->entities.get(), {"player"});

			return std::pair<bool, std::string>(players.size() == 0, "lol u died");
		});

	SDL_Log("Got to game->run()!");
	//view->load(game, mapfile);
	auto mapdata = loadMapCompiled(game, mapfile);
	game->state->rootnode = mapdata;

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
