#include <grend/gameMain.hpp>
#include <grend/gameMainDevWindow.hpp>
#include <grend/gameObject.hpp>
//#include <grend/playerView.hpp>
#include <grend/geometryGeneration.hpp>
#include <grend/gameEditor.hpp>
#include <grend/controllers.hpp>

#include <grend/ecs/ecs.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>

#include <memory>
#include <map>
#include <vector>
#include <set>

using namespace grendx;
using namespace grendx::ecs;

#include <logic/projalphaView.hpp>
#include <logic/wfcGenerator.hpp>
#include <logic/UI.hpp>
#include <logic/levelController.hpp>

#include <nuklear/nuklear.h>

void projalphaView::logic(gameMain *game, float delta) {
	// XXX: handle input from end of frame at render() to logic() here
	//      on the next frame... should probably override handleInput()
	nk_input_end(nk_ctx);

	if (input.mode == modes::MainMenu
		|| input.mode == modes::NewGame
		|| input.mode == modes::Intro
		|| input.mode == modes::Pause
		|| input.mode == modes::Won
		)
	{
		// XXX:
		return;
	}

	for (auto& p : floors) {
		p->processMessages();
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
		input.setMode(modes::Intro);
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

	} else if (input.mode == modes::Intro) {
		renderWorld(game, cam, mapQueue, flags);

		// TODO: need to set post size on resize event..
		//post->setSize(winsize_x, winsize_y);
		post->setUniform("exposure", game->rend->exposure);
		post->setUniform("time_ms",  SDL_GetTicks() * 1.f);
		post->draw(game->rend->framebuffer);

		drawIntroWindow(game, winsize_x, winsize_y);

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

// TODO: idk if this is going to be needed, at least not in
//       this project...
//       could consider removing it
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


