#pragma once

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

#include <memory>
#include <chrono>
#include <map>
#include <vector>
#include <set>
#include <functional>
#include <initializer_list>

#include <components/inputHandler.hpp>

#include <logic/wfcGenerator.hpp>
#include <logic/UI.hpp>
#include <logic/levelController.hpp>

#include <nuklear/nuklear.h>

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
		renderQueue mapQueue = renderQueue(cam);

		void incrementFloor(gameMain *game, int amount);
		bool nearNode(gameMain *game, const std::string& name, float thresh = 3.f);

	private:
		void drawMainMenu(gameMain *game, int wx, int wy);
		void drawInventory(gameMain *game, int wx, int wy);
		void drawWinScreen(gameMain *game, int wx, int wy);
		void drawPauseMenu(gameMain *game, int wx, int wy);
		void drawTileDebug(gameMain *game);
};
