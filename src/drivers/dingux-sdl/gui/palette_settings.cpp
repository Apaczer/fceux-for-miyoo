#include "../utilities.h"

// Externals
extern Config *g_config;

// Custom palette
static void custom_update(unsigned long key) {
	char exePath[512];
	const char *types[] = { ".pal", NULL };
	char palname[128] = "";
	
	// Left cursor clear palette
	if (key != DINGOO_LEFT) {;
		fceuExecutablePath(exePath, sizeof(exePath));
		strncat(exePath, "/palettes", 9);

		if (!RunFileBrowser(exePath, palname, types, "Choose nes palette (.pal)") || palname[0] == '\0')
		{
			return;
		}	
	}

	std::string cpalette = std::string(palname);
	g_config->setOption("SDL.Palette", cpalette);
	
	// Clear custom palette
	if(!cpalette.size()) {
		FCEUI_SetUserPalette(NULL, 0);
	}
}

// Force Gray Scale
static void forcegrayscale_update(unsigned long key)
{
	int val;

	if (key == DINGOO_RIGHT) val = 1;
	if (key == DINGOO_LEFT) val = 0;

	g_config->setOption("SDL.ForceGrayScale", val);
}

// De-emphasis Bit Swap
static void deemphbitswap_update(unsigned long key)
{
	int val;

	if (key == DINGOO_RIGHT) val = 1;
	if (key == DINGOO_LEFT) val = 0;

	g_config->setOption("SDL.DeempBitSwap", val);
}

// NTSC TV's colors
static void ntsc_update(unsigned long key)
{
	int val;

	if (key == DINGOO_RIGHT) val = 1;
	if (key == DINGOO_LEFT) val = 0;

	g_config->setOption("SDL.NTSCpalette", val);
}

// NTSC Tint
static void tint_update(unsigned long key)
{
	int val;
	g_config->getOption("SDL.Tint", &val);

	if (key == DINGOO_RIGHT) val = val < 255 ? val+1 : 255;
	if (key == DINGOO_LEFT) val = val > 0 ? val-1 : 0;

	g_config->setOption("SDL.Tint", val);
}

// NTSC Hue
static void hue_update(unsigned long key)
{
	int val;
	g_config->getOption("SDL.Hue", &val);

	if (key == DINGOO_RIGHT) val = val < 255 ? val+1 : 255;
	if (key == DINGOO_LEFT) val = val > 0 ? val-1 : 0;

	g_config->setOption("SDL.Hue", val);
}

/* PALETTE MENU */

static SettingEntry palette_menu[] = 
{
	{"Custom palette", "Load custom palette", "SDL.Palette", custom_update},
	{"Force Grayscale","Force gray scale","SDL.ForceGrayScale", forcegrayscale_update},
	{"De-emphasis Bit Swap","De-emphasis Bit Swap","SDL.DeempBitSwap", deemphbitswap_update},
	{"NTSC Palette", "Emulate NTSC TV's colors", "SDL.NTSCpalette", ntsc_update},
	{"Tint", "Sets tint for NTSC color", "SDL.Tint", tint_update},
	{"Hue", "Sets hue for NTSC color", "SDL.Hue", hue_update},
};
static int palette_menu_items = sizeof(palette_menu) / sizeof(palette_menu[0]);

int RunPaletteSettings()
{
	static int index = 0;
	static int spy = 72;
	int done = 0, y, i;
	
	int max_entries = 9;
	int menu_size = palette_menu_items;

	static int offset_start = 0;
	static int offset_end = menu_size > max_entries ? max_entries : menu_size;

	char tmp[32];
	int  itmp;

	g_dirty = 1;
	while (!done) {
		// Parse input
		readkey();
		if (parsekey(DINGOO_B)) done = 1;
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

		if (parsekey(DINGOO_RIGHT, 1) || parsekey(DINGOO_LEFT, 1) || parsekey(DINGOO_A))
			palette_menu[index].update(g_key);
  
		// Draw stuff
		if( g_dirty ) 
		{
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

			DrawText(gui_screen, "Palette Settings", 8, 37); 

			// Draw menu
			for(i=offset_start,y=72;i < offset_end;i++,y+=15) {
				DrawText(gui_screen, palette_menu[i].name, OPTION_LABEL_COLUMN, y);
		
				g_config->getOption(palette_menu[i].option, &itmp);
				if (!strncmp(palette_menu[i].name, "Custom palette", 14)) {
					std::string palname;
					g_config->getOption(palette_menu[i].option, &palname);

					// Remove path of string
					const int sz = static_cast<int> (palname.size());
					const int path_sz = palname.rfind("/", palname.size());

					if (path_sz == sz)
						strncpy(tmp, palname.c_str(), 32);
					else
						strncpy(tmp, palname.substr(path_sz + 1, sz - 1
								- path_sz).c_str(), 32);
				} else if (!strncmp(palette_menu[i].name, "NTSC Palette", 12)
					 ||!strncmp(palette_menu[i].name, "Force Grayscale", 15)
					 ||!strncmp(palette_menu[i].name, "De-emphasis Bit Swap", 20)) {
					sprintf(tmp, "%s", itmp ? "on" : "off");
				}
				else sprintf(tmp, "%d", itmp);

				DrawText(gui_screen, tmp, OPTION_VALUE_COLUMN, y);
			}

			// Draw info
			DrawText(gui_screen, palette_menu[index].info, 8, 225);
			
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
