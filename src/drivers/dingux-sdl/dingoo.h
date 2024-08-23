#ifndef __FCEU_DINGOO_H
#define __FCEU_DINGOO_H

#include <SDL/SDL.h>
#include "main.h"
#include "dface.h"
#include "input.h"

// I'm using this as a #define so the compiler can optimize the
// modulo operation
#define PERIODIC_SAVE_INTERVAL 5000 // milliseconds

const int INVALID_STATE = 99;

void DoFun(int frameskip, int periodic_saves);

int LoadGame(const char *path);
int CloseGame(void);
uint64 FCEUD_GetTime();

int FCEUD_LoadMovie(const char *name, char *romname);
int FCEUD_DriverReset();
int FCEUD_VideoRegionSave(int pal);
int FCEUD_ReloadConfig(void);
void FCEUD_SetAutoResume(int val);
#ifndef RETROFW
void FCEUD_SetVideoFilter(int video_filter);
#endif

void FCEUI_FDSFlip(void);

extern int dendy;
extern int pal_emulation;
extern bool swapDuty;
extern bool paldeemphswap;

#endif
