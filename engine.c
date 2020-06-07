// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2005 *nixCoders team - don't forget to credits us
// $Id$

#include "eth.h"

static cvarTable_t cvarTable[] = {
	#ifdef ETH_PRIVATE
		PRIVATE_CVARS
	#endif
	// aimbot
	{ &cv_aim,			"aim",			"1", CVAR_ARCHIVE },
	{ &cv_aimvecz,		"aimvecz",		"0", CVAR_ARCHIVE },
	{ &cv_aimdead,		"aimdead",		"0", CVAR_ARCHIVE },
	// visual
	{ &cv_advert,		"advert",		"1", CVAR_ARCHIVE },
	{ &cv_chams,			"chams",			"1", CVAR_ARCHIVE },
	{ &cv_glow,			"glow",			"1", CVAR_ARCHIVE },
	{ &cv_radar,		"radar",		"1", CVAR_ARCHIVE },
	{ &cv_spawntimer,	"spawntimer",	"1", CVAR_ARCHIVE },
	{ &cv_stats,		"stats",		"1", CVAR_ARCHIVE },
	{ &cv_wallhack,		"wallhack",		"1", CVAR_ARCHIVE },
	// esp
	{ &cv_espfade,		"espfade",		"75",	CVAR_ARCHIVE },
	{ &cv_espname,		"espname",		"1",	CVAR_ARCHIVE },
	{ &cv_espname_bg,	"espname_bg",	"1",	CVAR_ARCHIVE },
	{ &cv_espicon,		"espicon",		"1",	CVAR_ARCHIVE },
	{ &cv_espicon_bg,	"espicon_bg",	"1",	CVAR_ARCHIVE },
	{ &cv_espdist,		"espdist",		"2000",	CVAR_ARCHIVE },
	// sound
	{ &cv_sndmisc,		"sndmisc",		"1",	CVAR_ARCHIVE },
	{ &cv_sndspree,		"sndspree",		"1",	CVAR_ARCHIVE },
	{ &cv_spreetime,		"spreetime",		"5",	CVAR_ARCHIVE },
	// misc
	{ &cv_autoshoot,	"autoshoot",	"0", CVAR_ARCHIVE },
	{ &cv_bunnyjump,		"bunnyjump",		"0", CVAR_ARCHIVE },
	{ &cv_sniperzoom,	"sniperzoom",	"0", CVAR_ARCHIVE }
};
static int cvarTableSize = sizeof(cvarTable) / sizeof(cvarTable[0]);

int wrap_CG_R_AddRefEntityToScene(refEntity_t *refEnt) {
	ethEntity_t* entity = &eth.entities[refEnt->entityNum];

	entity->isValidPlayer = (refEnt->entityNum < MAX_CLIENTS)
		&& (cg_entities[refEnt->entityNum].currentState.eType == ET_PLAYER)
		&& cgs.clientinfo[refEnt->entityNum].infoValid
		&& cg_entities[refEnt->entityNum].currentValid
		&& (cg.snap->ps.clientNum != refEnt->entityNum);

	// Get player infos
	if (entity->isValidPlayer) {
		entity->isEnemy = (cgs.clientinfo[refEnt->entityNum].team != cgs.clientinfo[cg.snap->ps.clientNum].team);
		entity->isDead = (cg_entities[refEnt->entityNum].currentState.eFlags & EF_DEAD);
		// aim player part
		orientation_t tagTarget;
		char *tag;
		switch(cv_aim.integer) {
			default:
			case 1: tag = "tag_head"; break;
			case 2: tag = "tag_mouth"; break;
			case 3: tag = "tag_torso"; break;
			case 4: tag = "tag_chest"; break;
		}
		CG_GetTag(refEnt->entityNum, tag, &tagTarget);
		VectorCopy(tagTarget.origin, entity->target);
		// Prediction
		//VectorMA (entity->target, cg.snap->ping / 1000, cg_entities[refEnt->entityNum].currentState.pos.trDelta, entity->target);
		#ifdef ETH_PRIVATE
			PRIVATE_CORRECTION
		#endif
		// aimvecz correction
		entity->target[ROLL] += (float)cv_aimvecz.integer;
		// distance
		entity->distance = VectorDistance(cg_entities[cg.snap->ps.clientNum].lerpOrigin, entity->target);
		// isVisible
		trace_t trace;
		orig_CG_Trace(&trace, cg.refdef.vieworg, NULL, NULL, entity->target, cg.snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_CORPSE);
		entity->isVisible = (trace.fraction == 1.0f);
	}

	// Get missile infos
	if (isMissile(refEnt->entityNum)) {
		entity->distance = VectorDistance(cg_entities[cg.snap->ps.clientNum].lerpOrigin, cg_entities[refEnt->entityNum].lerpOrigin);
		// isEnemy
		if (cg_entities[refEnt->entityNum].currentState.teamNum > 4)
			entity->isEnemy = ((cgs.clientinfo[cg.snap->ps.clientNum].team + 4) != cg_entities[refEnt->entityNum].currentState.teamNum);
		else
			entity->isEnemy = (cgs.clientinfo[cg.snap->ps.clientNum].team != cg_entities[refEnt->entityNum].currentState.teamNum);
	}

	if ((entity->isValidPlayer || isMissile(refEnt->entityNum))) {
		// Visual
		entity->isInScreen = CG_WorldToScreen(cg_entities[refEnt->entityNum].lerpOrigin, &entity->screenX, &entity->screenY);
		if (entity->isValidPlayer && entity->isInScreen) {
//			if (cgs.clientinfo[refEnt->entityNum].team && refEnt->frame
//					&& refEnt->reFlags != ET_CORPSE	&& refEnt->reFlags != CONTENTS_WATER) {
			if (refEnt->frame && refEnt->torsoFrameModel && (refEnt->frame && (cg_entities[refEnt->entityNum].currentState.eType & ET_PLAYER))) {
				// wallhack
				if (!(refEnt->renderfx & RF_DEPTHHACK) && cv_wallhack.integer)
					refEnt->renderfx |= RF_DEPTHHACK | RF_NOSHADOW;
				// chams
				if (cv_chams.integer != 0)
					addChams(refEnt, entity);
			}
		}
		if (isMissile(refEnt->entityNum)) {
			// wallhack
			if (!(refEnt->renderfx & RF_DEPTHHACK) && cv_wallhack.integer)
				refEnt->renderfx |= RF_DEPTHHACK | RF_NOSHADOW;
		}
		// espColor
		setEspColors(entity);
		// espFade
		entity->espColor[3] = (float)cv_espfade.integer / 100.0f;
	}
	return 1;
}

void wrap_CG_DrawActiveFrame(void) {
	updateCvars();

	#ifdef ETH_PRIVATE
		priv_CG_DrawFrame();
	#endif

	// Update current name
	wrap_trap_Cvar_VariableStringBuffer("name", eth.playerName, sizeof(eth.playerName));

	// Bunny jump
	if (cv_bunnyjump.integer) {
		// Find the current key for jump.
		int key1, key2;
		orig_syscall(CG_KEY_BINDINGTOKEYS, "+moveup", &key1, &key2);
		// If jump keys press
		if (wrap_trap_Key_IsDown(key1) || wrap_trap_Key_IsDown(key2)) {
			// If player fall
			if (cg.snap->ps.velocity[ROLL] <= 00.0f)
				setAction(ACTION_JUMP, 1);
			else
				setAction(ACTION_JUMP, 0);
		} else {
			setAction(ACTION_JUMP, 0);
		}
	}
	
	drawEspEntities();
	if (cv_stats.integer)
		drawStats();
	// Spawntimer
	if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_ALLIES
			|| cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS) {
		if (cv_spawntimer.integer == 1) 
			drawSpawnTimerMiddle();
		else if (cv_spawntimer.integer == 2)
			drawSpawnTimerRight();
	}
	if (cv_radar.integer)
		drawRadar();
	if (cv_advert.integer) 
		drawAdvert();
	if (eth.isMenuOpen) {
		drawMenu();
		// Get mouse/key events
		orig_syscall(CG_KEY_SETCATCHER, KEYCATCH_UI);
	}

	if (cv_autoshoot.integer)
		setAction(ACTION_BINDMOUSE1, 0);
	else
		setAction(ACTION_BINDMOUSE1, 1);

	// Do differents actions with weapons
	int entityNum = -1;
	int targetFlag;
	switch (cg_entities[cg.snap->ps.clientNum].currentState.weapon) {
		case WP_LUGER:
		case WP_SILENCER:
		case WP_AKIMBO_LUGER:
		case WP_AKIMBO_SILENCEDLUGER:
		case WP_COLT:
		case WP_SILENCED_COLT:
		case WP_AKIMBO_COLT:
		case WP_AKIMBO_SILENCEDCOLT:
		case WP_MP40:
		case WP_THOMPSON:
		case WP_MOBILE_MG42:
		case WP_GARAND:
		case WP_K43:
		case WP_FG42:
		case WP_STEN:
			if (wrap_trap_Key_IsDown(K_MOUSE1)) {
				targetFlag = TARGET_PLAYER | TARGET_ENEMY | TARGET_VISIBLE;
				if (!cv_aimdead.integer)
					targetFlag |= TARGET_ALIVE;
				entityNum = findNextTarget(targetFlag);
				if ((entityNum != -1) && cv_aim.integer)
					doAimBot(eth.entities[entityNum].target);
			}
			if (entityNum != -1)
				doAutoShoot();
			else
				setAction(ACTION_ATTACK, 0);
			break;
		case WP_KNIFE:
			entityNum = findNextTarget(TARGET_PLAYER | TARGET_ALIVE | TARGET_ENEMY | TARGET_VISIBLE);
			if (entityNum != -1 && wrap_trap_Key_IsDown(K_MOUSE1))
				doAimBot(cg_entities[entityNum].pe.headRefEnt.origin);
			doAutoShoot();
			break;
		case WP_GARAND_SCOPE:
		case WP_K43_SCOPE: {
			static int lastShoot = 0;
			entityNum = findNextTarget(TARGET_PLAYER | TARGET_ENEMY | TARGET_VISIBLE | TARGET_ALIVE);
			if ((entityNum != -1) && cv_aim.integer)
				doAimBot(eth.entities[entityNum].target);
				
			if (lastShoot == 0) // init
				lastShoot = cg.time + 700;
			if ((entityNum != -1) && ((cg.time - lastShoot) >= 700)) {
				doAutoShoot();
				lastShoot = cg.time;
			} else {
				setAction(ACTION_ATTACK, 0);
			}
			break;
		}	
		case WP_FG42SCOPE:
			entityNum = findNextTarget(TARGET_PLAYER | TARGET_ENEMY | TARGET_VISIBLE | TARGET_ALIVE);
			if ((entityNum != -1) && cv_aim.integer)
				doAimBot(eth.entities[entityNum].target);
			if (entityNum != -1)
				doAutoShoot();
			else
				setAction(ACTION_ATTACK, 0);
			break;
		default:	// All others weapons
			doAutoShoot();
	}
}

int wrap_CG_EntityEvent (centity_t *cent, vec3_t position) {
	entityState_t *es;
	int event;
	int clientNum;
	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;
	clientNum = es->clientNum;
	// Filter for speedup ??
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	switch (event) {
		case EV_OBITUARY: {
			// Update stats
			// if it's the eth player
			if (!isCurrentPlayer()) {
				break;
			// If dead
			} else if (es->otherEntityNum == cg.snap->ps.clientNum) {
				if (es->otherEntityNum2 != cg.snap->ps.clientNum)
					eth.deathCount++;
				eth.killSpreeCount = 0;
				eth.lastKillTime = 0;
				eth.firstKillSpreeTime = 0;
			// If kill
			} else if ((es->otherEntityNum2 == cg.snap->ps.clientNum) && (es->otherEntityNum != cg.snap->ps.clientNum)) {
				eth.killCount++;
				// If not a killing spree
				if ((cg.time - eth.lastKillTime) > (cv_spreetime.integer * 1000)) {
					eth.firstKillSpreeTime = cg.time;
					eth.killSpreeCount = 1;
				} else {
					eth.killSpreeCount++;
				}
				if (cv_sndspree.integer)
					playSpreeSound();
				if (cv_sndmisc.integer && (es->eventParm == MOD_KNIFE))
					wrap_trap_S_StartLocalSound(eth.sounds[SOUND_HUMILIATION], CHAN_LOCAL_SOUND);

				eth.lastKillTime = cg.time;
			}
			break;
		}
		default:
			break;
	}
	return 1;
};

int wrap_CG_R_RenderScene(refdef_t *refDef) {
	// Check if the scene is mainview
	if ((refDef->x == 1) && (refDef->y == 1)) {
		// Remove sniper zoom.
		static float coef = 0;
		if (!coef)
			coef = refDef->fov_x / refDef->fov_y;
		if (cv_sniperzoom.integer) {
			refDef->fov_x = cg_fov.value;
			refDef->fov_y = cg_fov.value / coef; // TODO: don't use a coef => use game funtion for get fov_y
			cg.zoomSensitivity = 1.0f;
		}

		// Call original function
		orig_syscall(CG_R_RENDERSCENE, refDef);
		return 0;
	}
	return 1;
}

// Return 0 if this pic don't have to be draw
int wrap_CG_R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader) {

	// Reverse wrap_CG_AdjustFrom640()
 	x /= cgs.screenXScale;
 	y /= cgs.screenYScale;
 	w /= cgs.screenXScale;
 	h /= cgs.screenYScale;
 
 	// Remove sniper pic
	if (hShader == cgs.media.reticleShaderSimple || hShader == cgs.media.binocShaderSimple)
		return 0;
	// Remove sniper black area
	if (y == 0 && w == 80 && (hShader == cgs.media.whiteShader))
		return 0;

	return 1;
}

void wrap_CG_Init(void) {
	srand(time(NULL));
	orig_syscall(CG_PRINT, "LOADING... eth v" ETH_VERSION "\n");
	registerCvars();

	// Init stats
	eth.lastKillTime = 0;
	eth.firstKillSpreeTime = 0;

	// Init eth medias
	unlockPurePk3();
	registerEthMedias();
	restorePurePk3Lock();
	wrap_trap_S_StartLocalSound(eth.sounds[SOUND_PREPARE], CHAN_AUTO);

	initActions();
	resetAllActions();

	#ifdef ETH_PRIVATE
		priv_CG_init();
	#endif
}

void wrap_CG_Shutdown(void) {
	setAction(ACTION_BINDMOUSE1, 1);
}

int wrap_CG_FinishWeaponChange(int lastweap, int newweap) {
	// Sniper hack
	if (!((lastweap == WP_K43_SCOPE && newweap == WP_K43)
			|| (lastweap == WP_GARAND_SCOPE && newweap == WP_GARAND)
			|| (lastweap == WP_FG42SCOPE && newweap == WP_FG42)))
		return 1;

	int weapon = lastweap;
	if (weapon == WP_K43_SCOPE)
		weapon = WP_K43;
	if (weapon == WP_GARAND_SCOPE)
		weapon = WP_GARAND;
	if (weapon == WP_FG42SCOPE)
		weapon = WP_FG42;
	if (cg.snap->ps.ammoclip[weapon] == 0)
		return 1;

	// Find the key for reload;
	int key1, key2;
	orig_syscall(CG_KEY_BINDINGTOKEYS, "+reload", &key1, &key2);
	// If reload keys press
	if (wrap_trap_Key_IsDown(key1) || wrap_trap_Key_IsDown(key2))
		return 1;

	return 0;
}

void registerCvars() {
	int i;
	cvarTable_t *cv;
	for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
		// Add cvar prefix before register
		char cvarName[50];
		sprintf(cvarName, "%s%s", ETH_CVAR_PREFIX, cv->cvarName);
		wrap_trap_Cvar_Register(cv->vmCvar, cvarName, cv->defaultString, cv->cvarFlags);
	}
}

void updateCvars() {
	int i;
	cvarTable_t *cv;
	for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++)
		wrap_trap_Cvar_Update(cv->vmCvar);
}
