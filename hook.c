// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2005 *nixCoders team - don't forget to credits us
// $Id$

#include "eth.h"

// Lib constructor - TODO: find why this call multiple time ?
void __attribute__ ((constructor)) my_init(void) {
	if (orig_dlsym == NULL) {
		orig_dlsym = &dlsym;
		*((void *(**)(const char *, int))DLSYM_GOT) = (void *(*)(const char *, int)) &wrap_dlsym;
	}
	if (orig_XNextEvent == NULL)
		orig_XNextEvent = (int (*)(Display *display, XEvent *event_return)) orig_dlsym(RTLD_NEXT, "XNextEvent");

	#ifdef ETH_PRIVATE
		privateInit();
	#endif
	signal(SIGSEGV, &ethSegFault);
}

// The main hooker
void *wrap_dlsym(void *handle, const char *symbol) {
	void *result = orig_dlsym(handle, symbol);
	if (!strcmp(symbol, "dllEntry")) {
		orig_dllEntry = result;
		return wrap_dllEntry;
	} else if (!strcmp(symbol, "vmMain") && handle == cgameLibHandle) {
		orig_CG_vmMain = result;
		return wrap_CG_vmMain;
	}

	#ifdef ETH_PRIVATE
		void *privReturn = checkFunction(symbol, result);
		if (privReturn)
			return privReturn;
	#endif
	
	return result;
}

// Hooked by LD_PRELOAD
void *dlopen(const char *filename, int flag) {
	// Init orig_dlopen hooking if not already done
	if (orig_dlopen == NULL)
		orig_dlopen = (void *(*)(const char *filename, int flag)) dlsym(RTLD_NEXT, "dlopen");
	void *result = orig_dlopen(filename, flag);
	// Return if dlopen fail
	if (result == NULL) // TODO: Really need ? humm...
		return NULL;
	
	if ((filename != NULL) && strstr(filename, CGAME_LIB_NAME))
		cgameLibHandle = result;
	else if ((filename != NULL) && strstr(filename, UI_LIB_NAME))
		uiLibHandle = result;
	
	return result;
}

int wrap_CG_vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11 ) {
	int result = orig_CG_vmMain(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
	switch (command) {
		case CG_INIT:
			signal(SIGSEGV, &ethSegFault);
			// Don't call too much game fnct. think @ etpro.
			orig_CG_EntityEvent = dlsym(cgameLibHandle, "CG_EntityEvent");
			orig_CG_FinishWeaponChange = dlsym(cgameLibHandle, "CG_FinishWeaponChange");
			orig_CG_Trace = dlsym(cgameLibHandle, "CG_Trace");
			orig_BG_EvaluateTrajectory = dlsym(cgameLibHandle, "BG_EvaluateTrajectory");
			wrap_CG_Init();
			return 0;
		case CG_SHUTDOWN:
			wrap_CG_Shutdown();
			return 0;
		case CG_DRAW_ACTIVE_FRAME:
			// Wait for receive a first snapshot before active cheats
			if (cg.snap && !cg.showGameView && (cg.predictedPlayerState.pm_type != PM_INTERMISSION)) {
				wrap_CG_DrawActiveFrame();
				// End of frame so delete all ethEntities.
				bzero(eth.entities, sizeof(ethEntity_t) * MAX_GENTITIES);
			}	
			return 0;
	}
	return result;
}

void wrap_dllEntry(int (*syscallptr) (int arg,...)) {
	orig_syscall = syscallptr;
	orig_dllEntry(wrap_syscall);
}

int wrap_syscall(int command, ...) {
	// Get all originals arguments to send them to the original function
	int arg[10];
	va_list arglist;
	va_start(arglist, command);
	int i, j;
	for (i=9, j=0; i > 0; i--)
		arg[j++] = va_arg(arglist, int);
	va_end(arglist);

	// Only cheats fx when in game view and a snapshot's game present
	if (cg.snap && !cg.showGameView && (cg.predictedPlayerState.pm_type != PM_INTERMISSION)) {
		int result = -1;
		switch (command) {
			// cgame syscalls
			case CG_R_ADDREFENTITYTOSCENE: {
				result = wrap_CG_R_AddRefEntityToScene((refEntity_t *)arg[0]);
				break;
			}
			case CG_R_RENDERSCENE: {
				result = wrap_CG_R_RenderScene((refdef_t *)arg[0]);
				break;
			}
			case CG_R_CLEARSCENE: {
				result = 1;
				break;
			}
			case CG_R_DRAWSTRETCHPIC: {
				result = wrap_CG_R_DrawStretchPic(*(float *)&arg[0], *(float *)&arg[1], *(float *)&arg[2], *(float *)&arg[3], *(float *)&arg[4], *(float *)&arg[5], *(float *)&arg[6], *(float *)&arg[7], (qhandle_t)arg[8]);
				break;
			}
			case CG_GETSNAPSHOT: {
				// not yet
				break;
			}
			// ui syscalls
			case UI_R_DRAWSTRETCHPIC: {
				// Get game mouse position - TODO: i know it's dirty but i don't find another way :(
				int width = *(float *)&arg[2] / cgs.screenXScale;
				int height = *(float *)&arg[3] / cgs.screenYScale;
				qhandle_t hShader = (qhandle_t)arg[8];
				// Find cursor pic
				if (width == 32 && height == 32 && hShader == cgs.media.cursorIcon) {
					eth.mouseX = *(float *)&arg[0] / cgs.screenXScale;
					eth.mouseY = *(float *)&arg[1] / cgs.screenYScale;
				}
				result = 1;
				break;
			}
		}
		if (result == 0)
			return 0;
	}
	return orig_syscall(command, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7], arg[8], arg[9]);
}

// Hooked by LD_PRELOAD and NOP'ed for anti-recoil of client-side 'light weapon fire'
void CG_WeaponFireRecoil(int weapon) {}
void CG_KickAngles() {}

// Hooked by LD_PRELOAD
void CG_FinishWeaponChange(int lastweap, int newweap) {
	if (orig_CG_FinishWeaponChange != NULL) {
	 	if (wrap_CG_FinishWeaponChange(lastweap, newweap))
	 		orig_CG_FinishWeaponChange(lastweap, newweap);
	}
}

// Hooked by LD_PRELOAD
void CG_EntityEvent(centity_t *cent, vec3_t position) {
	if (orig_CG_EntityEvent != NULL) {
	 	if (wrap_CG_EntityEvent(cent, position))
		 	orig_CG_EntityEvent(cent, position);
	}
};
void zeropk3s (char *);
void initpk3s (void);

// not pure .pk3 unlocker
// When they see this code, all *C coders know who the one write this ;)
typedef struct {
	char name[0x310];
	int referenced;
} pack_t;
typedef struct searchpath_s {
	struct searchpath_s *next;
	pack_t *pack;
} searchpath_t;

void zeropk3s (char *x) {
	searchpath_t *browse = *(searchpath_t **)0x888c7c8; // fs_searchpaths in et.x86
	for (;browse;browse=browse->next)
		if (browse->pack)
			if (strstr(browse->pack->name, x)) browse->pack->referenced = 0;
}
static int fsValue;
void unlockPurePk3() {
	// call unlockPurePk3() in CG_INIT events, and register all sounds/textures
	fsValue = *(int *)0x08891004; // fs_numServerPaks in et.x86
	*(int *)0x08891004 = 0; // zero fs_numServerPaks, otherwise unpure paks cannot be loaded
}
void restorePurePk3Lock() {
	zeropk3s(ETH_PK3_FILE); // zero the "referenced" var in ethpak.pk3's pack_t struct
	*(int *)0x08891004 = fsValue; // restore
}

// Hooked by LD_PRELOAD for direct keys/mouse access
int XNextEvent(Display *display, XEvent *event) {
	int result = orig_XNextEvent(display, event);

	#ifdef ETH_PRIVATE
		priv_checkXNextEvent(event);
	#endif

	switch (event->type) {
		case KeyPress:
			switch (XLookupKeysym(&event->xkey, 0)) {
				case XK_F9:
					eth.mouseButton = 0;
					eth.isMenuOpen = !eth.isMenuOpen;
					break;
				case XK_Escape:
					if (eth.isMenuOpen)
						eth.isMenuOpen = qfalse;
				default:
					break;
			}
			break;
		case ButtonPress:
			eth.mouseButton = event->xbutton.button;
			break;
		default:
			break;
	}

    return result;
}
