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
#include <entities/healthPickup.hpp>
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

// from the nuklear demo code
enum theme {THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK};

static void
set_style(struct nk_context *ctx, enum theme theme)
{
    struct nk_color table[NK_COLOR_COUNT];
    if (theme == THEME_WHITE) {
        table[NK_COLOR_TEXT] = nk_rgba(70, 70, 70, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_HEADER] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_BORDER] = nk_rgba(0, 0, 0, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(185, 185, 185, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(170, 170, 170, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(160, 160, 160, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(150, 150, 150, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(120, 120, 120, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_SELECT] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(80, 80, 80, 255);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(70, 70, 70, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(60, 60, 60, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_EDIT] = nk_rgba(150, 150, 150, 255);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(0, 0, 0, 255);
        table[NK_COLOR_COMBO] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_CHART] = nk_rgba(160, 160, 160, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(45, 45, 45, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(180, 180, 180, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(140, 140, 140, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(150, 150, 150, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(160, 160, 160, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(180, 180, 180, 255);
        nk_style_from_table(ctx, table);
    } else if (theme == THEME_RED) {
        table[NK_COLOR_TEXT] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(30, 33, 40, 215);
        table[NK_COLOR_HEADER] = nk_rgba(181, 45, 69, 220);
        table[NK_COLOR_BORDER] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(190, 50, 70, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(195, 55, 75, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 60, 60, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SELECT] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(186, 50, 74, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(191, 55, 79, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_EDIT] = nk_rgba(51, 55, 67, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_COMBO] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_CHART] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(170, 40, 60, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(30, 33, 40, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(181, 45, 69, 220);
        nk_style_from_table(ctx, table);
    } else if (theme == THEME_BLUE) {
        table[NK_COLOR_TEXT] = nk_rgba(20, 20, 20, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(202, 212, 214, 215);
        table[NK_COLOR_HEADER] = nk_rgba(137, 182, 224, 220);
        table[NK_COLOR_BORDER] = nk_rgba(140, 159, 173, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(137, 182, 224, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(142, 187, 229, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(147, 192, 234, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(177, 210, 210, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(182, 215, 215, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(137, 182, 224, 255);
        table[NK_COLOR_SELECT] = nk_rgba(177, 210, 210, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(137, 182, 224, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(177, 210, 210, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(137, 182, 224, 245);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(142, 188, 229, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(147, 193, 234, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_EDIT] = nk_rgba(210, 210, 210, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(20, 20, 20, 255);
        table[NK_COLOR_COMBO] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_CHART] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(137, 182, 224, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(190, 200, 200, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(156, 193, 220, 255);
        nk_style_from_table(ctx, table);
    } else if (theme == THEME_DARK) {
        table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 215);
        table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
        table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 93, 121, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(63, 98, 126, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 53, 56, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 245);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 83, 111, 255);
        nk_style_from_table(ctx, table);
    } else {
        nk_style_default(ctx);
    }
}



/*
class landscapeEventSystem : public entitySystem {
	public:
		virtual void update(entityManager *manager, float delta);

		generatorEventQueue::ptr queue = std::make_shared<generatorEventQueue>();
};
*/

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

#if 0
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
#endif

// XXX
static glm::vec2 movepos(0, 0);
static glm::vec2 actionpos(0, 0);

projalphaView::projalphaView(gameMain *game)
	: gameView(),
	  level(new levelController)
	  //wfcgen(new wfcGenerator(game, DEMO_PREFIX "assets/obj/ld48/tiles/wfc-config.json"))
	  //wfcgen(new wfcGenerator(game, DEMO_PREFIX "assets/obj/catacomb-tiles/wfc-config.json"))
{
    //ctx = nk_sdl_init(win);
    nk_ctx = nk_sdl_init(game->ctx.window);

	if (!nk_ctx) {
		throw std::logic_error("Couldn't initialize nk_ctx!");
	}

	set_style(nk_ctx, THEME_RED);

    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
	{
		struct nk_font_atlas *atlas;
		nk_sdl_font_stash_begin(&atlas);
		struct nk_font *roboto;
		roboto = nk_font_atlas_add_from_file(atlas, GR_PREFIX "assets/fonts/Roboto-Regular.ttf", 16, 0);
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
	game->entities->systems["UI"] = std::make_shared<uiSystem>(nk_ctx, cam);

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

	/*
	auto generatorSys = std::make_shared<landscapeEventSystem>();
	game->entities->systems["landscapeEvents"] = generatorSys;
	wfcgen->setEventQueue(generatorSys->queue);
	*/

	bindCookedMeshes();
	input.bind(MODAL_ALL_MODES, resizeInputHandler(game, post));
	input.bind(MODAL_ALL_MODES, [=, this] (SDL_Event& ev, unsigned flags) {
		nk_sdl_handle_event(&ev);
		return MODAL_NO_CHANGE;
	});

#if defined(GAME_BUILD_DEBUG)
	input.bind(MODAL_ALL_MODES, [=, this] (SDL_Event& ev, unsigned flags) {
		if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_t) {
			debugTiles =! debugTiles;
		}
		return MODAL_NO_CHANGE;
	});
#endif

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
		                                 -M_PI/2.f, -M_PI/2.f, -M_PI/2.f));
	input.bind(modes::Move, [=, this] (SDL_Event& ev, unsigned flags) {
		inputSystem->handleEvent(game->entities.get(), ev);
		return MODAL_NO_CHANGE;
	});

	input.bind(modes::Move, [=, this] (SDL_Event& ev, unsigned flags) {
		if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_SPACE) {
			if (nearNode(game, "exit")) {
				incrementFloor(game, 1);
			}

			if (nearNode(game, "entry")) {
				incrementFloor(game, -1);
			}
		}

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

projalphaView::floorStates::floorStates(gameMain *game,
                                        projalphaView *view,
                                        std::string z,
                                        std::string spec)
	: zone(z)
	  //mapQueue(view->cam)
{
	generator = std::make_shared<wfcGenerator>(game, spec),
	generator->generate(game, {});

	SDL_Log("Generated");
	generator->setPosition(game, glm::vec3(0));
	SDL_Log("Set position");

	//setNode("foo", game->state->rootnode, generator->getNode());
	//mapQueue.add(generator->getNode());
	//SDL_Log("Adding to queue: %lu meshes\n", mapQueue.meshes.size());

	gameObject::ptr wfcroot = generator->getNode()->getNode("nodes");
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

			int mod = rand() % 3;

			if (mod == 0) {
				auto hen = new healthPickup(game->entities.get(),
				                            ptr->getTransformTRS().position + glm::vec3(0, 0.75, 0));
				
				game->entities->add(hen);
				levelEntities.push_back(hen);

			} else if (mod == 1) {
				// chest
				auto cen = new chestItem(game->entities.get(),
				                         ptr->getTransformTRS().position + glm::vec3(0, 0.75, 0));

				game->entities->add(cen);
				levelEntities.push_back(cen);

			} else {
				auto xen = new coinPickup(game->entities.get(),
				                          ptr->getTransformTRS().position + glm::vec3(0, 0, 0));
				
				game->entities->add(xen);
				levelEntities.push_back(xen);
			}
		}

		/*
		if (currentFloor == 5) {
			auto en = new amuletPickup(game->entities.get(), game, amuletPos);
			game->entities->add(en);
		}
		*/

		if (rand() % 10 == 0) {
			auto en = new amuletPickup(game->entities.get(), game, amuletPos);
			game->entities->add(en);
		}
	}
	
	SDL_Log("Generated entities");

	auto entranceNode = wfcroot->getNode("entry");
	auto exitNode     = wfcroot->getNode("exit");

	if (entranceNode) entrance = entranceNode->getTransformTRS().position;
	if (exitNode)     exit     = exitNode->getTransformTRS().position;

	SDL_Log("Have entrance: (%g, %g, %g)", entrance.x, entrance.y, entrance.z);
	SDL_Log("Have exit: (%g, %g, %g)", exit.x, exit.y, exit.z);
/*
	entity *playerEnt = findFirst(game->entities.get(), {"player"});
	if (playerEnt && wfcroot) {
		auto p = wfcroot->getNode((amount > 0)? "entry" : "exit");
		TRS t = p->getTransformTRS();
		// XXX: avoid falling below staircases
		t.position += glm::vec3(0, 2, 0);
		updateEntityTransforms(game->entities.get(), playerEnt, t);
		std::cerr << "Setting player transform" << std::endl;
	}
	*/
}

projalphaView::floorStates* projalphaView::getFloor(gameMain *game, int n) {
#if 0
	/*
	static floorStates *foo = nullptr;

	if (!foo) {
		foo = new floorStates(
			game,
			this,
			"catacombs",
			DEMO_PREFIX "assets/obj/catacomb-tiles/wfc-config.json"
		);
	}

	return foo;
	*/

#else
	if (n < 0) return nullptr;

	if (n < floors.size()) {
		return &floors[n];
	}

	// otherwise, have to generate a new floor
	// TODO: what happens if there's a jump larger than one level, not
	//       just pushing to the back?

/*
	floors.push_back(floorStates(
		game,
		this,
		"catacombs",
		// TODO: wfcGenerator will create a new spec class in the constructor,
		//       which means it'll load all the models again there...
		//       should have a spec class instead, and pass that to the generator
		DEMO_PREFIX "assets/obj/catacomb-tiles/wfc-config.json"
	));
	*/

	floorStates foo(
		game,
		this,
		"catacombs",
		DEMO_PREFIX "assets/obj/catacomb-tiles/wfc-config.json"
	);

	//SDL_Log("Generated floor, map queue has %lu meshes", foo.mapQueue.meshes.size());

	floors.push_back(foo);
	return &floors.back();
#endif
}

// lols
#define let auto

void projalphaView::incrementFloor(gameMain *game, int amount) {
	int nextFloor = currentFloor + amount;
	auto cur  = getFloor(game, currentFloor);
	//let cur = nullptr;
	let next = getFloor(game, nextFloor);

	if (cur) {
		/* deactivate stuff */;
		/*
		for (auto& p : cur->generator->mapobjs) {
			p->deactivate();
		}
		*/
		cur->generator->mapobjs.clear();
	}

	if (next) {
		/* activate stuff */
		next->generator->mapobjs.clear();
		next->generator->setPosition(game, glm::vec3(0));
		mapQueue.clear();
		mapQueue.add(next->generator->getNode());

		SDL_Log("Built queue: %lu meshes\n", mapQueue.meshes.size());

		entity *playerEnt = findFirst(game->entities.get(), {"player"});
		if (playerEnt) {
			TRS t;
			t.position = (amount > 0)? next->entrance : next->exit;
			// XXX: avoid falling below staircases
			t.position += glm::vec3(0, 2, 0);
			updateEntityTransforms(game->entities.get(), playerEnt, t);
			SDL_Log("Setting player transform");
		}

		/*
		for (auto& p : next->generator->mapobjs) {
			p->activate();
		}
		*/
	};

	currentFloor = nextFloor;
/*
	for (auto& ent : levelEntities) {
		if (ent->active) {
			// XXX: items in the inventory are deactivated, and so
			//      won't be removed here...
			//      bit of a hack, but probably ok performance-wise
			game->entities->remove(ent);
		}
	}
	*/

#if 0
	mapPhysics.clear();
	mapQueue.clear();
	levelEntities.clear();
	wfcgen->generate(game, {});

				//setNode("wfc", node, wfcgen->getNode());

	mapQueue.add(wfcgen->getNode());

	gameObject::ptr wfcroot = wfcgen->getNode()->getNode("nodes");
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

			int mod = rand() % 3;

			if (mod == 0) {
				auto hen = new healthPickup(game->entities.get(),
				                            ptr->getTransformTRS().position + glm::vec3(0, 0.75, 0));
				
				game->entities->add(hen);
				levelEntities.push_back(hen);

			} else if (mod == 1) {
				// chest
				auto cen = new chestItem(game->entities.get(),
				                         ptr->getTransformTRS().position + glm::vec3(0, 0.75, 0));

				game->entities->add(cen);
				levelEntities.push_back(cen);

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

	entity *playerEnt = findFirst(game->entities.get(), {"player"});
	if (playerEnt && wfcroot) {
		auto p = wfcroot->getNode((amount > 0)? "entry" : "exit");
		TRS t = p->getTransformTRS();
		// XXX: avoid falling below staircases
		t.position += glm::vec3(0, 2, 0);
		updateEntityTransforms(game->entities.get(), playerEnt, t);
		std::cerr << "Setting player transform" << std::endl;
	}
#endif
}

/*
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
*/

bool projalphaView::nearNode(gameMain *game, const std::string& name, float thresh)
{
	auto floor = getFloor(game, currentFloor);
	// XXX: don't like this one bit
	if (!floor) return false;

	gameObject::ptr wfcroot = floor->generator->getNode()->getNode("nodes");
	//gameObject::ptr wfcroot = wfcgen->getNode()->getNode("nodes");
	entity *playerEnt = findFirst(game->entities.get(), {"player"});

	if (wfcroot->hasNode(name) && playerEnt) {
		TRS nodeTrans = wfcroot->getNode(name)->getTransformTRS();
		TRS playerTrans = playerEnt->getNode()->getTransformTRS();
		// TODO: need a way to calculate the transform from this node
		//glm::vec3 pos = nodeTrans.position - glm::vec3(64, 0, 64);
		//glm::vec3 pos = nodeTrans.position - glm::vec3(64, 0, 64);
		glm::vec3 pos = nodeTrans.position;
		return glm::distance(pos, playerTrans.position) < thresh;
	}

	return false;
}

void projalphaView::logic(gameMain *game, float delta) {
	// XXX: handle input from end of frame at render() to logic() here
	//      on the next frame... should probably override handleInput()
	nk_input_end(nk_ctx);

	if (input.mode == modes::MainMenu
		|| input.mode == modes::NewGame
		|| input.mode == modes::Pause
		|| input.mode == modes::Won
		)
	{
		// XXX:
		return;
	}

	/*
	// big XXX
	if (currentMap != loadedMap) {
		loadedMap = currentMap;
		load(game, currentMap);
	}
	*/

/*
	// TODO: is this still even being used
	if (input.mode == modes::Loading) {
		gameObject::ptr wfcroot = wfcgen->getNode()->getNode("nodes");
		//if (!game->state->rootnode->hasNode("wfc")) {
		if (!wfcroot || !wfcroot->hasNode("leaves")) {
			return;

		} else {
			input.setMode(modes::Move);
		}
	}
	*/

	if (input.mode == modes::Loading) {
		input.setMode(modes::Move);
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
		//wfcgen->setPosition(game, transform.position);
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
}

void projalphaView::render(gameMain *game) {
	int winsize_x, winsize_y;
	SDL_GetWindowSize(game->ctx.window, &winsize_x, &winsize_y);
	renderFlags flags = game-> rend->getLightingFlags();

	if (input.mode == modes::MainMenu) {
		renderWorld(game, cam, mapQueue, flags);

		// TODO: need to set post size on resize event..
		//post->setSize(winsize_x, winsize_y);
		post->setUniform("exposure", game->rend->exposure);
		post->setUniform("time_ms",  SDL_GetTicks() * 1.f);
		post->draw(game->rend->framebuffer);

		drawMainMenu(game, winsize_x, winsize_y);

	} else if (input.mode == modes::NewGame) {
		renderWorld(game, cam, mapQueue, flags);

		// TODO: need to set post size on resize event..
		//post->setSize(winsize_x, winsize_y);
		post->setUniform("exposure", game->rend->exposure);
		post->setUniform("time_ms",  SDL_GetTicks() * 1.f);
		post->draw(game->rend->framebuffer);

		drawNewGameMenu(game, winsize_x, winsize_y);

	} else if (input.mode == modes::Pause) {
		renderWorld(game, cam, mapQueue, flags);

		// TODO: need to set post size on resize event..
		//post->setSize(winsize_x, winsize_y);
		post->setUniform("exposure", game->rend->exposure);
		post->setUniform("time_ms",  SDL_GetTicks() * 1.f);
		post->draw(game->rend->framebuffer);

		// TODO: function to do this
		drawPauseMenu(game, winsize_x, winsize_y);

	} else if (input.mode == modes::Won) {
		renderWorld(game, cam, mapQueue, flags);

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
		renderWorld(game, cam, mapQueue, flags);

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
		//if (floor) renderWorld(game, cam, floor->mapQueue, flags);
		renderWorld(game, cam, mapQueue, flags);

		post->setUniform("exposure", game->rend->exposure);
		post->setUniform("time_ms",  SDL_GetTicks() * 1.f);
		post->draw(game->rend->framebuffer);

		Framebuffer().bind();
		setDefaultGlFlags();

		disable(GL_DEPTH_TEST);
		disable(GL_SCISSOR_TEST);
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		renderHealthbars(game->entities.get(), nk_ctx, cam);

		if (debugTiles) {
			drawTileDebug(game);
		}

		drawNavPrompts(game, winsize_x, winsize_y);
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
	return;
	// avoid reloading if the target map is already loaded
	if (true || map != currentMap) {
		TRS staticPosition; // default
		gameObject::ptr newroot
			//= game->state->rootnode
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
				//mapPhysics.clear();
				//mapQueue.clear();

/*
				game->phys->addStaticModels(nullptr,
				                            node,
				                            staticPosition,
				                            mapPhysics);

				compileModels(models);
				*/

				level->reset();
				//game->state->rootnode = node;
				setNode("asyncLoaded", node, std::make_shared<gameObject>());
				setNode("entities", node, game->entities->root);
				setNode("maproot", game->state->rootnode, node);
				//setNode("wfc", node, wfcgen->getNode());
				//mapQueue.add(wfcgen->getNode());

				return true;
			});

			return true;
		});
	}
}

void projalphaView::drawMainMenu(gameMain *game, int wx, int wy) {
	if (nk_begin(nk_ctx, "Main menu", nk_rect(50, 50, 220, 220),
	             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE))
	{
		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "New game")) {
			SDL_Log("New game!");
			input.setMode(modes::NewGame);
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
}

void projalphaView::drawNewGameMenu(gameMain *game, int wx, int wy) {
	bool reset = false;

	if (nk_begin(nk_ctx, "New Game", nk_rect(50, 50, 1180, 600),
	             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE))
	{
		nk_label_wrap(nk_ctx, "Select a class");

		nk_layout_row_dynamic(nk_ctx, 400, 4);
		if (nk_group_begin(nk_ctx, "ExplorerGroup", 0)) {
			nk_layout_row_dynamic(nk_ctx, 0, 1);
			nk_label_wrap(nk_ctx, "A hardy adventurer from the surface, eager to find the thing");
			nk_label_wrap(nk_ctx, "+ Well-rounded, best for beginners");
			nk_label_wrap(nk_ctx, "+ Mid strength, dexterity");
			nk_label_wrap(nk_ctx, "+ Starts with flashlight, 10 flares, and arrows");
			nk_label_wrap(nk_ctx, "> Natural alignment with bunker clan");
			nk_label_wrap(nk_ctx, "> Neutral alignment with other clans and mobs");
			nk_label_wrap(nk_ctx, "- Not very stealthy");
			nk_label_wrap(nk_ctx, "- Low aptitude for special abilities");

			if (nk_button_label(nk_ctx, "Explorer")) {
				SDL_Log("New game!");
				reset = true;
			}

			nk_group_end(nk_ctx);
		}

		if (nk_group_begin(nk_ctx, "ThiefGroup", 0)) {
			nk_layout_row_dynamic(nk_ctx, 0, 1);
			nk_label_wrap(nk_ctx, "An outlaw that steals stuff??");
			nk_label_wrap(nk_ctx, "+ High dexterity");
			nk_label_wrap(nk_ctx, "+ High stealth");
			nk_label_wrap(nk_ctx, "+ Starts with flashlight and throwing stars");
			nk_label_wrap(nk_ctx, "> Mid strength");
			nk_label_wrap(nk_ctx, "> Natural alignment with thieves guild");
			nk_label_wrap(nk_ctx, "- Disliked by all other clans and mobs");
			nk_label_wrap(nk_ctx, "- Unable to do business at non-thief vendors");

			if (nk_button_label(nk_ctx, "Thief")) {
				SDL_Log("New game!");
				reset = true;
			}

			nk_group_end(nk_ctx);
		}

		if (nk_group_begin(nk_ctx, "GhoulGroup", 0)) {
			nk_layout_row_dynamic(nk_ctx, 0, 1);
			nk_label_wrap(nk_ctx, "A cursed being, born in and molded by the darkness");
			nk_label_wrap(nk_ctx, "+ Very high stealth");
			nk_label_wrap(nk_ctx, "+ High dexterity");
			nk_label_wrap(nk_ctx, "+ Very sensitive to light and sound, high visibility");

			nk_label_wrap(nk_ctx, "> Starts with throwable rocks");
			nk_label_wrap(nk_ctx, "> Neutral alignment with mobs");
			nk_label_wrap(nk_ctx, "- Low strength");
			nk_label_wrap(nk_ctx, "- Does not start with a light source");
			nk_label_wrap(nk_ctx, "- Not aligned with any clans");

			if (nk_button_label(nk_ctx, "Ghoul")) {
				SDL_Log("New game!");
				reset = true;
			}

			nk_group_end(nk_ctx);
		}

		if (nk_group_begin(nk_ctx, "CultistGroup", 0)) {
			nk_layout_row_dynamic(nk_ctx, 0, 1);
			nk_label_wrap(nk_ctx, "A devoted follower of the arcane");
			nk_label_wrap(nk_ctx, "+ Mid strength, stealth, and dexterity");
			nk_label_wrap(nk_ctx, "+ Starts with a lamp and daggers");
			nk_label_wrap(nk_ctx, "> Natural alignment with cultist clan");
			nk_label_wrap(nk_ctx, "- Not very stealthy");
			nk_label_wrap(nk_ctx, "- Low aptitude for special abilities");

			if (nk_button_label(nk_ctx, "Cultist")) {
				SDL_Log("New game!");
				reset = true;
			}

			nk_group_end(nk_ctx);
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (nk_button_label(nk_ctx, "Back to main menu")) {
			SDL_Log("Back to main");
			input.setMode(modes::MainMenu);
		}
	}
	nk_end(nk_ctx);

	if (reset) {
		SDL_Log("Selected, loading...");
		level->reset();
		input.setMode(modes::Loading);
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
	auto stats = castEntityComponent<playerInfo*>(game->entities.get(), playerEnt, "playerInfo");

	if (!inv || !stats) return;

	static std::map<entity*, std::string> names;

	if (nk_begin(nk_ctx, "Player inventory", nk_rect(50, 50, 720, 250),
	             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE))
	{
		nk_layout_row_dynamic(nk_ctx, 200, 2);

		if (nk_group_begin(nk_ctx, "invlist", 0)) {
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

				nk_layout_row_dynamic(nk_ctx, 0, 4);
				nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "%lu : %s", items.size(), name);

				Wieldable *w;
				castEntityComponent(w, game->entities.get(), ent, "Wieldable");

				if (w) {
					static const char *wieldClicked = nullptr;

					if (nk_button_label(nk_ctx, "Wield")) {
						SDL_Log("Wield %s", name);
						//stats->primaryWeapon = name;
						wieldClicked = name;
					}

					if (wieldClicked == name) {
						static struct nk_rect s = {20, 100, 220, 90};
						if (nk_popup_begin(nk_ctx, NK_POPUP_STATIC, "Error", 0, s))
						{
							nk_layout_row_dynamic(nk_ctx, 25, 1);
							nk_label(nk_ctx, "Wield", NK_TEXT_LEFT);
							nk_layout_row_dynamic(nk_ctx, 25, 2);
							if (nk_button_label(nk_ctx, "Primary")) {
								stats->primaryWeapon = name;
								wieldClicked = nullptr;
								nk_popup_close(nk_ctx);
							}
							if (nk_button_label(nk_ctx, "Secondary")) {
								stats->secondaryWeapon = name;
								wieldClicked = nullptr;
								nk_popup_close(nk_ctx);
							}
							if (nk_button_label(nk_ctx, "Accessory")) {
								stats->accessory = name;
								wieldClicked = nullptr;
								nk_popup_close(nk_ctx);
							}
							nk_popup_end(nk_ctx);
						} else wieldClicked = nullptr;

					}

					if (nk_button_label(nk_ctx, "Use")) {
						SDL_Log("Using %s", name);

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
			nk_group_end(nk_ctx);
		}

		if (nk_group_begin(nk_ctx, "playerstuff", 0)) {
			nk_layout_row_dynamic(nk_ctx, 0, 1);

			entity *prim = inv->findType(stats->primaryWeapon);
			entity *sec  = inv->findType(stats->secondaryWeapon);
			entity *acc  = inv->findType(stats->accessory);

			std::string pstr = (prim? "" : "(empty) ") + stats->primaryWeapon;
			std::string sstr = (sec?  "" : "(empty) ") + stats->secondaryWeapon;
			std::string astr = (acc?  "" : "(empty) ") + stats->accessory;

			auto useItem = [&] (entity *item) {
				Wieldable *w;
				castEntityComponent(w, game->entities.get(), item, "Wieldable");

				if (w) {
					inv->remove(game->entities.get(), item);
					game->entities->activate(item);
					w->action(game->entities.get(), playerEnt);
				}
			};

			if (nk_button_label(nk_ctx, pstr.c_str()) && prim) {
				SDL_Log("A button!");
				useItem(prim);
			}

			if (nk_button_label(nk_ctx, sstr.c_str()) && sec) {
				SDL_Log("A button!");
				useItem(sec);
			}

			if (nk_button_label(nk_ctx, astr.c_str()) && acc) {
				SDL_Log("A button!");
				useItem(acc);
			}


			nk_group_end(nk_ctx);
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

#include <nuklear/canvas.h>
void projalphaView::drawTileDebug(gameMain *game) {
	entity *playerEnt = findFirst(game->entities.get(), {"player", "inventory"});
	//gameObject::ptr wfcroot = wfcgen->getNode()->getNode("nodes");

	auto floor = getFloor(game, currentFloor);
	// XXX: don't like this one bit
	if (!floor) return;

	gameObject::ptr wfcroot = floor->generator->getNode()->getNode("nodes");

	if (!playerEnt || !wfcroot) return;

	glm::vec3 playerPos = playerEnt->node->getTransformTRS().position;
	std::pair<int, int> playerCoord = {
		int(playerPos.x/4 + 0.5),
		int(playerPos.z/4 + 0.5)
	};

	auto it = floor->generator->omnidijkstra.find(playerCoord);
	if (it == floor->generator->omnidijkstra.end()) {
		// no map
		return;
	}

	auto& tilemap = it->second;

	for (auto& [name, node] : wfcroot->nodes) {
		TRS transform = node->getTransformTRS();
		std::pair<int, int> tileCoord = {
			int(transform.position.x/4 + 0.5),
			int(transform.position.z/4 + 0.5)
		};

		if (floor->generator->traversableMask.get(tileCoord.first, tileCoord.second)) {
		//if (wfcgen->generatedMask.get(tileCoord.first, tileCoord.second)) {
			glm::vec4 screenpos = cam->worldToScreenPosition(transform.position);

			if (cam->onScreen(screenpos)) {
				screenpos.y = 1.0 - screenpos.y;
				screenpos.x *= game->rend->screen_x;
				screenpos.y *= game->rend->screen_y;

				/*
				struct nk_canvas canvas;

				if (nk_canvas_begin(nk_ctx, &canvas, name.c_str(), 0,
				                    screenpos.x, screenpos.y, 64, 32,
				                    nk_rgba(32, 32, 32, 127)))
				{
					nk_draw_text(canvas.painter, nk_rect(screenpos.x,screenpos.y,64,32), "testing this", 12, nk_ctx->style.font, nk_rgba(0xff, 0xff, 0xff, 0x80), nk_rgb(0,0,0));
				}
				nk_canvas_end(nk_ctx, &canvas);
				*/
				float hot = floor->generator->hotpathDistance.get(tileCoord);
				float dist = tilemap.get(tileCoord);

				if (nk_begin(nk_ctx, name.c_str(), nk_rect(screenpos.x, screenpos.y, 64, 32), 0)) {
					//double fps = manager->engine->frame_timer.average();
					//std::string fpsstr = std::to_string(fps) + "fps";
					
					auto foo = [] (float x) {
						char buf[16];
						sprintf(buf, "%.1f", x);
						/*
						return (x == HUGE_VALF)
							? std::string("X")
							: std::string(buf);
							*/
						return std::string(buf);
					};

					//std::string diststr = std::to_string(dist);
					std::string diststr = foo(hot) + ", " + foo(dist);
						/*
						std::to_string(hot) + " : " +
						((dist == INT_MAX)
							? std::string("X")
							: std::to_string(dist));
							*/

					nk_layout_row_dynamic(nk_ctx, 14, 1);
					//nk_label(nk_ctx, "testing this", NK_TEXT_LEFT);
					nk_label(nk_ctx, diststr.c_str(), NK_TEXT_LEFT);
				}
				nk_end(nk_ctx);
			}
		}
	}
}

void projalphaView::drawNavPrompts(gameMain *game, int wx, int wy) {
	int xpos = wx/2 - 64;

	if (nearNode(game, "exit") || nearNode(game, "entry")) {
		int ypos = wy/2 - 100 + 20*sin(4*SDL_GetTicks()/1000.f);

		if (nk_begin(nk_ctx, "[Space]", nk_rect(xpos, ypos, 128, 32), 0)) {
			const char *label = "asdf";

			if (nearNode(game, "exit")) {
				label = "[Space] Descend";
			} else if (nearNode(game, "entry") && currentFloor == 1) {
				label = "[Space] Exit Dungeon";
			} else {
				label = "[Space] Ascend";
			}

			nk_layout_row_dynamic(nk_ctx, 14, 1);
			nk_label(nk_ctx, label, NK_TEXT_CENTERED);
		}
		nk_end(nk_ctx);
	}

	if (nk_begin(nk_ctx, "Current floor", nk_rect(xpos, 48, 128, 32), 0)) {
		nk_layout_row_dynamic(nk_ctx, 14, 1);
		nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "Catacombs 1-%d", currentFloor);
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
		//view->wfcgen->generate(game, {});
	});

	view->level->addInit([=] () {
		entity *playerEnt;
		glm::vec3 pos(-5, 20, -5);
		//gameObject::ptr wfcroot = view->wfcgen->getNode()->getNode("nodes");

		auto floor = view->getFloor(game, view->currentFloor);
		if (floor) {
			pos = floor->entrance;
		}

			/*
		if (wfcroot->hasNode("entry")) {
			TRS transform = wfcroot->getNode("entry")->getTransformTRS();
			// TODO: need a way to calculate the transform from this node
			//pos = transform.position + glm::vec3(0, 20, 0) - glm::vec3(64, 0, 64);
			pos = transform.position + glm::vec3(0, 2, 0);
			//pos = transform.position;
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
