# FCEUX 2.4.0 for OpenDingux/RetroFW

FCEUX is a cross platform, NTSC and PAL Famicom/NES and Dendy emulator

This is a build for OpenDingux and RetroFW based on soarqin's fork with additions from gameblabla and pingflood's works and updated to the upstream version 2.4.0 (Released 24 June 2021).

You can see the upstream FCEUX changelog [here](http://fceux.com/web/help/WhatsNew240.html)

## Bios file

To load FDS files bios file `disksys.rom` is needed. It must be placed at `$HOME/.fceux` directory or in the game directory.

## Controls

FCEUX|Nes
-|-
Pad|Pad
A|A
B|B
X|TurboA
Y|TurboB
Select|Select
Start|Start
POWER, L2|Open menu
Select+Start|Open menu in RetroFW (Configurable)

#### Hotkeys

Hotkey|Action
-|-
R1 + A				|Save state (current slot from gui is used)
R1 + B 				|Load state (current slot from gui is used)
R1 + X				|Toggle fullscreen
R1 + Y				|Flip fds disk
R1 + UP				|Toggle framerate display
R1 + LEFT			|Insert vsuni coin
R1 + SELECT		|Save snapshot
R1 + START		|Pause emulation
L1					  |Toggle throttling

#### Controls in menus

Button|Action
-|-
 Up/Down           |Move to select a submenu or option
 A                 |Enter submenu
 Left/Right        |Change values for selected option
 B                 |Go back to previous menu or exit if you are in top menu

#### Controls in ROM Browser

Button|Action
-|-
 Up/Down           |Move to select file or directory
 A                 |Load selected file/enter directory
 B                 |Navigate up one directory level
 X                 |Exit from file browser
 Left/Right        |Select first/last file in current page or advanve one page up/down if first/last file is already selected
 L1                |Go to last file in current directory
 R1                |Go to first file in current directory
 Select            |Save current directory as default when open ROM browser

> L1 and R1 is equal to L and R for devices withouth L2 and R2 buttons.

## Compilation

Use `make` to compile. If your toolchain is not in `/opt/gcw0-toolchain` pass your path in the variable **TOOLCHAIN**.

Executable and opk base filenames can be changed with **TARGET** variable. By default the base filename is *fuse_od*. Upstream version and build date will be added to opk name.

For 2014 OpenDingux (Stock firmware o Rogue) pass the variable ODVERSION=2014

To compile for RetroFW pass *retrofw* value to **DEVICE** variable. For RetroFW opk and ipk files will be created.

Examples:

`make TOOLCHAIN=/opt/retrofw2-toolchain TARGET=fceux_retrofw DEVICE=retrofw -f Makefile`

`make TOOLCHAIN=/opt/opendingux-toolchain TARGET=fceux_od_beta -f Makefile`

`make TOOLCHAIN=/opt/gcw0-toolchain TARGET=fceux_gcw0 ODVERSION=2014 -f Makefile`

Build files are created in the `bin`directory.

## What's new

#### 21 July 2021

  - The screenshot menu option now saves directly when the A button is pressed. Until now you had to press the A button and then exit from menu to emulator for the screenshot to be saved.
> Screenshots are saved at `$HOME/.fceux/snaps` directory. Filenames follow the scheme `<rom name>-<sequential number>.png`.
  - Fixed a bug in the automatic video region change that caused the save settings per game to not work.
  - RetroFW: Add Start+Select combo to open menu. Configurable in Control option. (taken from pingflood's RetroFW port).
  - RetroFW. Add ipk build for RetroFW 1 devices.
  - OpenDingux: For hardware scaling in OpenDingux 2014 versions (stock firmware or Rogue), the full screen is forced by disabling the aspect ratio
  - Add manual to opk/ipk and updated the Readme.

#### 10 July 2021

  - Based on soarqin's fork and upgraded to upstream FCEUX 2.4.0 version with additions from gameblabla and pingflood's versions.
  - Changes to Makefile for beta OD and RetroFW 2 compilation
  - Fixed the exit from main menu
  - Load rom: pagination with left and right cursors changed to scroll one page
  - For OpenDingux beta: Set video refresh rate based on cartridge region (taken from gameblabla's port)
  - Automatically set video mode based on cartridge region
  - Fixed the Flip disc option from main menu option
  - Enable Flip disc option in main menu only if a FDS file is inserted (taken from pingflood's RetroFW port)
  - Try to load disksys.rom from current rom directory (taken from pingflood's RetroFW port)
  - For debug purposes: when launch FCEUX witouth cartridge launch emulator rom selector
  - Change button layout in Control Setup
  - Some other minor changes

## Thanks

Thanks go to all people who worked on each incarnation of FCEUX: the_gama, ValdikSS and DiegoSLTS, Steward-fu, soarqin, gameblabla, pingflood.