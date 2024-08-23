#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "cheat_list.h"

// Externals
extern Config *g_config;
extern int globalCheatDisabled;
extern int disableAutoLSCheats;

extern uint64 FCEUD_GetTime(void);
extern uint64 FCEUD_GetTimeFreq(void);

int global_enabled;
bool ls_enabled;

static CheatList *list;

#define INFO_ENTRIES 6
static char cheat_infos[INFO_ENTRIES][18] = {
	"B - Go Back",
	"X - Submenu",
	"A - Toggle cheat",
	"Y - Import cheats",
	"START - Enable",
	"SELECT - Autoload"
};

static INLINE void cheats_print_global_configuration(void) {
	DrawText(gui_screen, "Enable cheats:", 182, 8);
	DrawText(gui_screen, global_enabled ? "On" : "Off", 298, 8);
	DrawText(gui_screen, "Auto load/save:", 182, 18);
	DrawText(gui_screen, ls_enabled ? "On" : "Off", 298, 18);
}

static void cheats_info_callback(void) {
	cheats_print_global_configuration();
	if ( list->Size() > 0 ) {
	    if ( list->IsActive(current_index) ) 
		DrawChar(gui_screen,('*'-32) & 0x7f, 0, 37);
	    DrawText(gui_screen, list->GetCode(current_index), 200, 37);
	}
}

// Import cheat file
static int import_cheats(CheatList *list) {
	const char *types[] = { ".cht", NULL };
	char cheatfile[128] = "";

	if (!RunFileBrowser(NULL, cheatfile, types, "Choose cheat file (.cht)") || cheatfile[0] == '\0') {
	    return 0;
	}
	
	return list->LoadCheats(cheatfile,true);
}

static int cmd_toggle_cheat(void) {
	if (list->Size() > 0) {
	    list->ToggleCheat(current_index);
	    g_dirty = 1;
	} 
	return 0;
}

static int cmd_delete_cheat(void) {
	if (list->Size() > 0) {
	    if (list->DelCheat(current_index))
		return 3;
	    else
		return 0;
	} else {
	    return 0;
	}
}

static int cmd_delete_all_cheats(void) {
	if (list->Size() > 0) {
	    if (list->DelAllCheats())
		return 3;
	    else
		return 0;
	} else
	    return 0;
}

static int cmd_import_cheat_file(void) {
	int status = import_cheats(list);
	if (status)
	    return 3; // Restart
	else
	    return 0;
}

static int cmd_return_to_game(void) {
	return 2; // Exit
}

static int cmd_enable_global_cheats(void) {
	global_enabled = !global_enabled;
	FCEUI_GlobalToggleCheat(global_enabled);
	g_config->setOption("SDL.CheatDisabled", globalCheatDisabled);
	g_config->save();
	
	g_dirty = 1;
	
	return 0;
}

static int cmd_enable_autoload(void) {
	ls_enabled = !ls_enabled;
	disableAutoLSCheats = ls_enabled ? 0 : 2;
	g_config->setOption("SDL.AutoLSCheatsDisabled", disableAutoLSCheats);
	g_config->save();
	
	g_dirty = 1;
	
	return 0;
}

static MenuEntry oc_cheats_submenu[] =
{
	{ "Toggle cheat", "Enable/disable cheat", cmd_toggle_cheat },
	{ "Delete cheat", "Delete current cheat", cmd_delete_cheat },
	{ "Delete all cheats", "Delete all cheats", cmd_delete_all_cheats },
	{ "Import cheats", "Import cheat file (.cht)", cmd_import_cheat_file },
	{ "Enable cheats globally", "Enable/Disable cheats globally", cmd_enable_global_cheats },
	{ "Enable autoload/save", "Enable/Disable autoload/save", cmd_enable_autoload },
	{ "Exit to game", "Exit to game", cmd_return_to_game },
};

const static int submenu_entries = (sizeof(oc_cheats_submenu) / sizeof(oc_cheats_submenu[0]));

static int toggle_cheat(int index) {
    current_index = index;
    return cmd_toggle_cheat();
}

extern void InitGuiVideo();
int RunCheatBrowser(const char *info = NULL) {
	int size = 0;
	int done = 0;
	int return_value = 0;
	int index;
	int offset_start, offset_end;
	static int max_entries = 9;
	int scrollModifier = 1; // OpenDingux - 1 page scrolling

	static int spy;
	int y, i;
	uint64 time_start, time_current;
	uint8 info_index = 0;
	
	InitGuiVideo();

	time_start = FCEUD_GetTime();

	// Create file list
	list = new CheatList();
	if (list == NULL)
		return 0;

	scrollModifier *= max_entries;
	
	global_enabled = !globalCheatDisabled;
	ls_enabled = ( disableAutoLSCheats == 2 ) ? false : true;

RESTART:
	size = list->Size();

	spy = 72;
	index = 0;
	offset_start = 0;
	offset_end = size > max_entries ? max_entries : size;

	g_dirty = 1;
	while (!done) {
		current_index = index;
		if ( ( (FCEUD_GetTime() - time_start) / FCEUD_GetTimeFreq() ) > 3 ) {
			info_index = ( info_index + 1 ) % INFO_ENTRIES;
			time_start = FCEUD_GetTime();
			g_dirty = 1;
		}
		// Parse input
		readkey();

		// Go to previous folder or return ...
		if (parsekey(DINGOO_B)) {
			done = 1;
		}

		// Enable/Disable Cheat
		if (parsekey(DINGOO_A)) {
			toggle_cheat(index);
		}

		if (parsekey(DINGOO_X)) {
		    int sb_status = RunSubmenu( oc_cheats_submenu, submenu_entries, (size > 0 ? list->GetLabel(index) : NULL), cheats_info_callback );
		    if ( sb_status == 3 ) {
			goto RESTART;
		    } else if ( sb_status == 2 ) {
			return_value = 1; // Exit to game
			done = 1;
		    } else if ( sb_status == 1 ) {
			done = 1;
		    }
		}
		
		
		if (parsekey(DINGOO_Y)) {
			if (cmd_import_cheat_file())
			    goto RESTART;
		}

		if (parsekey(DINGOO_SELECT)) {
			cmd_enable_autoload();
		}

		if (parsekey(DINGOO_START)) {
			cmd_enable_global_cheats();
		}

		if (size > 0) {
			// Move through cheat list
			if (parsekey(DINGOO_R, 0)) {
				int iSmartOffsetAdj = ((size <= max_entries) ? size : max_entries);
				index = size - 1;
				spy = 72 + 15*(iSmartOffsetAdj-1);
				offset_end = size;
				offset_start = offset_end - iSmartOffsetAdj;
			}

			if (parsekey(DINGOO_L, 0)) {
				goto RESTART;
			}

			if (parsekey(DINGOO_UP, 1)) {
				if (index > offset_start){
					index--;
					spy -= 15;
				} else if (offset_start > 0) {
					index--;
					offset_start--;
					offset_end--;
				} else {
					index = size - 1;
					offset_end = size;
					offset_start = size <= max_entries ? 0 : offset_end - max_entries;
					spy = 72 + 15*(index - offset_start);
				}
			}

			if (parsekey(DINGOO_DOWN, 1)) {
				if (index < offset_end - 1){
					index++;
					spy += 15;
				} else if (offset_end < size) {
					index++;
					offset_start++;
					offset_end++;
				} else {
					index = 0;
					offset_start = 0;
					offset_end = size <= max_entries ? size : max_entries;
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

				} else if (offset_end + scrollModifier <= size) {
						index += scrollModifier;
						offset_end += scrollModifier;
						offset_start += scrollModifier;
				} else {
					int iSmartOffsetAdj = ((size <= max_entries) ? size : max_entries);
					index = size - 1;
					spy = 72 + 15*(iSmartOffsetAdj-1);
					offset_end = size;
					offset_start = offset_end - iSmartOffsetAdj;
				}
			}
		}

		// Draw stuff
		if (g_dirty) {
			draw_bg(g_bg);

			//Draw Top and Bottom Bars
			DrawChar(gui_screen, SP_SELECTOR, 0, 37);
			DrawChar(gui_screen, SP_SELECTOR, 81, 37);
			DrawChar(gui_screen, SP_SELECTOR, 0, 225);
			DrawChar(gui_screen, SP_SELECTOR, 81, 225);
			DrawText(gui_screen, cheat_infos[info_index], 196, 225);
			DrawChar(gui_screen, SP_LOGO, 12, 9);

			// Draw selector
			DrawChar(gui_screen, SP_SELECTOR, 4, spy);
			DrawChar(gui_screen, SP_SELECTOR, 81, spy);

			if (g_romname)
			    DrawText(gui_screen, g_romname, 8, 37);
			else
			    DrawText(gui_screen, "Cheat Browser", 8, 37);
			
			// Active flags
			cheats_print_global_configuration();
			
			if (size > 0)
				DrawText(gui_screen, list->GetCode(index), 200, 37);

			// Draw file list
			for (i = offset_start, y = 72; i < offset_end; i++, y += 15) {
				if (list->IsActive(i)) 
				    DrawChar(gui_screen,('*'-32) & 0x7f, 8, y);
				DrawText(gui_screen, list->GetLabel(i), 16, y);
			}

			// Draw info
			if (info)
				DrawText(gui_screen, info, 8, 225);
			else {
				DrawText(gui_screen, "Select cheats", 8, 225);
			}

			// Draw offset marks
			if (offset_start > 0)
				DrawChar(gui_screen, SP_UPARROW, 157, 57);
			if (offset_end < list->Size())
				DrawChar(gui_screen, SP_DOWNARROW, 157, 212);

			g_dirty = 0;
		}

		SDL_Delay(4);

		// Update real screen
		FCEUGUI_Flip();

	}

	delete list;

	// Clear screen
	dingoo_clear_video();

	g_dirty = 1;
	return return_value;
}
