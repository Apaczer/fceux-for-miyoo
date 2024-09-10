// Externals
extern Config *g_config;

/* MENU COMMANDS */

// Use PAL or NTSC rate
static void pal_update(unsigned long key) {
	int val;
	g_config->getOption("SDL.PAL", &val);
	if (key == DINGOO_RIGHT)
		val = val < 2 ? val + 1 : 2;
	if (key == DINGOO_LEFT)
		val = val > 0 ? val - 1 : 0;

	g_config->setOption("SDL.PAL", val);
}
// Use PAL or NTSC rate
static void autoresume_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.AutoResume", val);
	FCEUD_SetAutoResume(val);
}

// TODO - Open game genie screen
static void gg_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.GameGenie", val);
}

static void sprite_limit_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.DisableSpriteLimit", val);
}

static void throttle_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.FPSThrottle", val);
#ifdef FRAMESKIP
	int frameskip;
	g_config->getOption("SDL.Frameskip", &frameskip);
	if (val && frameskip < 0) g_config->setOption("SDL.Frameskip", 0);
#endif
}

static void showfps_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.ShowFPS", val);
}

static void show_mouse_update(unsigned long key) {
	int val;

	if (key == DINGOO_RIGHT)
		val = 1;
	if (key == DINGOO_LEFT)
		val = 0;

	g_config->setOption("SDL.ShowMouseCursor", val);
}

static void mouse_update(unsigned long key) {
	int val;
	g_config->getOption("SDL.MouseSpeed", &val);

	if (key == DINGOO_RIGHT)
		val = val < 8 ? val + 1 : 8;
	if (key == DINGOO_LEFT)
		val = val > 0 ? val - 1 : 0;

	g_config->setOption("SDL.MouseSpeed", val);
}

// Frameskip
#ifdef FRAMESKIP
static void frameskip_update(unsigned long key) {
	int val;
	g_config->getOption("SDL.Frameskip", &val);

	if (key == DINGOO_RIGHT) val = val < 11 ? val+1 : -1;
	if (key == DINGOO_LEFT) val = val > -1 ? val-1 : 11;

	g_config->setOption("SDL.Frameskip", val);
	if (val<0) g_config->setOption("SDL.FPSThrottle", 0);
}
#endif

/* MAIN SETTINGS MENU */

static SettingEntry
	st_menu[] = {
		{ "PAL", "Change TV system", "SDL.PAL", pal_update },
		{ "Auto-Resume", "Auto-Resume Play", "SDL.AutoResume", autoresume_update },
		{ "Game Genie", "Emulate Game Genie", "SDL.GameGenie", gg_update },
		{ "No sprite limit", "Disable sprite limit", "SDL.DisableSpriteLimit", sprite_limit_update },
		{ "FPS Throttle", "Use fps throttling", "SDL.FPSThrottle", throttle_update },
#ifdef FRAMESKIP
		{ "Frameskip", "Frameskip [Auto/0-11]", "SDL.Frameskip", frameskip_update},
#endif
		{ "Show FPS", "Show frames per second", "SDL.ShowFPS", showfps_update },
		{ "Show mouse", "Show/hide mouse cursor", "SDL.ShowMouseCursor", show_mouse_update },
		{ "Mouse speed", "Mouse cursor speed", "SDL.MouseSpeed", mouse_update },
};
static int st_menu_items = sizeof(st_menu) / sizeof(st_menu[0]);

int RunMainSettings() {
	static int index = 0;
	static int spy = 72;
	int done = 0, y, i;

	int max_entries = 9;
	int menu_size = st_menu_items;

	static int offset_start = 0;
	static int offset_end = menu_size > max_entries ? max_entries : menu_size;

	char tmp[32];
	int itmp;

	g_dirty = 1;
	while (!done) {
		// Parse input
		readkey();
		if (parsekey(DINGOO_B))
			done = 1;
		if (parsekey(DINGOO_UP, 1)) {
			if (index > 0) {
				index--;

				if (index >= offset_start)
					spy -= 15;

				if ((offset_start > 0) && (index < offset_start)) {
					offset_start--;
					offset_end--;
				}
			} else {
				index = menu_size-1;
				offset_end = menu_size;
				offset_start = menu_size <= max_entries ? 0 : offset_end - max_entries;
				spy = 72 + 15*(index - offset_start);
			}
		}

		if (parsekey(DINGOO_DOWN, 1)) {
			if (index < (menu_size - 1)) {
				index++;

				if (index < offset_end)
					spy += 15;

				if ((offset_end < menu_size) && (index >= offset_end)) {
					offset_end++;
					offset_start++;
				}
			} else {
				index = 0;
				offset_start = 0;
				offset_end = menu_size <= max_entries ? menu_size : max_entries;
				spy = 72;
			}
		}

		if (parsekey(DINGOO_LEFT, 1) || parsekey(DINGOO_RIGHT, 1) || parsekey(
				DINGOO_A))
			st_menu[index].update(g_key);

		// Draw stuff
		if (g_dirty) {
			draw_bg(g_bg);
			
			//Draw Top and Bottom Bars
			DrawChar(gui_screen, SP_SELECTOR, 0, 37);
			DrawChar(gui_screen, SP_SELECTOR, 81, 37);
			DrawChar(gui_screen, SP_SELECTOR, 0, 225);
			DrawChar(gui_screen, SP_SELECTOR, 81, 225);
			DrawText(gui_screen, "B - Go Back", 235, 225);
			DrawChar(gui_screen, SP_LOGO, 12, 9);
			
			// Draw selector
			DrawChar(gui_screen, SP_SELECTOR, 56, spy);
			DrawChar(gui_screen, SP_SELECTOR, 77, spy);

			DrawText(gui_screen, "Main Settings", 8, 37);

			// Draw menu
			for (i = offset_start, y = 72; i < offset_end; i++, y += 15) {
				DrawText(gui_screen, st_menu[i].name, OPTION_LABEL_COLUMN, y);

				g_config->getOption(st_menu[i].option, &itmp);

				if (!strncmp(st_menu[i].name, "Mouse speed", 11)) {
					sprintf(tmp, "%d", itmp);
   				} else if (!strncmp(st_menu[i].name, "PAL", 3)) {
					if (itmp==0)
						sprintf(tmp, "%s", "NTSC");
					else if (itmp==1)
						sprintf(tmp, "%s", "Pal");
					else if (itmp==2)
						sprintf(tmp, "%s", "Dendy");
#ifdef FRAMESKIP
				} else if (!strncmp(st_menu[i].name, "Frameskip", 9)) {
					if (itmp<0)
						sprintf(tmp, "%s", "Auto");
					else
						sprintf(tmp, "%d", itmp);
#endif
				} else
					sprintf(tmp, "%s", itmp ? "on" : "off");
				DrawText(gui_screen, tmp, OPTION_VALUE_COLUMN, y);
			}

			// Draw info
			DrawText(gui_screen, st_menu[index].info, 8, 225);

			// Draw offset marks
			if (offset_start > 0)
				DrawChar(gui_screen, SP_UPARROW, 157, 57);
			if (offset_end < menu_size)
				DrawChar(gui_screen, SP_DOWNARROW, 157, 212);

			g_dirty = 0;
		}

		SDL_Delay(16);

		// Update real screen
		FCEUGUI_Flip();
	}

	// Clear screen
	dingoo_clear_video();

	g_dirty = 1;
	return 0;
}
