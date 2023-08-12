# FCEUX 2.6.5 for OpenDingux/RetroFW

FCEUX is a cross platform, NTSC and PAL Famicom/NES and Dendy emulator

This is a build for OpenDingux and RetroFW based on soarqin's fork with additions from gameblabla, pingflood and asoderq/sydarn2 works and updated to the upstream version 2.6.5 (Released 7 February 2023).

You can see the upstream FCEUX changelog [here](https://fceux.com/web/help/WhatsNew265.html)

## Bios file

To load FDS files bios file `disksys.rom` is needed. It must be placed at `$HOME/.fceux` directory or in the game directory.

## Auto-resume play

If enabled, FCEUX will make a special savestate every time you close ROM (loading another rom or exiting from emulator), and will automatically load the savestate when you open this ROM next time, so you can continue from where you left the game.

This savestates will be created in the `$HOME/.fceux/fcs` directory and it will be named `<filename's game>-resume.fcs`

## Cheats

*Adapted from FCEUX upstream help that you can [see here](http://fceux.com/web/help/CheatSearch.html)*

By default cheat files (.cht) are stored in the "cheats" subdirectory under the base FCEUX (`$HOME/.fceux/cheats`). The files are in a simple plain-text format. Each line represents a one-byte memory patch. The format is as follows(text in brackets [] represents optional parameters):

    [S][C][:]Address(hex):Value(hex):[Compare value:]Description

Example:

    040e:05:Infinite super power.

A colon(:) near the beginning of the line is used to disable the cheat. "S" denotes a cheat that is a read-substitute-style cheat(such as with Game Genie cheats), and a "C" denotes that the cheat has a compare value.

When a game is loaded, FCEUX will load any accompanying saved `.cht` file in `$HOME/.fceux/cheats` automatically. This can be changed by disabling Auto load/save in **Cheat browser**.

>ROM Detectives has an excellent guide and a collection of cheats for use directly with FCEUX ([see it here](http://www.romdetectives.com/Wiki/index.php?title=Cheat_Archive_-_FCEUX))

>Retro Game Corps also have an excellent cheats guide ([see it here](https://retrogamecorps.com/2020/09/06/rg350-cheats-guide/)) including a cht compilation file that I have used to test this functionality

### Cheat browser

You can access to Cheat browser from main menu or with hotkey `L1 + RIGHT`.

The Cheat browser will cheat the list of current loaded cheats. It will display cheat description or code if no description is available.

A `*` in front of chate name will be displayed for enabled cheats. You can enable/disable a cheat by selecting it and use `A` button.

If there are no loaded cheats for current rom you can import them from a `.cht` file located in any directory. For import use the `Y` button, it will init a browser in default ROM folder, navigate to directory where desired `.cht` file is located and load it using the `A` button.  You can import cheats also from submenu, see below for details.

When a cheat file is imported **it will overwrite any cheat previously loaded** and it will be saved in `$HOME/.fceux/cheats` for future auto load/save.

Cheats and auto load/save options can be changed in Cheat browser with Start and Select buttons respectively. Differently than upstream FCEUX this options will be saved in the configuration file. Also take into account that these are global options and not per game options.

With `X` button a submenu is open with different options, some of them for the selected cheat and other for global operations. From here you can toggle, delete selected cheat, delete all, import cheats or enable cheats globally and enable autoload/save cheats.

## Game Genie

You must place Game Genie rom in the `$HOME/.fceux` directory and it must be named `gg.rom`.

You can toggle it in `Settings Menu --> Main Setup` or with the hotkey `R1 + DOWN`.

When toggled a hard reset will be done if a rom was loaded.

## Overclock

*Adapted from FCEUX upstream help that you can [see here](http://fceux.com/web/help/Timing.html)*

#### Overclocking (old PPU only)
Overclocks the console by adding dummy scanlines to the usual PPU loop, causing CPU to run more cycles per frame. Can be done in two different ways: by adding Post-render scanlines (Before NMI in other emulators) and by adding Vblank scanlines (After NMI in other emulators). The method to be used depends on the game. Maximum value is 786.

#### Don't overclock 7-bit samples
Such samples are played by the game at the rate it wants, so by running extra cycles, it will generate extra samples. To prevent those from being sped up, this option allows to cancel all the dummy scanlines once a 7-bit sample starts. This hardly affects gameplay, since such samples cause heavy lag, preventing the game from actually operating, so disabling overclocking during them won't slow the game down.

#### Notes for Opendingux/RetroFW port
Overclock is limited to a maximum of 786 scanlines. Taking as reference 262 scanlines (1 NTSC frame) this is 4x CPU overclock.
With less power devices, as JZ4760(B), using more than 262 scanlines can produce a real slowdown to emulation and framerate.

- Tipically use is to use Post-render scanlines, but some games need VBlank scanlines as Kirby's Adventure or Contra force.
- The scanlines values to be added depend on the game, but as reference 1 NTSC frame is chosen (262 scanlines), but also 240 or 260 scanlines are typically used.
- In the overclock settings menu left and right cursors substract or add 10 scanlines for Post-Render or Vblank options. Maintaining pressed the `A` button the step value will be 1 scanline and maintaining pressed the `Y` button the step value will be 262 scanlines.

> See [this thread](http://tasvideos.org/forum/viewtopic.php?t=18323) at FCEUX forum with information about overclocking. In this post there is a document attached with a overclocking game database.

> See [This video](https://www.youtube.com/watch?v=VzQE4pLjhg4&t=323s) with a comparison side by side of some overclocked games.

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
R1 + DOWN			|Toggle Game Genie
R1 + LEFT			|Insert vsuni coin
R1 + RIGHT			|Open Cheat browser
R1 + SELECT		|Save snapshot
R1 + START		|Pause emulation
L1 + A				|Toggle throttling
L1 + B 				|Clip top/bottom (8 pixels each)
L1 + X				|Change Pixel Aspect Ratio
L1 + Y				|Clip sides (8 pixels each side)
L1 + UP				|Toggle de-emphasis bit swap
L1 + LEFT/RIGHT                 |OpenDingux: Decrease/Increase video filter value

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

#### Controls in Cheat Browser

Button|Action
-|-
 Up/Down           |Move to select cheat
 A           |Enable/disable cheat
 B           |Exit to previous menu
 X           |Exit to emulator
 Y           |Open browser to select a `.cht` file to import. This override current cheats
 Left/Right           |Select first/last cheat in current page or advanve one page up/down if first/last cheat is already selected
 L1           |Go to last cheat
 R1           |Go to first cheat
 Start           |Enable/disable cheats globally
 Select           |Enable/disable cheatfiles auto load/save golbally

##### Controls in Cheat Browser submenu

Button|Action
-|-
 Up/Down           |Move to select option
 A                 |Select option
 B                 |Exit to Cheat Browser

#### Controls in Overclock settings
Button|Action
-|-
 Up/Down           |Move to select option
 Left/Right        |Change values for selected option. For VBlank or Post-render options a default step of 10 scanlines is used.
 A                 |Change de step value to 1 scanline. *Maintain pressed while press left or right.*
 Y                 |Change de step value to 262 scanlines. *Maintain pressed while press left or right.*
 B                 |Go back to previous menu

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

`make TOOLCHAIN=/opt/lepus-toolchain TARGET=fceux_lepus DEVICE=lepus -f Makefile`

Build files are created in the `bin`directory.

## What's new

#### 7 August 2023
  - Updated to upstream 2.6.5. See upstream changes [here](https://fceux.com/web/help/WhatsNew265.html)

#### 18 February 2022
  - Updated to upstream 2.6.2. See upstream changes [here](https://fceux.com/web/help/WhatsNew262.html)
  - Change video mode to 320x240 when access cheats with hotkey
  - Added Pixel Aspect Ratio for hardware scaling (1:1, 8:7, 4:3).
  - Added configuration to change NTSC start & end scanlines.
  - Added L hotkeys:
     - L + A Turbo mode
     - L + B Clip top/bottom (8 pixels each)
     - L + X Change Pixel Aspect Ratio
     - L + Y Clip sides (8 pixels each side)
  - Matched throttle and sound implementations with the upstream Qt/SDL driver.
  - Added Integer Frame Rate option to better sync video with the video display refresh.
  - Added sound Buffer size option. It is expressed in ms. Default value is 128 and the valid values range is from 15 to 200.
  - Enabled Frameskip configuration with levels Auto and from 0 to 11. Default level is 0.
  - This changes have impact in previously configured games so you must revise its configurations:
    - FPS throttle 'Off' previously implements an implicit frameskip based on sound buffer. Now this frameskip is configured by setting the option Frameskip to 'Auto'.
    - FPS throttle 'On' now uses nanosleep to sync the game to the intended framerate.
    - The Frameskip levels (0-11) are done via a framekip table, the upstream FCEUX frameskipping is very aggresive. The frameskip table is borrowed from mame084 source so the thanks goes to MAME dev team.
    - Sound Buffer size is defaulted to 128 ms. Previous builds have 30 ms as default value.
  - Included upstream palette files in the opk file.
  - Rearranged config menu:
    - Add Palette settings submenu.
    - Move custom palette and NTSC Palette control to Palette settings menu.
    - Add Force Grayscale and De-emphasis Bit Swap options to Palette menu.
    - Add hotkey L1 + UP to toggle de-emphasis bit swap.
    - In Palette settings Custom palette search first for palette files in the `palettes` directory distributed with the opk. You can navigate to other directories, use the B button to go up in directory structure and A button to enter in a directory.
    - In Palette settings use the Left cursor to clean custom palette.
  - Added video filter option for hardware scaler. 
    - Only for OpenDingux (JZ47xx and JZ46xx). 
    - Add hotkey L + LEFT/RIGHT to decrease/increase the filter value (0-32). 
    - Values: 0 = Nearest, 1 = Bilinear, 2-32 = Bicubic.

#### 25 September 2021
  - Merged [asoderq/sydarn2 overclocking work](https://github.com/asoderq/fceux-for-retrogame/releases/tag/2021-09-11). As upstream it only works with Old PPU.
  - Added submenu in cheat browser with some new actions (delete cheat, delete all cheats) and other already existing options
  - Fixed enabling Scanline start and end when loading a rom. For this FCEUD_ReloadConfig is refactored to call UpdateEMUCore, also moving enabling autoresume to UpdateEMUCore.
  - Build for OpenDingux for lepus.
  - Fix video region autoset after last refactorings.
  - Merge latest commits from upstream.
  - For RetroFW and Lepus OD PGO builds has been used.

#### 25 July 2021
  - Some refactorings for Game Genie use and assign hotkey to toggle it.
  - Added autoresume. It can be enabled in `Main settings`. This is a feature from upstream FCEUX.
  - Added Cheat browser. With it you can import cheats from `.cht` files, that will be saved in `$HOME/.fceux/cheats` for later autoload, and enable/disable loaded cheats. Also you can enable/disable cheats and enable/disable the auto load/save of cheats files per game globally, both are by default enabled.

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

Thanks go to current maintainers of upstream FCEUX and all people who worked on each incarnation of FCEUX: the_gama, ValdikSS and DiegoSLTS, Steward-fu, soarqin, gameblabla, pingflood.
