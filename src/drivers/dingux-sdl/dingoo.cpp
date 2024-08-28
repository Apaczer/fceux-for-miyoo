#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <limits.h>
#include <math.h>
#include <libgen.h>

#include "main.h"
#include "throttle.h"
#include "config.h"

#include "../common/cheat.h"
#include "../../fceu.h"
#include "../../movie.h"
#include "../../version.h"
#ifdef _S9XLUA_H
#include "../../fceulua.h"
#endif

#include "input.h"
#include "dface.h"

#include "gui/gui.h"

#include "SDL/SDL.h"
#include "dingoo.h"
#include "dingoo-video.h"
#include "dummy-netplay.h"

#include "../common/configSys.h"
#include "../../utils/timeStamp.h"
#include "../../oldmovie.h"
#include "../../types.h"

#ifdef CREATE_AVI
#include "../videolog/nesvideos-piece.h"
#endif

#ifdef WIN32
#include <windows.h>
#endif

#ifdef OD2014
#define OD_DOWNSCALE_FILE "/sys/devices/platform/jz-lcd.0/sharpness_downscaling"
#define OD_UPSCALE_FILE   "/sys/devices/platform/jz-lcd.0/sharpness_upscaling"
#endif

extern void InitGuiVideo();

extern double g_fpsScale;

extern bool MaxSpeed;

extern int globalCheatDisabled;
extern int disableAutoLSCheats;

int isloaded;

int closeFinishedMovie = 0;

// NOTE: an ugly hack to get
// mouse support
int showmouse = 0;
int mousespeed = 3;

int showfps = 0;

int master_volume = 100;

bool turbo = false;

int CloseGame(void);

static int inited = 0;

int eoptions = 0;

static int fpsthrottle = 0;
static int frameskip = 0;
static int periodic_saves = 0;

static void DriverKill(void);
static int DriverInitialize(FCEUGI *gi);
uint64 FCEUD_GetTime();
int gametype = 0;
#ifdef CREATE_AVI
int mutecapture;
#endif
static int noconfig;

int pal_emulation;
int dendy;
bool swapDuty;

// Taken from mame084s, so credits to mame dev team
// frameskip table
static const int frameskip_table[12][12] =
{
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,1 },
	{ 0,0,0,0,0,1,0,0,0,0,0,1 },
	{ 0,0,0,1,0,0,0,1,0,0,0,1 },
	{ 0,0,1,0,0,1,0,0,1,0,0,1 },
	{ 0,1,0,0,1,0,1,0,0,1,0,1 },
	{ 0,1,0,1,0,1,0,1,0,1,0,1 },
	{ 0,1,0,1,1,0,1,0,1,1,0,1 },
	{ 0,1,1,0,1,1,0,1,1,0,1,1 },
	{ 0,1,1,1,0,1,1,1,0,1,1,1 },
	{ 0,1,1,1,1,1,0,1,1,1,1,1 },
	{ 0,1,1,1,1,1,1,1,1,1,1,1 }
};

// originally in src/drivers/common/vidblit.cpp
bool   paldeemphswap   = 0;

char* DriverUsage = "\
		Option         Value   Description\n\
		--pal          {0|1}   Use PAL timing.\n\
		--newppu       {0|1}   Enable the new PPU core. (WARNING: May break savestates)\n\
		--inputcfg     d       Configures input device d on startup.\n\
		--gamegenie    {0|1}   Enable emulated Game Genie.\n\
		--frameskip    x       Set # of frames to skip per emulated frame.\n\
		--xres         x       Set horizontal resolution for full screen mode.\n\
		--yres         x       Set vertical resolution for full screen mode.\n\
		--autoscale    {0|1}   Enable autoscaling in fullscreen. \n\
		--keepratio    {0|1}   Keep native NES aspect ratio when autoscaling. \n\
		--(x/y)scale   x       Multiply width/height by x. \n\
		(Real numbers >0 with OpenGL, otherwise integers >0).\n\
		--(x/y)stretch {0|1}   Stretch to fill surface on x/y axis (OpenGL only).\n\
		--bpp       {8|16|32}  Set bits per pixel.\n\
		--opengl       {0|1}   Enable OpenGL support.\n\
		--fullscreen   {0|1}   Enable full screen mode.\n\
		--noframe      {0|1}   Hide title bar and window decorations.\n\
		--special      {1-4}   Use special video scaling filters\n\
		(1 = hq2x 2 = Scale2x 3 = hq3x 4 = Scale3x)\n\
		--palette      f       Load custom global palette from file f.\n\
		--sound        {0|1}   Enable sound.\n\
		--soundrate	   x       Set sound playback rate to x Hz.\n\
		--soundq     {0|1|2}   Set sound quality. (0=Low;1=High;2=Very High)\n\
		--soundbufsize x       Set sound buffer size to x ms.\n\
		--volume     {0-256}   Set volume to x.\n\
		--soundrecord  f       Record sound to file f.\n\
		--input(1,2)   d       Set the input device for input 1 or 2.\n\
		Devices:  gamepad zapper powerpad.0 powerpad.1 arkanoid\n\
		--input(3,4)   d       Set the famicom expansion device for input(3, 4)\n\
		Devices: quizking hypershot mahjong toprider ftrainer\n\
		familykeyboard oekakids arkanoid shadow bworld 4player\n\
		--playmov      f       Play back a recorded FCM/FM2 movie from filename f.\n\
		--pauseframe   x       Pause movie playback at frame x.\n\
		--fcmconvert   f       Convert fcm movie file f to fm2.\n\
		--ripsubs      f       Convert movie's subtitles to srt\n\
		--subtitles    {0,1}   Enable subtitle display\n\
		--fourscore    {0,1}   Enable fourscore emulation\n\
		--no-config    {0,1}   Use default config file and do not save\n\
		--net          s       Connect to server 's' for TCP/IP network play.\n\
		--port         x       Use TCP/IP port x for network play.\n\
		--user         x       Set the nickname to use in network play.\n\
		--pass         x       Set password to use for connecting to the server.\n\
		--netkey       s       Use string 's' to create a unique session for the game loaded.\n\
		--players      x       Set the number of local players.\n";

// these should be moved to the man file
//--nospritelim  {0|1}   Disables the 8 sprites per scanline limitation.\n
//--trianglevol {0-256}  Sets Triangle volume.\n
//--square1vol  {0-256}  Sets Square 1 volume.\n
//--square2vol  {0-256}  Sets Square 2 volume.\n
//--noisevol	{0-256}  Sets Noise volume.\n
//--pcmvol	  {0-256}  Sets PCM volume.\n
//--lowpass	  {0|1}   Enables low-pass filter if x is nonzero.\n
//--doublebuf	{0|1}   Enables SDL double-buffering if x is nonzero.\n
//--slend	  {0-239}   Sets the last drawn emulated scanline.\n
//--ntsccolor	{0|1}   Emulates an NTSC TV's colors.\n
//--hue		   x	  Sets hue for NTSC color emulation.\n
//--tint		  x	  Sets tint for NTSC color emulation.\n
//--slstart	{0-239}   Sets the first drawn emulated scanline.\n
//--clipsides	{0|1}   Clips left and rightmost 8 columns of pixels.\n

// global configuration object
Config *g_config;

static void ShowUsage(char *prog) {
	printf("\nUsage is as follows:\n%s <options> filename\n\n", prog);
	puts("Options:");
	puts(DriverUsage);
#ifdef _S9XLUA_H
	puts ("--loadlua      f        Loads lua script from filename f.");
#endif
#ifdef CREATE_AVI
	puts ("--videolog     c        Calls mencoder to grab the video and audio streams to\n					   encode them. Check the documentation for more on this.");
	puts ("--mute        {0|1}     Mutes FCEUX while still passing the audio stream to\n					   mencoder.");
#endif
	puts("");
}

/**
 * Prints an error string to STDOUT.
 */
void FCEUD_PrintError(char *s) {
	puts(s);
}

/**
 * Prints the given string to STDOUT.
 */
void FCEUD_Message(char *s) {
	fputs(s, stdout);
}

/**
 * Reload game config or default config
 */
int FCEUD_ReloadConfig(void) {
	int pal;

	g_config->getOption("SDL.PAL", &pal);
	if (g_config->reload(FCEU_MakeFName(FCEUMKF_CFG, 0, 0))==-1)
	    return 0;
	g_config->setOption("SDL.PAL", pal);

	// Update emulator core configs
	UpdateEMUCore(g_config);

	return 1;
}

/**
 * Loads a game, given a full path/filename.  The driver code must be
 * initialized after the game is loaded, because the emulator code
 * provides data necessary for the driver code(number of scanlines to
 * render, what virtual input devices to use, etc.).
 */
int LoadGame(const char *path) {
	CloseGame();
	if (!FCEUI_LoadGame(path, 1)) {
		return 0;
	}
	ParseGIInput(GameInfo);
	RefreshThrottleFPS();

#ifdef FRAMESKIP
	// Update frameskip value
	g_config->getOption("SDL.Frameskip", &frameskip);
#endif

	if (!DriverInitialize(GameInfo)) {
		return (0);
	}

	// set pal/ntsc
	int id;
	//g_config->getOption("SDL.PAL", &id);
    g_config->getOption("SDL.Region", &id);
	FCEUI_SetRegion(id,1);

	std::string filename;
	g_config->getOption("SDL.Sound.RecordFile", &filename);
	if (filename.size()) {
		if (!FCEUI_BeginWaveRecord(filename.c_str())) {
			g_config->setOption("SDL.Sound.RecordFile", "");
		}
	}

	// Set mouse cursor's movement speed
	g_config->getOption("SDL.MouseSpeed", &mousespeed);
	g_config->getOption("SDL.ShowMouseCursor", &showmouse);

	// Show or not to show fps, that is the cuestion ...
	g_config->getOption("SDL.ShowFPS", &showfps);
	g_config->getOption("SDL.FPSThrottle", &fpsthrottle);

	// Update emulator core options from last game loaded
	UpdateEMUCore(g_config);
	// Update configs for input system
	UpdateInputConfig(g_config);

	isloaded = 1;

	FCEUD_NetworkConnect();
	return 1;
}

/**
 * Closes a game.  Frees memory, and deinitializes the drivers.
 */
int CloseGame() {
	std::string filename;

	if (!isloaded) {
		return (0);
	}

	FCEUI_CloseGame();
	DriverKill();
	isloaded = 0;
	GameInfo = 0;

	g_config->getOption("SDL.Sound.RecordFile", &filename);
	if (filename.size()) {
		FCEUI_EndWaveRecord();
	}

	InputUserActiveFix();
	return (1);
}

void FCEUD_Update(uint8 *XBuf, int32 *Buffer, int Count);

void DoFun(int frameskip, int periodic_saves)
{
	uint8 *gfx = 0;
	int32 *sound = 0;
	int32 ssize = 0;
	static int fskipc = 0;
	static int frameskip_counter = 0;

	//TODO peroidic saves, working on it right now
	if (periodic_saves && FCEUD_GetTime() % PERIODIC_SAVE_INTERVAL < 30){
		FCEUI_SaveState(NULL, false);
	}
#ifdef FRAMESKIP
	/* Frameskip decision based on the audio buffer */
	if (frameskip == -1) {
	    // Fill up the audio buffer with up to 6 frames dropped.
	    int FramesSkipped = 0;
	    while (GameInfo
		&& GetBufferedSound() < GetBufferSize() * 3 / 2
		&& ++FramesSkipped < 6) {
		    FCEUI_Emulate(&gfx, &sound, &ssize, 1);
		    FCEUD_Update(NULL, sound, ssize);
	    }

	    // Force at least one frame to be displayed.
	    if (GameInfo) {
		    FCEUI_Emulate(&gfx, &sound, &ssize, 0);
		    FCEUD_Update(gfx, sound, ssize);
	    }

	    // Then render all frames while audio is sufficient.
	    while (GameInfo
		&& GetBufferedSound() > GetBufferSize() * 3 / 2) {
		    FCEUI_Emulate(&gfx, &sound, &ssize, 0);
		    FCEUD_Update(gfx, sound, ssize);
	    }
	}
	else
	{
	//fskipc = (fskipc + 1) % (frameskip + 1);
	frameskip_counter = (frameskip_counter + 1) % 12;
	fskipc = frameskip_table[frameskip][frameskip_counter];
#endif

	if(NoWaiting) {
	    gfx = 0;
	}
	FCEUI_Emulate(&gfx, &sound, &ssize, fskipc);
	FCEUD_Update(gfx, sound, ssize);

	//if(opause!=FCEUI_EmulationPaused()) {
	//      opause=FCEUI_EmulationPaused();
	//      SilenceSound(opause);
	//}

	if (fpsthrottle)
		while ( SpeedThrottle() ) { }
#ifdef FRAMESKIP
	}
#endif
}

/**
 * Initialize all of the subsystem drivers: video, audio, and joystick.
 */
static int DriverInitialize(FCEUGI *gi) {
	if (InitVideo(gi) < 0)
		return 0;
	inited |= 4;

	if (InitSound())
		inited |= 1;

	if (InitJoysticks())
		inited |= 2;

	int fourscore = 0;
	g_config->getOption("SDL.FourScore", &fourscore);
	eoptions &= ~EO_FOURSCORE;
	if (fourscore)
		eoptions |= EO_FOURSCORE;

	InitInputInterface();

	FCEUGUI_Reset(gi);
	return 1;
}

/**
 * Set Video Region option and save it.
 */
int FCEUD_VideoRegionSave(int pal) {
	int val;

	if (pal)
		val = 1;
	else
		val = 0;

	if (g_config->setOption("SDL.PAL", val))
	    return 0;

	// 20200720 FIX
	// Do not save config here, this breaks the save settings per game.
	// See function LoadGame
	// Save game config file
	// g_config->save(FCEU_MakeFName(FCEUMKF_CFG, 0, 0));
	return 1;
}

/**
 * Resets sound and video subsystem drivers.
 */
int FCEUD_DriverReset() {
	// Save game config file
	g_config->save(FCEU_MakeFName(FCEUMKF_CFG, 0, 0));

#ifdef FRAMESKIP
	// Update frameskip value
	g_config->getOption("SDL.Frameskip", &frameskip);
#endif

	// Kill drivers first
	if (inited & 4)
		KillVideo();
	if (inited & 1)
		KillSound();

	if (InitVideo(GameInfo) < 0)
		return 0;
	inited |= 4;

	if (InitSound())
		inited |= 1;

	// Set mouse cursor's movement speed
	g_config->getOption("SDL.MouseSpeed", &mousespeed);
	g_config->getOption("SDL.ShowMouseCursor", &showmouse);

	// Set showfps variable and throttle
	g_config->getOption("SDL.ShowFPS", &showfps);
	g_config->getOption("SDL.FPSThrottle", &fpsthrottle);

	return 1;
}

/**
 * Shut down all of the subsystem drivers: video, audio, and joystick.
 */
static void DriverKill() {
	// Save only game config file
	g_config->save(FCEU_MakeFName(FCEUMKF_CFG, 0, 0));

	if (inited & 2)
		KillJoysticks();
	if (inited & 4)
		KillVideo();
	if (inited & 1)
		KillSound();
	inited = 0;
}

/**
 * Update the video, audio, and input subsystems with the provided
 * video (XBuf) and audio (Buffer) information.
 */
void FCEUD_Update(uint8 *XBuf, int32 *Buffer, int Count)
{
	WriteSound(Buffer,Count);

	if (XBuf && (inited&4)) 
	{
		BlitScreen(XBuf);
	}
    
	FCEUD_UpdateInput();
}

/**
 * Opens a file to be read a byte at a time.
 */
EMUFILE_FILE* FCEUD_UTF8_fstream(const char *fn, const char *m)
{
	std::ios_base::openmode mode = std::ios_base::binary;
	if(!strcmp(m,"r") || !strcmp(m,"rb"))
		mode |= std::ios_base::in;
	else if(!strcmp(m,"w") || !strcmp(m,"wb"))
		mode |= std::ios_base::out | std::ios_base::trunc;
	else if(!strcmp(m,"a") || !strcmp(m,"ab"))
		mode |= std::ios_base::out | std::ios_base::app;
	else if(!strcmp(m,"r+") || !strcmp(m,"r+b"))
		mode |= std::ios_base::in | std::ios_base::out;
	else if(!strcmp(m,"w+") || !strcmp(m,"w+b"))
		mode |= std::ios_base::in | std::ios_base::out | std::ios_base::trunc;
	else if(!strcmp(m,"a+") || !strcmp(m,"a+b"))
		mode |= std::ios_base::in | std::ios_base::out | std::ios_base::app;
	return new EMUFILE_FILE(fn, m);
}


/**
 * Opens a file, C++ style, to be read a byte at a time.
 */
FILE *FCEUD_UTF8fopen(const char *fn, const char *mode) {
	return (fopen(fn, mode));
}

#define __COMPILER__STRING__ "gcc " __VERSION__
static const char *s_CompilerString = __COMPILER__STRING__;
/**
 * Returns the compiler string.
 */
const char *FCEUD_GetCompilerString() {
	return s_CompilerString;
}

/**
 * Unimplemented.
 */
void FCEUD_DebugBreakpoint() {
	return;
}

/**
 * Unimplemented.
 */
void FCEUD_FlushTrace()
{
}

/**
 * Unimplemented.
 */
void FCEUD_TraceInstruction() {
	return;
}

/**
 * Convert FCM movie to FM2 . 
 * Returns 1 on success, otherwise 0.
 */
int FCEUD_ConvertMovie(const char *name, char *outname) {
	int okcount = 0;
	std::string infname = name;

	// produce output filename
	std::string tmp;
	size_t dot = infname.find_last_of(".");
	if (dot == std::string::npos)
		tmp = infname + ".fm2";
	else
		tmp = infname.substr(0, dot) + ".fm2";

	MovieData md;
	EFCM_CONVERTRESULT result = convert_fcm(md, infname);

	if (result == FCM_CONVERTRESULT_SUCCESS) {
		okcount++;
		EMUFILE_FILE* outf = FCEUD_UTF8_fstream(tmp, "wb");
		md.dump(outf, false);
		delete outf;
		FCEUD_Message("Your file has been converted to FM2.\n");

		strncpy(outname, tmp.c_str(), 128);
		return 1;
	} else {
		FCEUD_Message("Something went wrong while converting your file...\n");
		return 0;
	}

	return 0;
}

/* 
 * Loads a movie, if fcm movie will convert first.  And will
 * ask for rom too, returns 1 on success, 0 otherwise.
 */
int FCEUD_LoadMovie(const char *name, char *romname) {
	std::string s = std::string(name);

	// Convert to fm2 if necessary ...
	if (s.find(".fcm") != std::string::npos) {
		char tmp[128];
		if (!FCEUD_ConvertMovie(name, tmp))
			return 0;
		s = std::string(tmp);
	}

	if (s.find(".fm2") != std::string::npos) {
		// WARNING: Must load rom first
		// Launch file browser to search for movies's rom file.
		const char *types[] = { ".nes", ".fds", ".zip", NULL };
		if (!RunFileBrowser(NULL, romname, types, "Select movie's rom file")) {
			FCEUI_printf("WARNING: Must load a rom to play the movie! %s\n",
					s.c_str());
			return 0;
		}

		if (LoadGame(romname) != 1)
			return -1;

		static int pauseframe;
		g_config->getOption("SDL.PauseFrame", &pauseframe);
		g_config->setOption("SDL.PauseFrame", 0);
		FCEUI_printf("Playing back movie located at %s\n", s.c_str());
		FCEUI_LoadMovie(s.c_str(), false, pauseframe ? pauseframe : false);
	} else {
		// Must be a rom file ...
		return 0;
	}

	return 1;
}

/**
 * The main loop for the SDL.
 */
#ifdef WIN32
#undef main
#endif
int main(int argc, char *argv[]) {

	int error;

	FCEUD_Message("\nStarting " FCEU_NAME_AND_VERSION "...\n");

#ifdef WIN32
	/* Taken from win32 sdl_main.c */
	SDL_SetModuleHandle(GetModuleHandle(NULL));
#endif

	/* SDL_INIT_VIDEO Needed for (joystick config) event processing? */
	if(SDL_Init(SDL_INIT_VIDEO)) {
		printf("Could not initialize SDL: %s.\n", SDL_GetError());
		return(-1);
	}

	// Initialize the configuration system
	g_config = InitConfig();

	if (!g_config) {
		SDL_Quit();
		return -1;
	}

	// For remote debug:
	// No rom is supplied so file selector will be used, but the background
	// is not loaded becouse the current dir is not stablished and it fail
	// to display any file,
	chdir(dirname(argv[0]));

	// Initialize the fceu320 gui
	FCEUGUI_Init(NULL);

	// initialize the infrastructure
	error = FCEUI_Initialize();

	if (error != 1) {
		ShowUsage(argv[0]);
		SDL_Quit();
		return -1;
	}

	int romIndex = g_config->parse(argc, argv);

	// This is here so that a default fceux.cfg will be created on first
	// run, even without a valid ROM to play.
	// Unless, of course, there's actually --no-config given
	// mbg 8/23/2008 - this is also here so that the inputcfg routines can
	// have a chance to dump the new inputcfg to the fceux.cfg
	// in case you didnt specify a rom filename
	g_config->getOption("SDL.NoConfig", &noconfig);
	if (!noconfig)
		g_config->save();
	
	// Cheats
	g_config->getOption("SDL.CheatDisabled", &globalCheatDisabled);
	g_config->getOption("SDL.AutoLSCheatsDisabled", &disableAutoLSCheats);

	std::string s;
	g_config->getOption("SDL.InputCfg", &s);

	// update the input devices
	UpdateInput(g_config);

	// check for a .fcm file to convert to .fm2
	g_config->getOption("SDL.FCMConvert", &s);
	g_config->setOption("SDL.FCMConvert", "");

	if (!s.empty()) {
		int okcount = 0;
		std::string infname = s.c_str();
		// produce output filename
		std::string outname;
		size_t dot = infname.find_last_of(".");
		if (dot == std::string::npos)
			outname = infname + ".fm2";
		else
			outname = infname.substr(0, dot) + ".fm2";

		MovieData md;
		EFCM_CONVERTRESULT result = convert_fcm(md, infname);

		if (result == FCM_CONVERTRESULT_SUCCESS) {
			okcount++;
			EMUFILE_FILE* outf = FCEUD_UTF8_fstream(outname, "wb");
			md.dump(outf, false);
			delete outf;
			FCEUD_Message("Your file has been converted to FM2.\n");
		} else
			FCEUD_Message(
					"Something went wrong while converting your file...\n");

		DriverKill();
		SDL_Quit();
		return 0;
	}

	int autoResume=0;
	g_config->getOption("SDL.AutoResume", &autoResume);
	FCEUD_SetAutoResume(autoResume);

	// check to see if movie messages are disabled
	int mm;
	g_config->getOption("SDL.MovieMsg", &mm);
	FCEUI_SetAviDisableMovieMessages(mm == 0);

	// check for a .fm2 file to rip the subtitles
	g_config->getOption("SDL.RipSubs", &s);
	g_config->setOption("SDL.RipSubs", "");
	if (!s.empty()) {
		MovieData md;
		std::string infname;
		infname = s.c_str();
		FCEUFILE *fp = FCEU_fopen(s.c_str(), 0, "rb", 0);

		// load the movie and and subtitles
		extern bool LoadFM2(MovieData&, EMUFILE*, int, bool);
		LoadFM2(md, fp->stream, INT_MAX, false);
		LoadSubtitles(md); // fill subtitleFrames and subtitleMessages
		delete fp;

		// produce .srt file's name and open it for writing
		std::string outname;
		size_t dot = infname.find_last_of(".");
		if (dot == std::string::npos)
			outname = infname + ".srt";
		else
			outname = infname.substr(0, dot) + ".srt";
		FILE *srtfile;
		srtfile = fopen(outname.c_str(), "w");

		if (srtfile != NULL) {
			extern std::vector<int> subtitleFrames;
			extern std::vector<std::string> subtitleMessages;
			float fps = (md.palFlag == 0 ? 60.0988 : 50.0069); // NTSC vs PAL
			float subduration = 3; // seconds for the subtitles to be displayed
			for (int i = 0; i < subtitleFrames.size(); i++) {
				fprintf(srtfile, "%i\n", i + 1); // starts with 1, not 0
				double seconds, ms, endseconds, endms;
				seconds = subtitleFrames[i] / fps;
				if (i + 1 < subtitleFrames.size()) { // there's another subtitle coming after this one
					if (subtitleFrames[i + 1] - subtitleFrames[i] < subduration
							* fps) { // avoid two subtitles at the same time
						endseconds = (subtitleFrames[i + 1] - 1) / fps; // frame x: subtitle1; frame x+1 subtitle2
					} else
						endseconds = seconds + subduration;
				} else
					endseconds = seconds + subduration;

				ms = modf(seconds, &seconds);
				endms = modf(endseconds, &endseconds);
				// this is just beyond ugly, don't show it to your kids
				fprintf(
						srtfile,
						"%02.0f:%02d:%02d,%03d --> %02.0f:%02d:%02d,%03d\n", // hh:mm:ss,ms --> hh:mm:ss,ms
						floor(seconds / 3600), (int) floor(seconds / 60) % 60,
						(int) floor(seconds) % 60, (int) (ms * 1000), floor(
								endseconds / 3600),
						(int) floor(endseconds / 60) % 60, (int) floor(
								endseconds) % 60, (int) (endms * 1000));
				fprintf(srtfile, "%s\n\n", subtitleMessages[i].c_str()); // new line for every subtitle
			}
			fclose(srtfile);
			printf("%d subtitles have been ripped.\n",
					(int) subtitleFrames.size());
		} else
			FCEUD_Message("Couldn't create output srt file...\n");

		DriverKill();
		SDL_Quit();
		return 0;
	}

	// check for --help or -h and display usage
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			ShowUsage(argv[0]);
			SDL_Quit();
			return 0;
		}
	}

	/*
	 // if there is no rom specified launch the file browser
	 if(romIndex <= 0) {
	 ShowUsage(argv[0]);
	 FCEUD_Message("\nError parsing command line arguments\n");
	 SDL_Quit();
	 return -1;
	 }
	 */

	// update the emu core
	UpdateEMUCore(g_config);

#ifdef CREATE_AVI
	g_config->getOption("SDL.VideoLog", &s);
	g_config->setOption("SDL.VideoLog", "");
	if(!s.empty()) {
		NESVideoSetVideoCmd(s.c_str());
		LoggingEnabled = 1;
		g_config->getOption("SDL.MuteCapture", &mutecapture);
	} else
	mutecapture = 0;
#endif

	{
		int id;
		g_config->getOption("SDL.InputDisplay", &id);
		extern int input_display;
		input_display = id;
		// not exactly an id as an true/false switch; still better than creating another int for that
		g_config->getOption("SDL.SubtitleDisplay", &id);
		extern bool movieSubtitles;
		movieSubtitles = id != 0;
	}

	// load the hotkeys from the config life
	setHotKeys();

	if (romIndex >= 0) {
		// load the specified game
		error = LoadGame(argv[romIndex]);
		if (error != 1) {
			DriverKill();
			SDL_Quit();
			return -1;
		}
		g_config->setOption("SDL.LastOpenFile", argv[romIndex]);
	} else {
		// Launch file browser
		const char *types[] = { ".nes", ".fds", ".zip", ".fcm", ".fm2", ".nsf",
				NULL };
		char filename[128] = "\0", romname[128];

		InitVideo(0); inited |= 4; // Hack to init video mode before running gui
		// Init screen to GUI Resolution
		InitGuiVideo();

		#ifdef WIN32
		if (!RunFileBrowser("D:\\", filename, types)) {
		#else
		if (!RunFileBrowser(NULL, filename, types) || filename[0] == '\0') {
		#endif
			DriverKill();
			SDL_Quit();
			return -1;
		}

		if (!isloaded)
			DriverKill();
		// Is this a movie?
		if (!(error = FCEUD_LoadMovie(filename, romname)))
			error = LoadGame(filename);

		if (error != 1) {
			DriverKill();
			SDL_Quit();
			return -1;
		}
		g_config->setOption("SDL.LastOpenFile", filename);
	}

	// movie playback
	g_config->getOption("SDL.Movie", &s);
	g_config->setOption("SDL.Movie", "");
	if (s != "") {
		if (s.find(".fm2") != std::string::npos) {
			static int pauseframe;
			g_config->getOption("SDL.PauseFrame", &pauseframe);
			g_config->setOption("SDL.PauseFrame", 0);
			FCEUI_printf("Playing back movie located at %s\n", s.c_str());
			FCEUI_LoadMovie(s.c_str(), false, pauseframe ? pauseframe : false);
		} else
			FCEUI_printf("Sorry, I don't know how to play back %s\n", s.c_str());
	}

    int periodic_saves;
    int save_state;
    g_config->getOption("SDL.PeriodicSaves", &periodic_saves);
    g_config->getOption("SDL.AutoSaveState", &save_state);
    if(periodic_saves && save_state < 10 && save_state >= 0){
        FCEUI_SelectState(save_state, 0);
    } else {
        periodic_saves = 0;
    }

#ifdef _S9XLUA_H
	// load lua script if option passed
	g_config->getOption("SDL.LuaScript", &s);
	g_config->setOption("SDL.LuaScript", "");
	if (s != "")
	FCEU_LoadLuaCode(s.c_str());
#endif

	{
		int id;
		g_config->getOption("SDL.NewPPU", &id);
		if (id)
			newppu = 1;
	}

	g_config->getOption("SDL.Frameskip", &frameskip);

	// loop playing the game
	while(GameInfo)
	{
		DoFun(frameskip,periodic_saves);
	}

	printf("Closing Game...\n");
	CloseGame();

	printf("Exiting Infrastructure...\n");
	// exit the infrastructure
	FCEUI_Kill();
	SDL_Quit();
	
	printf("Done!\n");
	return 0;
}

/**
 * Get the time in ticks.
 */
uint64 FCEUD_GetTime() {
	uint64 t;

	if (FCEU::timeStampModuleInitialized())
	{
		FCEU::timeStampRecord ts;

		ts.readNew();

		t = ts.toCounts();
	}
	else
	{
		t = (double)SDL_GetTicks();

		t = t * 1e-3;
	}
	return t;
}

/**
 * Get the tick frequency in Hz.
 */
uint64 FCEUD_GetTimeFreq(void)
{
	// SDL_GetTicks() is in milliseconds
	uint64 f = 1000;

	if (FCEU::timeStampModuleInitialized())
	{
		f = FCEU::timeStampRecord::countFreq();
	}
	return f;
}

/**
 * Prints a textual message without adding a newline at the end.
 *
 * @param text The text of the message.
 *
 * TODO: This function should have a better name.
 **/
void FCEUD_Message(const char *text) {
	fputs(text, stdout);
#ifdef _GTK
	pushOutputToGTK(text);
#endif
}

/**
 * Shows an error message in a message box.
 * (For now: prints to stderr.)
 *
 * @param errormsg Text of the error message.
 **/
void FCEUD_PrintError(const char *errormsg) {
	fprintf(stderr, "%s\n", errormsg);
}

// dummy functions

#define DUMMY(__f) void __f(void) {printf("%s\n", #__f); FCEU_DispMessage("Not implemented.", 0);}
DUMMY(FCEUD_HideMenuToggle)
DUMMY(FCEUD_MovieReplayFrom)
DUMMY(FCEUD_ToggleStatusIcon)
DUMMY(FCEUD_AviRecordTo)
DUMMY(FCEUD_AviStop)
void FCEUI_AviVideoUpdate(const unsigned char* buffer) {
}
int FCEUD_ShowStatusIcon(void) {
	return 0;
}
bool FCEUI_AviIsRecording(void) {
	return false;
}
void FCEUI_UseInputPreset(int preset) {
}
bool FCEUD_PauseAfterPlayback() {
	return false;
}
void FCEUD_TraceInstruction(uint8 *opcode, int size)
{
   // Place holder to allow for compiling. GTK GUI doesn't support this. Qt Does.
}
void FCEUD_DebugBreakpoint(int bp_num)
{
    // Place holder to allow for compiling. GTK GUI doesn't support this. Qt Does.
}
// These are actually fine, but will be unused and overriden by the current UI code.
void FCEUD_TurboOn(void) {
	NoWaiting |= 1;
}
void FCEUD_TurboOff(void) {
	NoWaiting &= ~1;
}
void FCEUD_TurboToggle(void) {
	NoWaiting ^= 1;
}
void FCEUD_SetAutoResume(int val) {
    AutoResumePlay = val ? true : false;
}
#ifndef RETROFW
void FCEUD_SetVideoFilter(int video_filter) {
	// Filter and sharpness level 
	//   - "0" is nearest-neighbour
	//   - "1" is bilinear
	//   - 2-32 is bicubic
	char sharpness_level_c[3] = "2";

	if (video_filter > 32)
		video_filter = 32;
	else if (video_filter < 0)
		video_filter = 0;

	sprintf(sharpness_level_c, "%d", video_filter);
#ifndef OD2014
	setenv("SDL_VIDEO_KMSDRM_SCALING_SHARPNESS", sharpness_level_c, 1);
#else
	FILE *upscale_file = fopen(OD_UPSCALE_FILE, "wb");
	if (upscale_file) {
		fputs(sharpness_level_c, upscale_file);
		fclose(upscale_file);
	}

	FILE *downscale_file = fopen(OD_DOWNSCALE_FILE, "wb");
	if (downscale_file) {
		fputs(sharpness_level_c, downscale_file);
		fclose(downscale_file);
	}
#endif
}
#endif
FCEUFILE* FCEUD_OpenArchiveIndex(ArchiveScanRecord& asr, std::string &fname,
		int innerIndex) {
	return 0;
}
FCEUFILE* FCEUD_OpenArchive(ArchiveScanRecord& asr, std::string& fname,
		std::string* innerFilename) {
	return 0;
}
FCEUFILE* FCEUD_OpenArchiveIndex(ArchiveScanRecord& asr, std::string &fname, int innerIndex, int* userCancel) { return 0; }
FCEUFILE* FCEUD_OpenArchive(ArchiveScanRecord& asr, std::string& fname, std::string* innerFilename, int* userCancel) { return 0; }
ArchiveScanRecord FCEUD_ScanArchive(std::string fname) {
	return ArchiveScanRecord();
}

extern uint8 SelectDisk, InDisk;
extern int FDSSwitchRequested;

void FCEUI_FDSFlip(void)
{
    /* Taken from fceugc 
       the commands shouldn't be issued in parallel so
     * we'll delay them so the virtual FDS has a chance
     * to process them
    */
    if(FDSSwitchRequested == 0)
        FDSSwitchRequested = 1;
}

bool enableHUDrecording = false;
bool FCEUI_AviEnableHUDrecording()
{
	if (enableHUDrecording)
		return true;

	return false;
}
