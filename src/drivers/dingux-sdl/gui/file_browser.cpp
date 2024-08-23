#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file_list.h"

#ifdef RETROFW
#define DEFAULT_ROM_DIR "/"
#else
#define DEFAULT_ROM_DIR "/media"
#endif

// Externals
extern SDL_Surface* screen;
extern Config *g_config;

uint64 FCEUD_GetTime(void);
uint64 FCEUD_GetTimeFreq(void);

static char file_infos[3][12] = {
	"B - Go Back",
	"X - Exit",
	"A - Load",
};

// Type of Browser from filetypes filter
static const char *
GetTypeOfBrowser(const char *types[]) {
	if (!strncmp(types[0],".nes",4))
		return "ROM Browser";
	else if (!strncmp(types[0],".pal",4))
		return "Palette Browser";
	else if (!strncmp(types[0],".cht",4))
		return "Cheat file Browser";

	return "File Browser";
}

static char s_LastDir[128] = DEFAULT_ROM_DIR;
int RunFileBrowser(char *source, char *outname, const char *types[],
		const char *info) {

	int size = 0;
	int index;
	int offset_start, offset_end;
	static int max_entries = 9;
	int scrollModifier = 1; // OpenDingux - 1 page scrolling
	int justsavedromdir = 0;
	int scrollMult;
	const char *browserType = GetTypeOfBrowser(types);

	static int spy;
	int y, i;
	uint64 time_start, time_current;
	uint8 info_index = 0;

	time_start = FCEUD_GetTime();

	// Try to get a saved romdir from a config file
	char* home = getenv("HOME");
	char romcfgfile [128];
	sprintf (romcfgfile, "%s/.fceux/romdir.cfg", home);
	FILE * pFile;
	pFile = fopen (romcfgfile,"r+");
	if (pFile != NULL) {
		fgets (s_LastDir , 128 , pFile);
		fclose (pFile);
		
		// Avoid crash if romdir configured do not exist
		struct stat ss;
		ss.st_mode = 0;
		if ((stat(s_LastDir, &ss) != 0) || !S_ISDIR(ss.st_mode)) {
			snprintf(s_LastDir, 7, DEFAULT_ROM_DIR);
		}
	}

	// Create file list
	FileList *list = new FileList(source ? source : s_LastDir, types);
	if (list == NULL)
		return 0;

	scrollModifier *= max_entries;

RESTART:
	size = list->Size();

	spy = 72;
	index = 0;
	offset_start = 0;
	offset_end = size > max_entries ? max_entries : size;

	g_dirty = 1;
	while (1) {
		if ( ( (FCEUD_GetTime() - time_start) / FCEUD_GetTimeFreq() ) > 3 ) {
			info_index = ( info_index + 1 ) % 3;
			time_start = FCEUD_GetTime();
			g_dirty = 1;
		}
		// Parse input
		readkey();
		// TODO - put exit keys

		// Go to previous folder or return ...
		if (parsekey(DINGOO_B)) {
			list->Enter(-1);
			goto RESTART;
		}

		// Enter folder or select rom ...
		if (parsekey(DINGOO_A)) {
			// Fix for OpenDingux when no files are shown don't
			// allow select
			if (size > 0) {
				if (list->GetSize(index) == -1) {
					list->Enter(index);
					goto RESTART;
				} else {
					strncpy(outname, list->GetPath(index), 128);
					break;
				}
			}
		}

		if (parsekey(DINGOO_X)) {
			return 1; // OpenDingux - Don't exit emulator
		}

		if (parsekey(DINGOO_SELECT)) {
			// Save the current romdir in a config file
			char* home = getenv("HOME");
			char romcfgfile [128];
			strncpy(s_LastDir, list->GetCurDir(), 128);
			sprintf (romcfgfile, "%s/.fceux/romdir.cfg", home);
			FILE * pFile;
			pFile = fopen (romcfgfile,"w+");
			fputs (s_LastDir,pFile);
			fclose (pFile);
			justsavedromdir = 1;
		}

		if (size > 0) {
			// Move through file list

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
			DrawText(gui_screen, file_infos[info_index], 235, 225);
			DrawChar(gui_screen, SP_LOGO, 12, 9);

			// Draw selector
			DrawChar(gui_screen, SP_SELECTOR, 4, spy);
			DrawChar(gui_screen, SP_SELECTOR, 81, spy);

			DrawText(gui_screen, browserType, 8, 37);

			// Draw file list
			for (i = offset_start, y = 72; i < offset_end; i++, y += 15) {
				DrawText(gui_screen, list->GetName(i), 8, y);
			}

			// Draw info
			if (info)
				DrawText(gui_screen, info, 8, 225);
			else {
				if (justsavedromdir == 1){
					DrawText(gui_screen, "ROM dir successfully saved!", 8, 225);
				} else {
					if (list->GetSize(index) == -1)
						DrawText(gui_screen, "SELECT - Save ROM dir", 8, 225);
					else
						DrawText(gui_screen, "SELECT - Save ROM dir", 8, 225);
				}
				justsavedromdir = 0;
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

	if (source == NULL)
		strncpy(s_LastDir, list->GetCurDir(), 128);
	delete list;

	return 1;
}
