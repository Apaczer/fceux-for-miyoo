
// Externals
extern Config *g_config;

/* MENU COMMANDS */

// Fullscreen mode
static char *scale_tag[] = {
		"Original",
#ifndef MIYOO
// no GPU means no HW scaling or at least it never worked on Miyoo
		"Hardware",
#endif
		"Aspect",
		"FS Fast",
		"FS Smooth"
};

// Fullscreen mode
#ifndef MIYOO
static char *aspect_tag[] = {
		"1:1",
		"8:7",
		"4:3",
};
#endif

#ifndef RETROFW
static char *video_filter_tag[] = {
		"Nearest",
		"Bilinear",
		"Bicubic",
};
#endif

static void fullscreen_update(unsigned long key)
{
	int val;
	g_config->getOption("SDL.Fullscreen", &val);

#ifdef MIYOO
	if (key == DINGOO_RIGHT) val = val < 3 ? val+1 : 3;
#else
	if (key == DINGOO_RIGHT) val = val < 4 ? val+1 : 4;
#endif
	if (key == DINGOO_LEFT) val = val > 0 ? val-1 : 0;
   
	g_config->setOption("SDL.Fullscreen", val);
}

static void aspectselect_update(unsigned long key)
{
	int val;
	g_config->getOption("SDL.AspectSelect", &val);

	if (key == DINGOO_RIGHT) val = val < 2 ? val+1 : 2;
	if (key == DINGOO_LEFT) val = val > 0 ? val-1 : 0;
   
	g_config->setOption("SDL.AspectSelect", val);
}

#ifndef RETROFW
static void videofilter_update(unsigned long key)
{
	int val;
	g_config->getOption("SDL.VideoFilter", &val);

	if (key == DINGOO_RIGHT) val = val < 32 ? val+1 : 32;
	if (key == DINGOO_LEFT) val = val > 0 ? val-1 : 0;
   
	g_config->setOption("SDL.VideoFilter", val);
}
#endif

// Clip sides
static void clip_update(unsigned long key)
{
	int val, tmp;
	g_config->getOption("SDL.ClipSides", &tmp);

	if (key == DINGOO_RIGHT) val = 1;
	if (key == DINGOO_LEFT) val = 0;

	g_config->setOption("SDL.ClipSides", val);
}

// PPU emulation
static void newppu_update(unsigned long key)
{
	int val, tmp;
	g_config->getOption("SDL.NewPPU", &tmp);

	if (key == DINGOO_RIGHT) val = 1;
	if (key == DINGOO_LEFT) val = 0;

	g_config->setOption("SDL.NewPPU", val);
}

// Triple buffer
static void triplebuffering_update(unsigned long key)
{
	int val, tmp;
	g_config->getOption("SDL.TripleBuffering", &tmp);

	if (key == DINGOO_RIGHT) val = 1;
	if (key == DINGOO_LEFT) val = 0;

	g_config->setOption("SDL.TripleBuffering", val);
}

// Scanline start
static void slstart_update(unsigned long key)
{
	int val;
	g_config->getOption("SDL.ScanLineStartNTSC", &val);

	if (key == DINGOO_RIGHT) val = val < 239 ? val+1 : 239;
	if (key == DINGOO_LEFT) val = val > 0 ? val-1 : 0;

	g_config->setOption("SDL.ScanLineStartNTSC", val);
}

// Scanline end
static void slend_update(unsigned long key)
{
	int val;
	g_config->getOption("SDL.ScanLineEndNTSC", &val);

	if (key == DINGOO_RIGHT) val = val < 239 ? val+1 : 239;
	if (key == DINGOO_LEFT) val = val > 0 ? val-1 : 0;

	g_config->setOption("SDL.ScanLineEndNTSC", val);
}

// Scanline start
static void slstartpal_update(unsigned long key)
{
	int val;
	g_config->getOption("SDL.ScanLineStartPAL", &val);

	if (key == DINGOO_RIGHT) val = val < 239 ? val+1 : 239;
	if (key == DINGOO_LEFT) val = val > 0 ? val-1 : 0;

	g_config->setOption("SDL.ScanLineStartPAL", val);
}

// Scanline end
static void slendpal_update(unsigned long key)
{
	int val;
	g_config->getOption("SDL.ScanLineEndPAL", &val);

	if (key == DINGOO_RIGHT) val = val < 239 ? val+1 : 239;
	if (key == DINGOO_LEFT) val = val > 0 ? val-1 : 0;

	g_config->setOption("SDL.ScanLineEndPAL", val);
}

// Integer frame rate
static void integer_framerate_update(unsigned long key)
{
	bool val;

	if (key == DINGOO_RIGHT) val = 1;
	if (key == DINGOO_LEFT) val = 0;

	g_config->setOption("SDL.IntFrameRate", val);
}

/* VIDEO SETTINGS MENU */

static SettingEntry vd_menu[] = 
{
	{"Video scaling", "Select video scale mode", "SDL.Fullscreen", fullscreen_update},
#ifndef MIYOO
	{"HW PAR", "Hardware scaling Pixel AR", "SDL.AspectSelect", aspectselect_update},
#endif
#ifndef RETROFW
	{"HW Video filter", "Hardware scaling video filter", "SDL.VideoFilter", videofilter_update},
#endif
	{"Clip sides", "Clips left and right columns", "SDL.ClipSides", clip_update},
#ifdef SDL_TRIPLEBUF
	{"Triple buffer", "Triple buffer", "SDL.TripleBuffering", triplebuffering_update},
#else
	{"Double buffer", "Double buffer", "SDL.TripleBuffering", triplebuffering_update},
#endif
	{"New PPU", "New PPU emulation engine", "SDL.NewPPU", newppu_update},
	{"NTSC Scanline start", "NTSC first drawn scanline", "SDL.ScanLineStartNTSC", slstart_update},
	{"NTSC Scanline end", "NTSC last drawn scanline", "SDL.ScanLineEndNTSC", slend_update},
	{"PAL Scanline start", "PAL first drawn scanline", "SDL.ScanLineStartPAL", slstartpal_update},
	{"PAL Scanline end", "PAL last drawn scanline", "SDL.ScanLineEndPAL", slendpal_update},
	{"Integer Frame Rate","Use Integer Frame Rate","SDL.IntFrameRate",integer_framerate_update},
};
static int vd_menu_items = sizeof(vd_menu) / sizeof(vd_menu[0]);

int RunVideoSettings()
{
	static int index = 0;
	static int spy = 72;
	int done = 0, y, i;
	
	int max_entries = 9;
	int menu_size = vd_menu_items;

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

		if (parsekey(DINGOO_RIGHT, 1) || parsekey(DINGOO_LEFT, 1))
			vd_menu[index].update(g_key);
  
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

			DrawText(gui_screen, "Video Settings", 8, 37); 

			// Draw menu
			for(i=offset_start,y=72;i < offset_end;i++,y+=15) {
				DrawText(gui_screen, vd_menu[i].name, OPTION_LABEL_COLUMN, y);
		
				g_config->getOption(vd_menu[i].option, &itmp);
				if (!strncmp(vd_menu[i].name, "Video scaling", 13)) {
					sprintf(tmp, "%s", scale_tag[itmp]);
				}
#ifndef MIYOO
				else if (!strncmp(vd_menu[i].name, "HW PAR", 6)) {
					sprintf(tmp, "%s", aspect_tag[itmp]);
				}
#endif
#ifndef RETROFW
				else if (!strncmp(vd_menu[i].name, "HW Video filter", 15)) {
					if (itmp > 1)
						sprintf(tmp, "%s (%d)", video_filter_tag[2], itmp);
					else
						sprintf(tmp, "%s", video_filter_tag[itmp]);
				}
#endif
				else if (!strncmp(vd_menu[i].name, "Clip sides", 10)
					|| !strncmp(vd_menu[i].name, "New PPU", 7)
#ifdef SDL_TRIPLEBUF
					|| !strncmp(vd_menu[i].name, "Triple buffer", 13)
#else
					|| !strncmp(vd_menu[i].name, "Double buffer", 13)
#endif
					|| !strncmp(vd_menu[i].name, "Integer Frame", 13)) {
					sprintf(tmp, "%s", itmp ? "on" : "off");
				}
				else sprintf(tmp, "%d", itmp);

				DrawText(gui_screen, tmp, OPTION_VALUE_COLUMN, y);
			}

			// Draw info
			DrawText(gui_screen, vd_menu[index].info, 8, 225);
			
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
