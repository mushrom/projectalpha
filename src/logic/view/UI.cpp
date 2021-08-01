#include <grend/ecs/ecs.hpp>
#include <logic/projalphaView.hpp>
#include <nuklear/nuklear.h>

// for inventory/action stufff
#include <components/itemPickup.hpp>
#include <components/playerInfo.hpp>
#include <components/inventory.hpp>
#include <entities/healthPickup.hpp>

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
