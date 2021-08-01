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

projalphaView::projalphaView(gameMain *game)
	: gameView(),
	  level(new levelController)
{
	spec = std::make_shared<wfcSpec>(
		game,
		DEMO_PREFIX "assets/obj/catacomb-tiles/wfc-config.json"
	);

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
                                        wfcSpec::ptr spec)
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
}

projalphaView::floorStates* projalphaView::getFloor(gameMain *game, int n) {
	if (n < 0) return nullptr;

	if (n < floors.size()) {
		return &floors[n];
	}

	// otherwise, have to generate a new floor
	// TODO: what happens if there's a jump larger than one level, not
	//       just pushing to the back?

	floorStates foo(game, this, "catacombs", spec);
	//SDL_Log("Generated floor, map queue has %lu meshes", foo.mapQueue.meshes.size());

	floors.push_back(foo);
	return &floors.back();
}

void projalphaView::incrementFloor(gameMain *game, int amount) {
	int nextFloor = currentFloor + amount;
	auto cur  = getFloor(game, currentFloor);
	auto next = getFloor(game, nextFloor);

	if (cur) {
		/* deactivate stuff */;
		cur->generator->mapobjs.clear();

		// XXX
		// TODO: need to remove items from level entities when they're picked up...
		//       and deleted, need to re-add them to the current level when they're
		//       dropped from inventory, hmmmmm... not sure how to approach this
		for (auto& e : cur->levelEntities) {
			if (game->entities->valid(e)) {
				game->entities->deactivate(e);
			}
		}
	}

	if (next) {
		/* activate stuff */
		next->generator->mapobjs.clear();
		next->generator->setPosition(game, glm::vec3(0));
		mapQueue.clear();
		mapQueue.add(next->generator->getNode());
		SDL_Log("Built queue: %lu meshes\n", mapQueue.meshes.size());

		for (auto& e : next->levelEntities) {
			if (game->entities->valid(e)) {
				game->entities->activate(e);
			}
		}

		entity *playerEnt = findFirst(game->entities.get(), {"player"});
		if (playerEnt) {
			TRS t;
			t.position = (amount > 0)? next->entrance : next->exit;
			// XXX: avoid falling below staircases
			t.position += glm::vec3(0, 2, 0);
			updateEntityTransforms(game->entities.get(), playerEnt, t);
			SDL_Log("Setting player transform");
		}
	};

	currentFloor = nextFloor;
}

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
			glm::vec4 screenpos = cam->worldToScreenPosition(transform.position);

			if (cam->onScreen(screenpos)) {
				screenpos.y = 1.0 - screenpos.y;
				screenpos.x *= game->rend->screen_x;
				screenpos.y *= game->rend->screen_y;

				float hot = floor->generator->hotpathDistance.get(tileCoord);
				float dist = tilemap.get(tileCoord);

				if (nk_begin(nk_ctx, name.c_str(), nk_rect(screenpos.x, screenpos.y, 64, 32), 0)) {
					auto foo = [] (float x) {
						char buf[16];
						sprintf(buf, "%.1f", x);
						return std::string(buf);
					};

					std::string diststr = foo(hot) + ", " + foo(dist);

					nk_layout_row_dynamic(nk_ctx, 14, 1);
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

