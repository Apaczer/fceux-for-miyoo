/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel  
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/// \file
/// \brief Handles the graphical game display for the SDL implementation.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL/SDL.h>

#include "dingoo.h"
#include "dingoo-video.h"
#include "scaler.h"
#include "throttle.h"

#include "../common/vidblit.h"
#include "../../fceu.h"
#include "../../version.h"

#include "dface.h"

#include "../common/configSys.h"

// GLOBALS
SDL_Surface *screen;
SDL_Surface *nes_screen; // 256x224
#if defined(RETROFW) || defined(OD2014)
void *org_pixels;
#endif

extern Config *g_config;
extern double g_fpsScale; // defined in sdl-throttle

// STATIC GLOBALS
static int s_curbpp;
static int s_srendline, s_erendline;
static int s_tlines;
static int s_inited;
static bool s_VideoModeSet = false;

static int s_clipSides;
int s_fullscreen;
static int noframe;

static int FDSTimer = 0;
int FDSSwitchRequested = 0;

#define NWIDTH	(256 - (s_clipSides ? 16 : 0))
#define NOFFSET	(s_clipSides ? 8 : 0)

/* Blur effect taken from vidblit.cpp */
uint32 palettetranslate[65536 * 4];
static uint32 CBM[3] = { 63488, 2016, 31 };
static uint16 s_psdl[256];
extern u8 *XDBuf;

struct Color {
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 unused;
};

static struct Color s_cpsdl[256];

#define BLUR_RED	30
#define BLUR_GREEN	30
#define BLUR_BLUE	20

#ifdef SDL_TRIPLEBUF
#  define DINGOO_MULTIBUF SDL_TRIPLEBUF
#else
#  define DINGOO_MULTIBUF SDL_DOUBLEBUF
#endif

/**
 * Attempts to destroy the graphical video display.  Returns 0 on
 * success, -1 on failure.
 */

//draw input aids if we are fullscreen
bool FCEUD_ShouldDrawInputAids() {
	return s_fullscreen != 0;
}

int KillVideo() {
	// return failure if the video system was not initialized
	if (s_inited == 0)
		return -1;

	SDL_FreeSurface(nes_screen);
	s_inited = 0;
	s_VideoModeSet = false;
	return 0;
}

/**
 * These functions determine an appropriate scale factor for fullscreen/
 */
inline double GetXScale(int xres) {
	return ((double) xres) / NWIDTH;
}
inline double GetYScale(int yres) {
	return ((double) yres) / s_tlines;
}
void FCEUD_VideoChanged() {
	int region;
	g_config->getOption("SDL.PAL", &region);
	if 	(region==0)		{dendy = 0;PAL = 0;}
	else if (region==1)	{dendy = 0;PAL = 1;}
	else				{dendy = 1;PAL = 0;}
}
/**
 * Attempts to initialize the graphical video display.  Returns 0 on
 * success, -1 on failure.
 */
int InitVideo(FCEUGI *gi) {
	int triplebuf;	

	FCEUI_printf("Initializing video...\n");

	// load the relevant configuration variables
	g_config->getOption("SDL.Fullscreen", &s_fullscreen);
	g_config->getOption("SDL.ClipSides", &s_clipSides);
	g_config->getOption("SDL.TripleBuffering", &triplebuf);

	// check the starting, ending, and total scan lines
	FCEUI_GetCurrentVidSystem(&s_srendline, &s_erendline);
	s_tlines = s_erendline - s_srendline + 1;

	int brightness;
	g_config->getOption("SDL.Brightness", &brightness);

	s_inited = 1;
	FDSSwitchRequested = 0;

	//int desbpp;
	//g_config->getOption("SDL.SpecialFilter", &s_eefx);

	SetPaletteBlitToHigh((uint8 *) s_cpsdl);

	//Init video subsystem
	if (!(SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO))
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1) {
			fprintf(stderr,"%s",SDL_GetError());
		}

	// initialize dingoo video mode
	if (!s_VideoModeSet) {
		int w, h;
		if (s_fullscreen == 1) {
#ifndef MIYOO
			int aspect_ratio;
#ifndef RETROFW
			int videofilter;

			g_config->getOption("SDL.VideoFilter", &videofilter);
			FCEUD_SetVideoFilter(videofilter);
#endif
			g_config->getOption("SDL.AspectSelect", &aspect_ratio);
			switch (aspect_ratio) {
			// 1:1
			case 0:
				w = 320;
				h = 240;
				break;
			// 8:7
			case 1:
				w = 280;
				h = 240;
				break;
			// 4:3
			case 2:
			default:
				w = s_clipSides ? 240 : 256;
				h = PAL ? 240 : s_tlines > 224 ? 240 : 224;
				break;
			}
#ifdef OD2014
		        FILE* integer_scaling_file = fopen("/sys/class/graphics/fb0/device/integer_scaling", "w");
		        if (integer_scaling_file) {
			        fwrite("N", 1, 1, integer_scaling_file);
			        fclose(integer_scaling_file);
		        }

		        FILE* keep_aspect_ratio_file = fopen("/sys/class/graphics/fb0/device/keep_aspect_ratio", "w");
		        if (keep_aspect_ratio_file) {
			        fwrite("N", 1, 1, keep_aspect_ratio_file);
			        fclose(keep_aspect_ratio_file);
		        }
#endif
#endif
		} else {
			w = 320; h = 240;
		}
		// OpenDingux - SDL_VideoModeOK seems not to work in the new beta
		if (NoWaiting || g_fpsScale > 1.0  || !triplebuf)
			screen = SDL_SetVideoMode(w, h, 16, SDL_HWSURFACE);
		else
			screen = SDL_SetVideoMode(w, h, 16, SDL_HWSURFACE | DINGOO_MULTIBUF);
		if (screen) {
			s_VideoModeSet = true;
#if defined(RETROFW) || defined(OD2014)
			org_pixels = screen->pixels;
#endif
		}
	}

	// a hack to bind inner buffer to nes_screen surface
	extern uint8 *XBuf;

	nes_screen = SDL_CreateRGBSurfaceFrom(XBuf, 256, 224, 8, 256, 0, 0, 0, 0);
	if(!nes_screen)
		printf("Error in SDL_CreateRGBSurfaceFrom\n");
	SDL_SetPalette(nes_screen, SDL_LOGPAL, (SDL_Color *)s_cpsdl, 0, 256);

	SDL_ShowCursor(0);

	/* clear screen */
	dingoo_clear_video();

	return 0;
}

void InitGuiVideo() {
	if (screen->w == 320 && screen->h == 240 && screen->flags & DINGOO_MULTIBUF) return;
	// OpenDingux - SDL_VideoModeOK seems not to work in the new beta
	screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | DINGOO_MULTIBUF);
}

/**
 * Toggles the full-screen display.
 */
void ToggleFS() {
}

/* Taken from /src/drivers/common/vidblit.cpp */
static void CalculateShift(uint32 *CBM, int *cshiftr, int *cshiftl)
{
	int a, x, z, y;
	cshiftl[0] = cshiftl[1] = cshiftl[2] = -1;
	for (a = 0; a < 3; a++) {
		for (x = 0, y = -1, z = 0; x < 32; x++) {
			if (CBM[a] & (1 << x)) {
				if (cshiftl[a] == -1)
					cshiftl[a] = x;
				z++;
			}
		}
		cshiftr[a] = (8 - z);
	}
}

void SetPaletteBlitToHigh(uint8 *src)
{
	int cshiftr[3];
	int cshiftl[3];
	int x, y;

	CalculateShift(CBM, cshiftr, cshiftl);

	for (x = 0; x < 65536; x++) {
		uint16 lower, upper;

		lower = (src[((x & 255) << 2)] >> cshiftr[0]) << cshiftl[0];
		lower |= (src[((x & 255) << 2) + 1] >> cshiftr[1]) << cshiftl[1];
		lower |= (src[((x & 255) << 2) + 2] >> cshiftr[2]) << cshiftl[2];
		upper = (src[((x >> 8) << 2)] >> cshiftr[0]) << cshiftl[0];
		upper |= (src[((x >> 8) << 2) + 1] >> cshiftr[1]) << cshiftl[1];
		upper |= (src[((x >> 8) << 2) + 2] >> cshiftr[2]) << cshiftl[2];

		palettetranslate[x] = lower | (upper << 16);
	}
}

//takes a pointer to XBuf and applies fully modern deemph palettizing
u32 ModernDeemphColorMap(u8* src, u8* srcbuf, int xscale, int yscale)
{
	u8 pixel = *src;

	//look up the legacy translation
	u32 color = palettetranslate[pixel];

	int ofs = src-srcbuf;
	int xofs = ofs&255;
	int yofs = ofs>>8;
	if(xscale!=1) xofs /= xscale; //untested optimization
	if(yscale!=1) yofs /= yscale; //untested optimization
	ofs = xofs+yofs*256;

	//find out which deemph bitplane value we're on
	uint8 deemph = XDBuf[ofs];

	//if it was a deemph'd value, grab it from the deemph palette
	if(deemph != 0)
		color = palettetranslate[256+(pixel&0x3F)+deemph*64];

	return color;
}

/**
 * Sets the color for a particular index in the palette.
 */
void FCEUD_SetPalette(uint8 index, uint8 r, uint8 g, uint8 b)
{
	s_cpsdl[index].r = r;
	s_cpsdl[index].g = g;
	s_cpsdl[index].b = b;

	//uint32 col = (r << 16) | (g << 8) | b;
	//s_psdl[index] = (uint16)COL32_TO_16(col);
	s_psdl[index] = dingoo_video_color15(r, g, b);

	if (index == 255)
		SetPaletteBlitToHigh((uint8 *) s_cpsdl);
}

/**
 * Gets the color for a particular index in the palette.
 */
void FCEUD_GetPalette(uint8 index, uint8 *r, uint8 *g, uint8 *b)
{
	*r = s_cpsdl[index].r;
	*g = s_cpsdl[index].g;
	*b = s_cpsdl[index].b;
}

uint16 * FCEUD_GetPaletteArray16()
{
	return s_psdl;
}

/** 
 * Pushes the palette structure into the underlying video subsystem.
 */
static void RedoPalette() {
}

// XXX soules - console lock/unlock unimplemented?

///Currently unimplemented.
void LockConsole() {
}

///Currently unimplemented.
void UnlockConsole() {
}

#define READU16(x)  (uint16) ((uint16)(x)[0] | (uint16)(x)[1] << 8) 

/**
 * Pushes the given buffer of bits to the screen.
 */
void BlitScreen(uint8 *XBuf) {
	int x, x2, y, y2;

	// Taken from fceugc
	// FDS switch disk requested - need to eject, select, and insert
	// but not all at once!
	if (FDSSwitchRequested) {
		switch (FDSSwitchRequested) {
		case 1:
			FDSSwitchRequested++;
			FCEUI_FDSInsert(); // eject disk
			FDSTimer = 0;
			break;
		case 2:
			if (FDSTimer > 60) {
				FDSSwitchRequested++;
				FDSTimer = 0;
				FCEUI_FDSSelect(); // select other side
				FCEUI_FDSInsert(); // insert disk
			}
			break;
		case 3:
			if (FDSTimer > 200) {
				FDSSwitchRequested = 0;
				FDSTimer = 0;
			}
			break;
		}
		FDSTimer++;
	}

	// TODO - Move these to its own file?
	if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);

	register uint8 *pBuf = XBuf;

#ifdef MIYOO
	if(s_fullscreen == 3) { // fullscreen smooth
#else
	if(s_fullscreen == 4) { // fullscreen smooth
#endif
		if (s_clipSides) {
			upscale_320x240_bilinearish_clip((uint32 *)screen->pixels, (uint8 *)XBuf + 256 * 8, 256);
		} else {
			upscale_320x240_bilinearish_noclip((uint32 *)screen->pixels, (uint8 *)XBuf + 256 * 8, 256);
		}
#ifdef MIYOO
	} else if(s_fullscreen == 2) { // fullscreen
#else
	} else if(s_fullscreen == 3) { // fullscreen
#endif
		switch(screen->w) {
			case 480: upscale_480x272((uint32 *)screen->pixels, (uint8 *)XBuf + 256 * 8); break;
			case 400: upscale_384x240((uint32 *)screen->pixels, (uint8 *)XBuf + 256 * 8); break;
			case 320: upscale_320x240((uint32 *)screen->pixels, (uint8 *)XBuf + 256 * 8); break;
		}
#ifdef MIYOO
	} else if(s_fullscreen == 1) { // aspect fullscreen
#else
	} else if(s_fullscreen == 2) { // aspect fullscreen
#endif
		switch(screen->w) {
			case 480: upscale_384x272((uint32 *)screen->pixels, (uint8 *)XBuf + 256 * 8); break;
			case 400:
			case 320:
				pBuf += (s_srendline * 256) + 8;
				register uint16 *dest = (uint16 *) screen->pixels;
				//dest += (320 * s_srendline) + 20;
				dest += (screen->w * s_srendline) + (screen->w - 280) / 2 + ((screen->h - 240) / 2) * screen->w;

				// semi fullscreen no blur
				for (y = s_tlines; y; y--) {
					for (x = 240; x; x -= 6) {
						__builtin_prefetch(dest + 2, 1);
						*dest++ = s_psdl[*pBuf];
						*dest++ = s_psdl[*(pBuf + 1)];
						*dest++ = s_psdl[*(pBuf + 2)];
						*dest++ = s_psdl[*(pBuf + 3)];
						*dest++ = s_psdl[*(pBuf + 3)];
						*dest++ = s_psdl[*(pBuf + 4)];
						*dest++ = s_psdl[*(pBuf + 5)];
						pBuf += 6;
					}
					pBuf += 16;
					//dest += 40;
					dest += screen->w - 280;
				}
		}
	} else { // native res
		int32 pinc = (screen->w - NWIDTH) >> 1;
		int32 append = (256 - NWIDTH) >> 1;

		register uint32 *dest = (uint32 *) screen->pixels;

		// XXX soules - not entirely sure why this is being done yet
		pBuf += (s_srendline * 256) + NOFFSET;
		//dest += (s_srendline * 320) + pinc >> 1;
		dest += (screen->w/2 * s_srendline) + pinc / 2 + ((screen->h - 240) / 4) * screen->w;

		register uint16 *prBuf = (uint16 *)pBuf;
		for (y = s_tlines; y; y--, prBuf += append) {
			for (x = NWIDTH>>1; x; x--, prBuf++) {
				*dest++ = palettetranslate[*prBuf];
			}
			dest += pinc;
		}
	}

	if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
	SDL_Flip(screen);
	
	videoBufferSwapMark();
}

/**
 *  Converts an x-y coordinate in the window manager into an x-y
 *  coordinate on FCEU's screen.
 */
uint32 PtoV(uint16 x, uint16 y) {
	y = (uint16) ((double) y);
	x = (uint16) ((double) x);
	if (s_clipSides) {
		x += 8;
	}
	y += s_srendline;
	return (x | (y << 16));
}

bool disableMovieMessages = false;
bool FCEUI_AviDisableMovieMessages() {
	if (disableMovieMessages)
		return true;

	return false;
}

void FCEUI_SetAviDisableMovieMessages(bool disable) {
	disableMovieMessages = disable;
}

//clear all screens (for multiple-buffering)
void dingoo_clear_video(void) {
	SDL_FillRect(screen,NULL,SDL_MapRGBA(screen->format, 0, 0, 0, 255));
	SDL_Flip(screen);
	SDL_FillRect(screen,NULL,SDL_MapRGBA(screen->format, 0, 0, 0, 255));
	SDL_Flip(screen);
#ifdef SDL_TRIPLEBUF
	SDL_FillRect(screen,NULL,SDL_MapRGBA(screen->format, 0, 0, 0, 255));
	SDL_Flip(screen);
#endif
}

#if defined(RETROFW) || defined(OD2014)
//flip all screens
void dingoo_flip_all_video(void) {
	while (screen->pixels != org_pixels)
		SDL_Flip(screen);
	SDL_Flip(screen);
}
#endif
