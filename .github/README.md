## FCEUX for MiyooCFW

FCEUX is a cross platform, NTSC and PAL Famicom/NES and Dendy emulator

### Bios file:

To load FDS files bios file `disksys.rom` is needed. It must be placed at `$HOME/.fceux` directory or in the game directory.

### Build steps:
- cross-compile (MiyooCFW)  
`make -j$(nproc)`
or
`make -j$(nproc) miyoo-ipk`

- native (PC-Linux)  
`make -j6 LINUX=yes`