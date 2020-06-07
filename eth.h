// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2005 *nixCoders team - don't forget to credits us
// $Id$

#ifndef ETH_H_
#define ETH_H_

#define ETH_VERSION "0.7-5-rc1"
#define ETH_CVAR_PREFIX "eth_"
#define ETH_PK3_FILE "eth.pk3"
//#define ETH_PRIVATE //1

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <GL/gl.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "sdk/src/cgame/cg_local.h"
#include "sdk/src/ui/ui_public.h"
#include "cg_functions.h"

#ifdef ETH_PRIVATE
	#include "private.h"
#endif

// --- extern Struct from the game ---
cg_t		cg;
cgs_t		cgs;
centity_t	cg_entities[MAX_GENTITIES];

vmCvar_t	 cg_fov;
vmCvar_t	 cg_bluelimbotime;
vmCvar_t	 cg_draw2D;
vmCvar_t	 cg_drawGun;
vmCvar_t	 cg_thirdPerson;
vmCvar_t	 cg_redlimbotime;

// --- cvars ---
// aimbot
vmCvar_t cv_aim;
vmCvar_t cv_aimvecz;
vmCvar_t cv_aimdead;
// visual
vmCvar_t cv_advert;
vmCvar_t cv_chams;
vmCvar_t cv_glow;
vmCvar_t cv_radar;
vmCvar_t cv_spawntimer;
vmCvar_t cv_stats;
vmCvar_t cv_wallhack;
// esp
vmCvar_t cv_espfade;
vmCvar_t cv_espname;
vmCvar_t cv_espname_bg;
vmCvar_t cv_espicon;
vmCvar_t cv_espicon_bg;
vmCvar_t cv_espdist;
// sound
vmCvar_t cv_sndmisc;
vmCvar_t cv_sndspree;
vmCvar_t cv_spreetime;
// misc
vmCvar_t cv_autoshoot;
vmCvar_t cv_bunnyjump;
vmCvar_t cv_sniperzoom;

// All entities info
typedef struct {
	qboolean isValidPlayer;
	qboolean isDead;
	qboolean isEnemy;
	qboolean isVisible;
	qboolean isInScreen;

	vec3_t target;
	float distance;

	vec4_t espColor;
	int screenX, screenY;
} ethEntity_t;

// For all console actions/commands
#define MAX_ACTION_LEN 100
typedef struct {
	int state;
	int defaultState;
	char startAction[MAX_ACTION_LEN];
	char stopAction[MAX_ACTION_LEN];
} ethAction_t;

typedef enum {
	ACTION_ATTACK,
	ACTION_BACKWARD,
	ACTION_BINDMOUSE1,
	ACTION_CROUCH,
	ACTION_JUMP,
	ACTION_PRONE,
	ACTION_RUN,
	ACTION_MAX // Number of actions
} ethActions_t;

// Sounds
typedef enum {
	// Misc sound
	SOUND_HEADSHOT1,
	SOUND_HEADSHOT2,
	SOUND_HUMILIATION,
	SOUND_PREPARE,
	// Spree sound
	// Level 1
	SOUND_DOUBLEKILL1,
	SOUND_EXCELENT,
	SOUND_IMPRESSIVE,
	SOUND_KILLINGSPREE1,
	SOUND_KILLINGSPREE2,
	SOUND_PERFECT,
	// Level 2
	SOUND_GODLIKE1,
	SOUND_GODLIKE2,
	SOUND_HOLYSHIT,
	SOUND_MULTIKILL1,
	SOUND_MULTIKILL2,
	SOUND_MULTIKILL3,
	SOUND_TRIPPLEKILL,
	// Level 3
	SOUND_DOMINATING1,
	SOUND_DOMINATING2,
	SOUND_ULTRAKILL1,
	SOUND_ULTRAKILL2,
	SOUND_ULTRAKILL3,
	// Level 4
	SOUND_MONSTERKILL1,
	SOUND_MONSTERKILL2,
	SOUND_UNREAL,
	SOUND_UNSTOPPABLE1,
	SOUND_UNSTOPPABLE2,
	SOUND_WICKEDSICK,
	SOUNDS_MAX
} ethSounds_t;

// Main eth struct
typedef struct {
	// Zero'ed every cg_frame
	ethEntity_t entities[MAX_GENTITIES];

	// Esp icons
	qhandle_t weaponIcons[WP_NUM_WEAPONS];
	qhandle_t classIcons[NUM_PLAYER_CLASSES];
	qhandle_t rankIcons[NUM_EXPERIENCE_LEVELS];

	// Color shaders
	qhandle_t blueShader;
	qhandle_t cyanShader;
	qhandle_t greenShader;
	qhandle_t redShader;
	qhandle_t tealShader;
	qhandle_t yellowShader;
	
	// Glow color shaders
	qhandle_t blueGlowShader;
	qhandle_t cyanGlowShader;
	qhandle_t greenGlowShader;
	qhandle_t redGlowShader;
	qhandle_t tealGlowShader;
	qhandle_t yellowGlowShader;

	// Sounds
	sfxHandle_t sounds[SOUNDS_MAX];

	// All actions
	ethAction_t actions[ACTION_MAX];

	// Name of the real current player not a spec one
	char playerName[MAX_QPATH];

	// Stats
	int killCount;
	int deathCount;
	int killSpreeCount;
	int firstKillSpreeTime;
	int	lastKillTime;

	// Menu
	int mouseX, mouseY, mouseButton;
	int isMenuOpen;
} eth_t;

eth_t eth;

/*
==============================
from engine.c
==============================
*/
typedef struct {
    vmCvar_t *vmCvar;
    char *cvarName;
    char *defaultString;
    int cvarFlags;
} cvarTable_t;

int wrap_CG_R_AddRefEntityToScene(refEntity_t *re);
int wrap_CG_R_RenderScene(refdef_t *refdef);
int wrap_CG_R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader);
void wrap_CG_DrawActiveFrame();
void wrap_CG_Init();
void wrap_CG_Shutdown();
void registerCvars();
void updateCvars();

/*
==============================
from hook.c
==============================
*/
#define CGAME_LIB_NAME "cgame.mp.i386.so"
#define UI_LIB_NAME "ui.mp.i386.so"
#define DLSYM_GOT 0x081be2b8 // dlsym's GOT entry on et.x86 2.60
void *cgameLibHandle;
void *uiLibHandle;
// System hooked functions
void *(*orig_dlsym) (void *handle, const char *symbol);
void *wrap_dlsym (void *handle, const char *symbol);
void *(*orig_dlopen) (const char *filename, int flag);
int (*orig_XNextEvent) (Display *display, XEvent *event_return);
// 'Pure pk3' unlocker
void unlockPurePk3();
void restorePurePk3Lock();
// Game hooked functions from "cgame.mp.i386.so"
void (*orig_dllEntry) (int (*syscallptr) (int arg,... ));
void wrap_dllEntry(int ( *syscallptr) (int arg,... ));
int (*orig_syscall) (int command, ...); // 0x8084f50 and never change
int wrap_syscall(int command, ...);
int (*orig_CG_vmMain) (int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11);
int wrap_CG_vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11);
void *(*orig_CG_EntityEvent) (centity_t *cent, vec3_t position);
int wrap_CG_EntityEvent (centity_t *cent, vec3_t position);
void (*orig_CG_FinishWeaponChange) (int lastweap, int newweap);
int wrap_CG_FinishWeaponChange (int lastweap, int newweap);
void (*orig_CG_Trace) (trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask);
void (*orig_BG_EvaluateTrajectory) (const trajectory_t *tr, int atTime, vec3_t result, qboolean isAngle, int splinePath);
// Game hooked functions from "ui.mp.i386.so"
// not yet :) there is some memory access problem (not thread safe ?)
void ethSegFault();
/*
==============================
from tools.c
==============================
*/
void doAimBot(vec3_t target);
void doAutoShoot(void);
int findNextTarget(int targetFlag);
int CG_WorldToScreen(vec3_t worldCoord, int *x, int *y);
qboolean isCurrentPlayer();

// target filters
int isPlantedDynamite(int entityNum);
int isDynamite(int entityNum);
int isDynamiteToArm(int entityNum);
int isLandmine(int entityNum);
int isLandminePlanted(int entityNum);
int isMissile(int entityNum);
int isGrenade(int entityNum);
int isSatchel(int entityNum);
int isMortar(const int entityNum);
int isPanzer(const int entityNum);

// For game console actions
void initActions();
void setAction(int action, int state);
void forceAction(int action, int state);
void resetAllActions();

// target filter
#define TARGET_DEAD			1
#define TARGET_ALIVE		2
#define TARGET_ENEMY		4
#define TARGET_FRIEND		8
#define TARGET_VISIBLE		16
#define TARGET_NOTVISIBLE	32
#define TARGET_DYNA_PLANTED	64
#define TARGET_DYNA_TOARM	128
#define TARGET_PLAYER		256

/*
==============================
from visuals.c
==============================
*/
void addChams(refEntity_t* refEnt, ethEntity_t* entity);
void drawFillRect(int x, int y, int width, int height, vec4_t color, qboolean border, int borderSize, vec4_t borderColor);
void drawAdvert();
void drawEspEntities();
void drawRadar();
void drawSpawnTimerMiddle();
void drawSpawnTimerRight();
void drawStats();
void drawMenu();
void playSpreeSound();
void registerEthMedias();
void setEspColors(ethEntity_t* entity);

#endif /* ETH_H_*/
