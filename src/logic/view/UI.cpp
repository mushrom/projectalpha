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

#include <entities/player.hpp>

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
	auto color = nk_style_item_color(nk_rgb(80, 80, 80));

	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.normal, color);
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.active, color);
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.hover,  color);
	nk_style_push_vec2(nk_ctx, &nk_ctx->style.button.padding, nk_vec2(2, 2));
}

static inline void pushActiveSelectedButton(struct nk_context *nk_ctx) {
	auto color = nk_style_item_color(nk_rgb(100, 100, 100));

	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.normal, color);
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.active, color);
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.hover,  color);
	nk_style_push_vec2(nk_ctx, &nk_ctx->style.button.padding, nk_vec2(2, 2));
}

static inline void pushEnabledButton(struct nk_context *nk_ctx) {
	auto color = nk_style_item_color(nk_rgb(80, 180, 80));

	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.normal, color);
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.active, color);
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.hover,  color);
	nk_style_push_vec2(nk_ctx, &nk_ctx->style.button.padding, nk_vec2(2, 2));
}

static inline void pushHoveredEnabledButton(struct nk_context *nk_ctx) {
	auto color = nk_style_item_color(nk_rgb(90, 130, 90));

	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.normal, color);
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.active, color);
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.hover,  color);
	nk_style_push_vec2(nk_ctx, &nk_ctx->style.button.padding, nk_vec2(2, 2));
}

static inline void pushActiveEnabledButton(struct nk_context *nk_ctx) {
	auto color = nk_style_item_color(nk_rgb(100, 160, 100));

	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.normal, color);
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.active, color);
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.hover,  color);
	nk_style_push_vec2(nk_ctx, &nk_ctx->style.button.padding, nk_vec2(2, 2));
}

static inline void pushDisabledButton(struct nk_context *nk_ctx) {
	auto color = nk_style_item_color(nk_rgb(180, 80, 80));

	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.normal, color);
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.active, color);
	nk_style_push_style_item(nk_ctx, &nk_ctx->style.button.hover,  color);
	nk_style_push_vec2(nk_ctx, &nk_ctx->style.button.padding, nk_vec2(2, 2));
}

static inline void popButtonStyle(struct nk_context *nk_ctx) {
	nk_style_pop_style_item(nk_ctx);
	nk_style_pop_style_item(nk_ctx);
	nk_style_pop_style_item(nk_ctx);
	nk_style_pop_vec2(nk_ctx);
}

static inline
bool selectableButton(struct nk_context *nk_ctx,
                      const char *label,
					  int curIndex,
					  int targetIndex,
                      int *enabled = nullptr,
                      bool inRow = true)
{
	const uint8_t *keystates = SDL_GetKeyboardState(NULL);
	bool hovered = curIndex == targetIndex;
	bool entered = edgeScancode(SDL_SCANCODE_RETURN) || edgeGamepad(SDL_CONTROLLER_BUTTON_A);
	bool pressed = inRow && hovered && entered;

	// TODO: need a way to move the hovered cursor with the mouse
	bool isEnabled = enabled && *enabled == targetIndex;
	if (hovered && isEnabled && inRow)
		pushActiveEnabledButton(nk_ctx);
	else if (hovered && isEnabled)
		pushHoveredEnabledButton(nk_ctx);
	else if (hovered && inRow)
		pushActiveSelectedButton(nk_ctx);
	else if (hovered)
		pushSelectedButton(nk_ctx);
	else if (isEnabled)
		pushEnabledButton(nk_ctx);
	else if (enabled)
		pushDisabledButton(nk_ctx);

	bool ret = nk_button_label(nk_ctx, label) || pressed;
	if (hovered || enabled) popButtonStyle(nk_ctx);

	if (ret) {
		if (enabled) {
			*enabled = targetIndex;
		}

		SDL_Log("Button %d selected", curIndex);
	}

	return ret;
}

static inline bool endSelectableButton(struct nk_context *nk_ctx) {
	return false;
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

#include <initializer_list>
static inline int *buttonNavTableMultirow(int *n,
                                          int maxrowval,
                                          std::initializer_list<int*> indlist)
{
	edgefoo[SDL_SCANCODE_RETURN] = toggleScancode(SDL_SCANCODE_RETURN);
	edgebar[SDL_CONTROLLER_BUTTON_A] = toggleGamepad(SDL_CONTROLLER_BUTTON_A);

	bool up = toggleScancode(SDL_SCANCODE_UP) || toggleGamepad(SDL_CONTROLLER_BUTTON_DPAD_UP);
	bool down = toggleScancode(SDL_SCANCODE_DOWN) || toggleGamepad(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
	bool left = toggleScancode(SDL_SCANCODE_LEFT) || toggleGamepad(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	bool right = toggleScancode(SDL_SCANCODE_RIGHT) || toggleGamepad(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

	int *index = indlist.begin()[*n];

	if (left)  *index = max(0,            *index - 1);
	if (right) *index = min(maxrowval,    *index + 1);
	if (up)    *n = max(0,                *n - 1);
	if (down)  *n = min(indlist.size()-1, *n + 1);

	return indlist.begin()[*n];
}

void projalphaView::drawMainMenu(gameMain *game, int wx, int wy) {
	static int idx = 0;

	int width  = 220;
	int height = 220;

	int center_x = (game->rend->screen_x - width)  / 2;
	int center_y = (game->rend->screen_y - height) / 2;
	bool reset = false;

	if (nk_begin(nk_ctx, "Main menu", nk_rect(center_x, center_y, width, height),
	             0))
	{
		buttonNavUpDown(&idx, 2);

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "New game", idx, 0)) {
			SDL_Log("New game!");
			//input.setMode(modes::NewGame);
			//input.setMode(modes::NewGame);
			reset = true;
		}
		
		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "Settings", idx, 1)) {
			SDL_Log("Settings");
			input.pushMode(modes::Settings);
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "Quit", idx, 2)) {
			SDL_Log("Quiterino");
		}
	}

	if (reset) {
		SDL_Log("Selected, loading...");
		level->reset();
		input.setMode(modes::Loading);
	}

	nk_end(nk_ctx);
}


extern struct nk_image fooimg;

void projalphaView::drawSettings(gameMain *game, int wx, int wy) {
	static int idx = 0;

	int width  = 540;
	int height = 350;

	int center_x = (game->rend->screen_x - width)  / 2;
	int center_y = (game->rend->screen_y - height) / 2;
	bool reset = false;

	static renderSettings settings;
	static bool showSteps = true;

	if (nk_begin(nk_ctx, "Settings", nk_rect(center_x, center_y, width, height),
	             NK_WINDOW_BORDER))
	{

		//nk_layout_row_dynamic(nk_ctx, width, 1);
		nk_layout_row_static(nk_ctx, 64, 64, 2);
		//SDL_Log("asdf: %d\n", fooimg.handle.id);
		if (nk_button_image(nk_ctx, fooimg))
		{
			SDL_Log("asdf");
		}

		nk_image(nk_ctx, fooimg);


		/*
		if (nk_button_label(nk_ctx, "yo")) {
			SDL_Log("ot");
		}
		*/

		/*
		if (nk_button_image_label(nk_ctx, fooimg, "asdfasdfa", NK_TEXT_CENTERED))
		{
			SDL_Log("asdf");
		}
		*/

		// XXX: need a less terrible way to do this
		static int rowidx = 0;
		static int shadowidx = 0;
		static int shadowlvl = 0;
		static int refidx = 0;
		static int reflvl = 0;
		static int msaaidx = 0;
		static int msaalvl = 0;
		static int residx = 0;
		static int reslvl = 0;
		static int scaleidx = 0;
		static int scalelvl = 0;
		static int fogidx = 0;
		static int foglvl = 0;
		static int ditheridx = 0;
		static int ditherlvl = 0;
		static int applyidx = 0;

		int *foo = buttonNavTableMultirow(&rowidx, 4, {
			&shadowidx, &refidx, &msaaidx, &residx, &scaleidx,
			&fogidx, &ditheridx, &applyidx
		});

		nk_layout_row_dynamic(nk_ctx, 0, 5);
		// Shadow settings
		nk_label_wrap(nk_ctx, "Shadows: ");
		if (selectableButton(nk_ctx, "Low", shadowidx, 0, &shadowlvl, rowidx == 0)) {
			SDL_Log("Low setting");
			settings.shadowAtlasSize = 1024;
			settings.shadowSize      = 64;
		}
		
		if (selectableButton(nk_ctx, "Medium", shadowidx, 1, &shadowlvl, rowidx == 0)) {
			SDL_Log("Medium");
			settings.shadowAtlasSize = 2048;
			settings.shadowSize      = 128;
		}

		if (selectableButton(nk_ctx, "High", shadowidx, 2, &shadowlvl, rowidx == 0)) {
			SDL_Log("High setting");
			settings.shadowAtlasSize = 4096;
			settings.shadowSize      = 256;
		}

		if (selectableButton(nk_ctx, "Ultra", shadowidx, 3, &shadowlvl, rowidx == 0)) {
			SDL_Log("Ultra setting");
			settings.shadowAtlasSize = 8192;
			settings.shadowSize      = 512;
		}

		// Reflection settings
		nk_layout_row_dynamic(nk_ctx, 0, 5);
		nk_label_wrap(nk_ctx, "Reflections: ");
		if (selectableButton(nk_ctx, "Low", refidx, 0, &reflvl, rowidx == 1)) {
			SDL_Log("Low setting");
			settings.reflectionAtlasSize = 1024;
			settings.reflectionSize      = 64;
		}
		
		if (selectableButton(nk_ctx, "Medium", refidx, 1, &reflvl, rowidx == 1)) {
			SDL_Log("Medium");
			settings.reflectionAtlasSize = 2048;
			settings.reflectionSize      = 128;
		}

		if (selectableButton(nk_ctx, "High", refidx, 2, &reflvl, rowidx == 1)) {
			SDL_Log("High setting");
			settings.reflectionAtlasSize = 4096;
			settings.reflectionSize      = 256;
		}

		if (selectableButton(nk_ctx, "Ultra", refidx, 3, &reflvl, rowidx == 1)) {
			SDL_Log("Ultra setting");
			settings.reflectionAtlasSize = 8192;
			settings.reflectionSize      = 512;
		}

		// MSAA settings
		nk_layout_row_dynamic(nk_ctx, 0, 5);
		nk_label_wrap(nk_ctx, "MSAA: ");
		if (selectableButton(nk_ctx, "Off", msaaidx, 0, &msaalvl, rowidx == 2)) {
			SDL_Log("MSAA off");
			settings.msaaLevel = 0;
		}
		
		if (selectableButton(nk_ctx, "2x", msaaidx, 1, &msaalvl, rowidx == 2)) {
			SDL_Log("MSAA 2x");
			settings.msaaLevel = 2;
		}

		if (selectableButton(nk_ctx, "4x", msaaidx, 2, &msaalvl, rowidx == 2)) {
			SDL_Log("MSAA 4x");
			settings.msaaLevel = 4;
		}

		if (selectableButton(nk_ctx, "8x", msaaidx, 3, &msaalvl, rowidx == 2)) {
			SDL_Log("MSAA 8x");
			settings.msaaLevel = 8;
		}

		nk_layout_row_dynamic(nk_ctx, 0, 5);
		nk_label_wrap(nk_ctx, "Target resolution: ");
		if (selectableButton(nk_ctx, "Auto", residx, 0, &reslvl, rowidx == 3)) {
			SDL_Log("Auto resolution");
			settings.targetResX = game->rend->screen_x;
			settings.targetResY = game->rend->screen_y;
		}

		if (selectableButton(nk_ctx, "720p", residx, 1, &reslvl, rowidx == 3)) {
			SDL_Log("720p resolution");
			settings.targetResX = 1280;
			settings.targetResY = 720;
		}

		if (selectableButton(nk_ctx, "768p", residx, 2, &reslvl, rowidx == 3)) {
			SDL_Log("768p resolution");
			settings.targetResX = 1366;
			settings.targetResY = 768;
		}

		if (selectableButton(nk_ctx, "1080p", residx, 3, &reslvl, rowidx == 3)) {
			SDL_Log("1080p resolution");
			settings.targetResX = 1920;
			settings.targetResY = 1080;
		}

		nk_layout_row_dynamic(nk_ctx, 0, 5);
		nk_label_wrap(nk_ctx, "Resolution scale: ");
		if (selectableButton(nk_ctx, "50%", scaleidx, 0, &scalelvl, rowidx == 4)) {
			SDL_Log("resolution scale 50%%");
			settings.scaleX = settings.scaleY = 0.5;
		}

		if (selectableButton(nk_ctx, "71%", scaleidx, 1, &scalelvl, rowidx == 4)) {
			SDL_Log("resolution scale 71%%");
			settings.scaleX = settings.scaleY = 0.7071;
		}

		if (selectableButton(nk_ctx, "80%", scaleidx, 2, &scalelvl, rowidx == 4)) {
			SDL_Log("resolution scale 80%%");
			settings.scaleX = settings.scaleY = 0.8;
		}

		if (selectableButton(nk_ctx, "100%", scaleidx, 3, &scalelvl, rowidx == 4)) {
			SDL_Log("resolution scale 100%%");
			settings.scaleX = settings.scaleY = 1.0;
		}

		nk_layout_row_dynamic(nk_ctx, 0, 5);
		nk_label_wrap(nk_ctx, "Fog quality: ");
		if (selectableButton(nk_ctx, "Off", fogidx, 0, &foglvl, rowidx == 5)) {
			SDL_Log("Fog off");
		}

		if (selectableButton(nk_ctx, "6x",  fogidx, 1, &foglvl, rowidx == 5)) {
			SDL_Log("Fog 6x");
		}

		if (selectableButton(nk_ctx, "16x", fogidx, 2, &foglvl, rowidx == 5)) {
			SDL_Log("Fog 16x");
		}

		if (selectableButton(nk_ctx, "32x", fogidx, 3, &foglvl, rowidx == 5)) {
			SDL_Log("Fog 32x");
		}

		nk_layout_row_dynamic(nk_ctx, 0, 5);
		nk_label_wrap(nk_ctx, "Dithering: ");
		if (selectableButton(nk_ctx, "Off", ditheridx, 0, &ditherlvl, rowidx == 6)) {
			SDL_Log("Fog off");
		}

		if (selectableButton(nk_ctx, "On",  ditheridx, 1, &ditherlvl, rowidx == 6)) {
			SDL_Log("Fog 6x");
		}

		if (selectableButton(nk_ctx, "On",  ditheridx, 2, &ditherlvl, rowidx == 6)) {
			SDL_Log("Fog 16x");
		}

		if (selectableButton(nk_ctx, "On",  ditheridx, 3, &ditherlvl, rowidx == 6)) {
			SDL_Log("Fog 32x");
		}


		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "Apply", idx, 0, nullptr, rowidx == 7)) {
			SDL_Log("Applied");
			input.popMode();

			game->rend->applySettings(settings);
			invalidateLightMaps(game->state->rootnode);
		}
	}

	nk_end(nk_ctx);
}

void projalphaView::drawNewGameMenu(gameMain *game, int wx, int wy) {
	static int idx;
	bool reset = false;

	if (nk_begin(nk_ctx, "New Game", nk_rect(50, 50, 1180, 600), 0)) {
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

	if (nk_begin(nk_ctx, "Welcome", nk_rect(200, 200, 800, 480), 0)) {
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

	if (nk_begin(nk_ctx, "Pause", nk_rect(50, 50, 220, 220), 0)) {
		buttonNavUpDown(&idx, 3);

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "Resume", idx, 0)) {
			SDL_Log("Resuming");
			input.setMode(modes::Move);
		}

		nk_layout_row_dynamic(nk_ctx, 0, 1);
		if (selectableButton(nk_ctx, "Settings", idx, 1)) {
			SDL_Log("Settings");
			input.pushMode(modes::Settings);
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
	entity *playerEnt = findFirstTypes<player, inventory>(game->entities.get());
	if (!playerEnt) return;

	auto inv   = getComponent<inventory>(game->entities.get(), playerEnt);
	auto stats = getComponent<playerInfo>(game->entities.get(), playerEnt);
	//auto inv = castEntityComponent<inventory*>(game->entities.get(), playerEnt, "inventory");
	//auto stats = castEntityComponent<playerInfo*>(game->entities.get(), playerEnt, "playerInfo");

	if (!inv || !stats) return;

	static std::map<entity*, std::string> names;
	static int idx = 0;
	int buttIdx = 0;

	if (nk_begin(nk_ctx, "Player inventory", nk_rect(50, 50, 720, 250), 0)) {
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

				Wieldable *w = getComponent<Wieldable>(game->entities.get(), ent);
				//Wieldable *w;
				//castEntityComponent(w, game->entities.get(), ent, "Wieldable");

				if (w) {
					static const char *wieldClicked = nullptr;

					if (selectableButton(nk_ctx, "A", idx, buttIdx++)) {
						SDL_Log("Wield primary %s", name);
						stats->primaryWeapon = name;
					}

					if (selectableButton(nk_ctx, "B", idx, buttIdx++)) {
						SDL_Log("Wield secondary %s", name);
						stats->secondaryWeapon = name;
					}

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
				Wieldable *w = getComponent<Wieldable>(game->entities.get(), item);
				//Wieldable *w;
				//castEntityComponent(w, game->entities.get(), item, "Wieldable");

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
	if (nk_begin(nk_ctx, "You are winnar!", nk_rect(wx/2, wy/2, 220, 220), 0))
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
	entity *playerEnt = findFirstTypes<player, inventory>(game->entities.get());

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
