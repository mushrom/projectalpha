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

#include <logic/gameController.hpp>
#include <logic/lootSystem.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>

#include <memory>
#include <map>
#include <vector>
#include <set>
#include <functional>

// XXX:  toggle using textures/models I have locally, don't want to
//       bloat the assets folder again
#define LOCAL_BUILD 0

using namespace grendx;
using namespace grendx::ecs;

// NOTE: need includes of basically all the ECS stuff here 
//       because serializers are defined in the projalphaView
//       constructor
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
	tunnelSpec = std::make_shared<wfcSpec>(
		game,
		DEMO_PREFIX "assets/obj/catacomb-tiles/wfc-config.json"
	);

	// XXX: all using the same spec for now 
	bunkerSpec = guildSpec = cultSpec = cellarSpec
		= ossuarySpec = troveSpec = tunnelSpec;

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
		{loadPostShader(GR_PREFIX "shaders/baked/texpresent.frag",
		                game->rend->globalShaderOptions)},
		SCREEN_SIZE_X, SCREEN_SIZE_Y));
	/*
	post = renderPostChain::ptr(new renderPostChain(
		{
		loadPostShader(GR_PREFIX "shaders/baked/fog-volumetric.frag",
		               game->rend->globalShaderOptions),
			//game->rend->postShaders["fog-depth"],
			//game->rend->postShaders["psaa"],
		},
		//{game->rend->postShaders["tonemap"], game->rend->postShaders["psaa"]},
		SCREEN_SIZE_X, SCREEN_SIZE_Y));
		*/
#endif

#if 0
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
#endif

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
		= killedParticles::ptr(new killedParticles({getTypeName<enemy>()}));
		//= killedParticles::ptr(new killedParticles({"enemy"}));
	game->entities->removeEvents["lootDrop"]
		//= std::make_shared<lootSystem>(std::vector<const char *> {"enemy"});
		= std::make_shared<lootSystem>(std::vector<const char *> {getTypeName<enemy>()});
	game->entities->removeEvents["spawnerParticles"]
		//= killedParticles::ptr(new killedParticles({"enemySpawner"}));
		= killedParticles::ptr(new killedParticles({getTypeName<enemySpawner>()}));
	game->entities->removeEvents["playerParticles"]
		= killedParticles::ptr(new killedParticles({getTypeName<player>()}));
		//= killedParticles::ptr(new killedParticles({"player"}));
	game->entities->removeEvents["barrelParticles"]
		= killedParticles::ptr(new killedParticles({getTypeName<explodyBarrel>()}));
		//= killedParticles::ptr(new killedParticles({"explodyBarrel"}));

	inputSystem = std::make_shared<inputHandlerSystem>();
	game->entities->systems["input"] = inputSystem;

	bindCookedMeshes();
	input.bind(MODAL_ALL_MODES, resizeInputHandler(game, post));
	input.bind(MODAL_ALL_MODES, [=, this] (SDL_Event& ev, unsigned flags) {
		nk_sdl_handle_event(&ev);
		return MODAL_NO_CHANGE;
	});

	input.bind(MODAL_ALL_MODES, [=, this] (SDL_Event& ev, unsigned flags) {
		// XXX
		updateController(ev);
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
	//input.bind(MODAL_ALL_MODES, controller::camMovement2D(cam, 15.f));
	input.bind(modes::Move, controller::camScrollZoom(cam, &zoom, 3.f));
	input.bind(modes::Move, inputMapper(inputSystem->inputs, cam));
	input.bind(MODAL_ALL_MODES, camMovement2D(inputSystem->inputs, cam, 15.f));
#endif

	input.bind(modes::Move,
		controller::camAngled2DRotatable(cam, game,
		                                 -M_PI/2.f, -M_PI/2.f, -M_PI/2.f));
	input.bind(modes::Move, [=, this] (SDL_Event& ev, unsigned flags) {
		inputSystem->handleEvent(game->entities.get(), ev);
		return MODAL_NO_CHANGE;
	});

	input.bind(modes::Move, [=, this] (SDL_Event& ev, unsigned flags) {
		if ((ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_SPACE)
			|| (ev.type == SDL_CONTROLLERBUTTONDOWN && ev.cbutton.button == SDL_CONTROLLER_BUTTON_X))
		{
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

		if (ev.type == SDL_CONTROLLERBUTTONUP && ev.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
			return (int)modes::Move;
		}

		return MODAL_NO_CHANGE;
	});

	// TODO: configurable keybinds
	input.bind(modes::Move, [=, this] (SDL_Event& ev, unsigned flags) {
		if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_TAB) {
			return (int)modes::Inventory;
		}

		if (ev.type == SDL_CONTROLLERBUTTONDOWN && ev.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
			return (int)modes::Inventory;
		}

		return MODAL_NO_CHANGE;
	});

	input.bind(modes::Move, [=, this] (SDL_Event& ev, unsigned flags) {
		if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) {
			return (int)modes::Pause;
		}

		if (ev.type == SDL_CONTROLLERBUTTONDOWN && ev.cbutton.button == SDL_CONTROLLER_BUTTON_START)
		{
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
