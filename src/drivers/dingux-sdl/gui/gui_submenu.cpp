int current_index = 0;

int RunSubmenu(MenuEntry submenu[], int entries, const char *title, void (*infocb)(void))
{
	static int index;
	static int spy;
	int done = 0, y, i;

	char tmp[32];
	int  itmp;

	static const int menu_size = entries;
	static const int max_entries = 8;
	static int offset_start;
	static int offset_end;
	int scrollModifier = 1; // OpenDingux - 1 page scrolling
	
	int exit_status = 0;

RESTART:
	spy = 72;
	index = 0;
	offset_start = 0;
	offset_end = entries > max_entries ? max_entries : entries;
	    
	g_dirty = 1;
	while (!done) {
		// Parse input
		readkey();
		// get the keyboard input
		#if SDL_VERSION_ATLEAST(1, 3, 0)	 
		g_keyState = SDL_GetKeyboardState(NULL);	 
		#else
		g_keyState = SDL_GetKeyState(NULL);
		#endif

		// Go to previous folder or return ...
		if (parsekey(DINGOO_B)) done = 1;
		
		// Do option
		if (parsekey(DINGOO_A)) {
			if (menu_size > 0) {
				    exit_status = submenu[index].command();
				    done = 1;
			}
		}

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

		if (parsekey(DINGOO_LEFT, 1)) {
			if (index > offset_start) {
				index = offset_start;

				spy = 72;

			} else if (index - scrollModifier >= 0){
					index -= scrollModifier;
					offset_start -= scrollModifier;
					offset_end = offset_start + max_entries;
			} else
				goto RESTART;
		}

		if (parsekey(DINGOO_RIGHT, 1)) {
			if (index < offset_end-1) {
				index = offset_end-1;

				spy = 72 + 15*(index-offset_start);

			} else if (offset_end + scrollModifier <= menu_size) {
					index += scrollModifier;
					offset_end += scrollModifier;
					offset_start += scrollModifier;
			} else {
				int iSmartOffsetAdj = ((menu_size <= max_entries) ? menu_size : max_entries);
				index = menu_size - 1;
				spy = 72 + 15*(iSmartOffsetAdj-1);
				offset_end = menu_size;
				offset_start = offset_end - iSmartOffsetAdj;
			}
		}

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

			DrawText(gui_screen, title, 8, 37);
			
			if (infocb) infocb();

			// Draw menu
			for (i = offset_start, y = 72; i < offset_end; i++, y += 15) {
				DrawText(gui_screen, submenu[i].name, OPTION_LABEL_COLUMN, y);
			}

			// Draw info
			DrawText(gui_screen, submenu[index].info, 8, 225);

			// Draw offset marks
			if (offset_start > 0)
				DrawChar(gui_screen, SP_UPARROW, 157, 62);
			if (offset_end < menu_size)
				DrawChar(gui_screen, SP_DOWNARROW, 157, 203);

			g_dirty = 0;
		}

		SDL_Delay(16);

		// Update real screen
		FCEUGUI_Flip();
	}

	// Clear screen
	dingoo_clear_video();

	g_dirty = 1;
	return exit_status;
}

