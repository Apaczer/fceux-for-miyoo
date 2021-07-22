#ifndef __FCEU_DINGOO_H
#define __FCEU_DINGOO_H

#include <SDL/SDL.h>
#include "main.h"
#include "dface.h"
#include "input.h"

void DoFun(int frameskip, int);

int LoadGame(const char *path);
int CloseGame(void);

int FCEUD_LoadMovie(const char *name, char *romname);
int FCEUD_DriverReset();
int FCEUD_VideoRegionSave(int pal);
int FCEUD_ReloadConfig(void);

void FCEUI_FDSFlip(void);

extern int dendy;
extern int pal_emulation;
extern bool swapDuty;
extern bool paldeemphswap;

#endif
