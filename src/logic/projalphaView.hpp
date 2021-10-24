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
#include <grend/ecs/message.hpp>

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
			Settings,
			NewGame,
			Intro,
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
		int currentFloor = -1;
		float zoom = 20.f;
		bool debugTiles = false;

		struct floorStates {
			typedef std::shared_ptr<floorStates> ptr;
			typedef std::weak_ptr<floorStates>   weakptr;
			typedef std::unique_ptr<floorStates> uniqueptr;

			floorStates(const floorStates& other)
				//: mapQueue(other.mapQueue)
			{
				zone = other.zone;
				generator = other.generator;
				mapPhysics = other.mapPhysics;
				levelEntities = other.levelEntities;

				entrance = other.entrance;
				exit = other.exit;
			}

			floorStates(gameMain *game,
			            projalphaView *view,
			            std::string z,
			            wfcSpec::ptr spec);

			void processMessages(void);

			std::string zone;

			std::shared_ptr<wfcGenerator> generator;
			std::vector<physicsObject::ptr> mapPhysics;
			std::set<entity*> levelEntities;
			// pre-built queue for faster drawing
			// renderQueue mapQueue;

			glm::vec3 entrance, exit;
			messaging::mailbox::ptr pickupEvents;
		};

		// TODO: would be good to keep a map of different zones
		wfcSpec::ptr tunnelSpec;
		wfcSpec::ptr bunkerSpec;
		wfcSpec::ptr guildSpec;
		wfcSpec::ptr cultSpec;
		wfcSpec::ptr cellarSpec;
		wfcSpec::ptr ossuarySpec;
		wfcSpec::ptr troveSpec;

		std::unique_ptr<levelController> level;
		//landscapeGenerator landscape;
		//std::unique_ptr<wfcGenerator> wfcgen;
		inputHandlerSystem::ptr inputSystem;
		std::string currentMap = "no map!";
		std::string loadedMap = "no map loaded either!";
		std::vector<floorStates::uniqueptr> floors;
		//std::vector<physicsObject::ptr> mapPhysics;
		//std::vector<entity*> levelEntities;
		renderQueue mapQueue = renderQueue(cam);

		void incrementFloor(gameMain *game, int amount);
		floorStates *getFloor(gameMain *game, int n);
		bool nearNode(gameMain *game, const std::string& name, float thresh = 3.f);

		inline wfcGenerator* getGenerator(void) {
			// check that currentFloor has a valid floor state
			return (floors.empty() || currentFloor < 0 || currentFloor >= (int)floors.size())
				? nullptr
				: floors[currentFloor]->generator.get();
		}

	private:
		void drawMainMenu(gameMain *game, int wx, int wy);
		void drawSettings(gameMain *game, int wx, int wy);
		void drawNewGameMenu(gameMain *game, int wx, int wy);
		void drawIntroWindow(gameMain *game, int wx, int wy);
		void drawInventory(gameMain *game, int wx, int wy);
		void drawWinScreen(gameMain *game, int wx, int wy);
		void drawPauseMenu(gameMain *game, int wx, int wy);
		void drawTileDebug(gameMain *game);
		void drawNavPrompts(gameMain *game, int wx, int wy);
};
