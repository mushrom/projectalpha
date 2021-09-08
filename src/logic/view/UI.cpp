#include <grend/ecs/ecs.hpp>
#include <logic/projalphaView.hpp>
#include <nuklear/nuklear.h>

// for inventory/action stufff
#include <components/itemPickup.hpp>
#include <components/playerInfo.hpp>
#include <components/inventory.hpp>
#include <entities/items/items.hpp>
#include <logic/globalMessages.hpp>
#include <logic/gameController.hpp>

static uint8_t foo[SDL_NUM_SCANCODES];
static uint8_t bar[SDL_NUM_SCANCODES];
static uint8_t edgefoo[SDL_NUM_SCANCODES];
static uint8_t edgebar[SDL_NUM_SCANCODES];

static inline bool toggleScancode(int n) {
	const uint8_t *keystates = SDL_GetKeyboardState(NULL);

	if (keystates[n] && !foo[n]) {
		foo[n] = 1;
		return true;

	} else if (!keystates[n] && foo[n]) {
		foo[n] = 0;
		return false;
	}

	return false;
}

static inline bool edgeScancode(int n) {
	const uint8_t *keystates = SDL_GetKeyboardState(NULL);

	return keystates[n] && edgefoo[n];
}

static inline bool toggleGamepad(SDL_GameControllerButton n) {
	if (!Controller()) return false;

	bool pressed = SDL_GameControllerGetButton(Controller(), n);

	if (pressed && !bar[n]) {
		bar[n] = 1;
		return true;
	}

	else if (!pressed && bar[n]) {
		bar[n] = 0;
		return false;
	}

	return false;
}

static inline bool edgeGamepad(SDL_GameControllerButton n) {
	if (!Controller()) return false;
	bool pressed = SDL_GameControllerGetButton(Controller(), n);

	return pressed && edgebar[n];
}

static inline void pushSelectedButton(struct nk_context *nk_ctx) {
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.normal, nk_style_item_color(nk_rgb(255, 0, 0)));
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.active, nk_style_item_color(nk_rgb(255, 0, 0)));
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.hover, nk_style_item_color(nk_rgb(255, 0, 0)));
	nk_style_push_vec2(nk_ctx, &nk_ctx->style.button.padding, nk_vec2(2, 2));
}

static inline void popSelectedButton(struct nk_context *nk_ctx) {
	nk_style_pop_style_item(nk_ctx);
	nk_style_pop_style_item(nk_ctx);
	nk_style_pop_style_item(nk_ctx);
	nk_style_pop_vec2(nk_ctx);
}

static inline
bool selectableButton(struct nk_context *nk_ctx,
                      const char *label,
					  int curIndex,
					  int targetIndex)
{
	const uint8_t *keystates = SDL_GetKeyboardState(NULL);
	bool hovered = curIndex == targetIndex;
	bool entered = edgeScancode(SDL_SCANCODE_RETURN) || edgeGamepad(SDL_CONTROLLER_BUTTON_A);
	bool pressed = hovered && entered;

	if (hovered) pushSelectedButton(nk_ctx);
	//bool ret = nk_button_label(nk_ctx, label) || pressed;
	//popSelectedButton(nk_ctx);
	bool ret = nk_button_label(nk_ctx, label) || pressed;
	if (hovered) popSelectedButton(nk_ctx);

	if (ret) {
		SDL_Log("Button %d selected", curIndex);
	}
	return ret;
}

static inline bool endSelectableButton(struct nk_context *nk_ctx) {
	//popSelectedButton(nk_ctx);
}

static inline void buttonNavUpDown(int *n, int maxval) {
	edgefoo[SDL_SCANCODE_RETURN] = toggleScancode(SDL_SCANCODE_RETURN);
	edgebar[SDL_CONTROLLER_BUTTON_A] = toggleGamepad(SDL_CONTROLLER_BUTTON_A);

	if (toggleScancode(SDL_SCANCODE_UP))   *n = max(0,      *n - 1);
	if (toggleScancode(SDL_SCANCODE_DOWN)) *n = min(maxval, *n + 1);

	if (toggleGamepad(SDL_CONTROLLER_BUTTON_DPAD_UP))
		*n = max(0, *n - 1);

	if (toggleGamepad(SDL_CONTROLLER_BUTTON_DPAD_DOWN))
		*n = min(maxval, *n + 1);
}

static inline void buttonNavTable(int *n, int maxval, int stride) {
	edgefoo[SDL_SCANCODE_RETURN] = toggleScancode(SDL_SCANCODE_RETURN);
	edgebar[SDL_CONTROLLER_BUTTON_A] = toggleGamepad(SDL_CONTROLLER_BUTTON_A);

	bool up = toggleScancode(SDL_SCANCODE_UP) || toggleGamepad(SDL_CONTROLLER_BUTTON_DPAD_UP);
	bool down = toggleScancode(SDL_SCANCODE_DOWN) || toggleGamepad(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
	bool left = toggleScancode(SDL_SCANCODE_LEFT) || toggleGamepad(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	bool right = toggleScancode(SDL_SCANCODE_RIGHT) || toggleGamepad(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

	if (left)  *n = max(0,      *n - 1);
	if (right) *n = min(maxval, *n + 1);
	if (up)    *n = max(0,      *n - stride);
	if (down)  *n = min(maxval, *n + stride);
}


void projalphaView::drawMainMenu(gameMain *game, int wx, int wy) {
	static int idx = 0;

	if (nk_begin(nk_ctx, "Main menu", nk_rect(50, 50, 220, 220),
	             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE))
	{
		buttonNavUpDown(&idx, 2);

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "New game", idx, 0)) {
			SDL_Log("New game!");
			input.setMode(modes::NewGame);
		}
		
		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "Settings", idx, 1)) {
			SDL_Log("Settings");
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "Quit", idx, 2)) {
			SDL_Log("Quiterino");
		}
	}
	nk_end(nk_ctx);
}

void projalphaView::drawNewGameMenu(gameMain *game, int wx, int wy) {
	static int idx;
	bool reset = false;

	if (nk_begin(nk_ctx, "New Game", nk_rect(50, 50, 1180, 600),
	             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE))
	{
		buttonNavTable(&idx, 4, 4);
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

			if (selectableButton(nk_ctx, "Explorer", idx, 0)) {
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

			if (selectableButton(nk_ctx, "Thief", idx, 1)) {
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

			if (selectableButton(nk_ctx, "Ghoul", idx, 2)) {
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

			if (selectableButton(nk_ctx, "Cultist", idx, 3)) {
				SDL_Log("New game!");
				reset = true;
			}

			nk_group_end(nk_ctx);
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "Back to main menu", idx, 4)) {
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

void projalphaView::drawIntroWindow(gameMain *game, int wx, int wy) {
	static int idx = 0;

	if (nk_begin(nk_ctx, "Welcome", nk_rect(200, 200, 800, 480),
	             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE))
	{
		buttonNavUpDown(&idx, 0);

		nk_layout_row_static(nk_ctx, 100, 480, 1);
		nk_label_wrap(nk_ctx,
			"Legend has it that long ago, some dude hid a "
			"priceless amulet in here somewhere for safe keeping, "
			"but was unable to recover it. If you find it you "
			"would make big monies... possibly magic? I don't know"
		);

		nk_layout_row_dynamic(nk_ctx, 0, 1);

		nk_label_wrap(nk_ctx, "CONTROLS: ");
		nk_label_wrap(nk_ctx, " W, A, S, D - Move up, left, down, right");
		nk_label_wrap(nk_ctx, "      Space - Jump");
		nk_label_wrap(nk_ctx, "      Mouse - Look around");
		nk_label_wrap(nk_ctx, " Left click - Primary action (bindable)");
		nk_label_wrap(nk_ctx, "Right click - Secondary action (bindable)");
		nk_label_wrap(nk_ctx, "        Tab - Manage inventory");
		nk_label_wrap(nk_ctx, "     Escape - Pause");

		if (selectableButton(nk_ctx, "Ok cool", idx, 0)) {
			SDL_Log("Cool");
			input.setMode(modes::Move);
		}
	}
	nk_end(nk_ctx);
}

void projalphaView::drawPauseMenu(gameMain *game, int wx, int wy) {
	static int idx = 0;

	if (nk_begin(nk_ctx, "Pause", nk_rect(50, 50, 220, 220),
	    NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE))
	{
		buttonNavUpDown(&idx, 3);

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "Resume", idx, 0)) {
			SDL_Log("Resuming");
			input.setMode(modes::Move);
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "Settings", idx, 1)) {
			SDL_Log("Settings");
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "Abandon run", idx, 2)) {
			SDL_Log("Abandon run");
			input.setMode(modes::MainMenu);
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "Quit", idx, 3)) {
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
	static int idx = 0;
	int buttIdx = 0;

	if (nk_begin(nk_ctx, "Player inventory", nk_rect(50, 50, 720, 250),
	             NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE))
	{
		buttonNavTable(&idx, 4*inv->items.size(), 4);

		nk_layout_row_dynamic(nk_ctx, 200, 2);
		if (nk_group_begin(nk_ctx, "invlist", 0)) {
			nk_layout_row_dynamic(nk_ctx, 0, 1);
			if (selectableButton(nk_ctx, "a button", idx, buttIdx++)) {
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

				nk_layout_row_dynamic(nk_ctx, 0, 5);
				nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "%lu : %s", items.size(), name);

				Wieldable *w;
				castEntityComponent(w, game->entities.get(), ent, "Wieldable");

				if (w) {
					static const char *wieldClicked = nullptr;

					if (selectableButton(nk_ctx, "A", idx, buttIdx++)) {
						SDL_Log("Wield primary %s", name);
						stats->primaryWeapon = name;
						//stats->primaryWeapon = name;
						//wieldClicked = name;
					}

					if (selectableButton(nk_ctx, "B", idx, buttIdx++)) {
						SDL_Log("Wield secondary %s", name);
						stats->secondaryWeapon = name;
						//stats->primaryWeapon = name;
						//wieldClicked = name;
					}

					/*
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
					*/

					if (selectableButton(nk_ctx, "Use", idx, buttIdx++)) {
						SDL_Log("Using %s", name);

						inv->remove(game->entities.get(), ent);
						game->entities->activate(ent);
						w->action(game->entities.get(), playerEnt);

						Messages()->publish({
							.type = "itemDropped",
							.ent  = ent,
							.comp = playerEnt,
						});
					}
				}

				if (selectableButton(nk_ctx, "Drop", idx, buttIdx++)) {
					inv->remove(game->entities.get(), ent);
					game->entities->activate(ent);
					TRS newtrans = playerEnt->node->getTransformTRS();
					newtrans.position += glm::vec3(3, 0, 3);
					ent->node->setTransform(newtrans);

					Messages()->publish({
						.type = "itemDropped",
						.ent  = ent,
						.comp = playerEnt,
					});
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

	auto floor = getFloor(game, currentFloor);

	if (floor) {
		if (nk_begin(nk_ctx, "Current floor", nk_rect(xpos, 48, 128, 32), 0)) {
			nk_layout_row_dynamic(nk_ctx, 14, 1);
			nk_labelf(nk_ctx, NK_TEXT_ALIGN_LEFT, "%s-%d", floor->zone.c_str(), currentFloor);
		}
	}
	nk_end(nk_ctx);
}
