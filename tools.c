// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2005 *nixCoders team - don't forget to credits us
// $Id$

#include "eth.h"

void doAimBot(vec3_t target) {
	float *y = (float*)0x906c1c8;
	float *x = (float*)0x906c1cc;

    vec3_t org, ang;
    VectorSubtract(target, cg.refdef.vieworg, org);
    vectoangles(org, ang);

	ang[YAW] = AngleNormalize180(ang[YAW]);
	ang[PITCH] = AngleNormalize180(ang[PITCH]);
	
    AnglesToAxis(ang, cg.refdef.viewaxis);

    ang[YAW] -= cg.refdefViewAngles[YAW];
	ang[YAW] = AngleNormalize180(ang[YAW]);

    ang[PITCH] -= cg.refdefViewAngles[PITCH];
	ang[PITCH] = AngleNormalize180(ang[PITCH]);

    *x += ang[YAW];
    *y += ang[PITCH];
}

// Return a entity number. -1 if no entity to aim.
int findNextTarget(int targetFlag) {
	int nearest = -1;

	int entityNum = 0;
	for (; entityNum<MAX_CLIENTS; entityNum++) {
		ethEntity_t* entity = &eth.entities[entityNum];
		int targetFilter = 1;
		if (targetFlag & TARGET_PLAYER)
			targetFilter = targetFilter && entity->isValidPlayer;
		if (targetFlag & TARGET_DEAD)
			targetFilter = targetFilter && entity->isDead;
		if (targetFlag & TARGET_ALIVE)
			targetFilter = targetFilter && !entity->isDead;
		if (targetFlag & TARGET_ENEMY)
			targetFilter = targetFilter && entity->isEnemy;
		if (targetFlag & TARGET_FRIEND)
			targetFilter = targetFilter && !entity->isEnemy;
		if (targetFlag & TARGET_VISIBLE)
			targetFilter = targetFilter && entity->isVisible;
		if (targetFlag & TARGET_NOTVISIBLE)
			targetFilter = targetFilter && !entity->isVisible;
		
		// Don't aim at invulnerable player
		if (entity->isValidPlayer && (cg_entities[entityNum].currentState.powerups & (1<<PW_INVULNERABLE)))
			targetFilter = 0;

		if (targetFilter && (entity->distance != 0)
				&& (nearest == -1 || (entity->distance < eth.entities[nearest].distance))) {
			nearest = entityNum;
		}
	}
	return nearest;
}

void doAutoShoot(void) {
	if (cv_autoshoot.integer) {
		if (wrap_trap_Key_IsDown(K_MOUSE1))
			setAction(ACTION_ATTACK, 1);
		else
			setAction(ACTION_ATTACK, 0);
	}
}

int CG_WorldToScreen(vec3_t worldCoord, int *x, int *y) {
	vec3_t local, transformed;
	vec3_t vfwd;
	vec3_t vright;
	vec3_t vup;
	float xzi;
	float yzi;
	
	AngleVectors (cg.refdefViewAngles, vfwd, vright, vup);
	
	VectorSubtract (worldCoord, cg.refdef.vieworg, local);
	
	transformed[0] = DotProduct (local, vright);
	transformed[1] = DotProduct (local, vup);
	transformed[2] = DotProduct (local, vfwd);
	
	// Make sure Z is not negative.
	if (transformed[2] < 0.01)
		return 0;
	
	// For correction when in scoped weapon
	static float coef = 0;
	if (!coef)
		coef = cg.refdef.fov_x / cg.refdef.fov_y;
	int weapon = cg_entities[cg.snap->ps.clientNum].currentState.weapon;
	if (weapon == WP_K43_SCOPE || weapon == WP_GARAND_SCOPE || weapon == WP_FG42SCOPE) {
		cg.refdef.fov_x = cg_fov.value;
		cg.refdef.fov_y = cg_fov.value / coef;
	}

	xzi = 320 / transformed[2] * (90.0 / cg.refdef.fov_x);
	yzi = 240 / transformed[2] * (90.0 / cg.refdef.fov_y);

	*x = 320 + xzi * transformed[0];
	*y = 240 - yzi * transformed[1];

	return 1;
}

void ethSegFault() {
	orig_syscall(CG_ERROR, "eth: A fatal error has occured. You must restart the game. (segfault)\n\neth v" ETH_VERSION " *nixCoders");
}

/*
==============================
 Console actions
==============================
*/

void initActions() {
	eth.actions[ACTION_ATTACK]		= (ethAction_t){0, 0, "+attack\n",				"-attack\n"};
	eth.actions[ACTION_BACKWARD]	= (ethAction_t){0, 0, "+backward\n",			"-backward\n"};
	eth.actions[ACTION_BINDMOUSE1]	= (ethAction_t){1, 1, "bind mouse1 +attack\n",	"unbind mouse1\n"};
	eth.actions[ACTION_CROUCH]		= (ethAction_t){0, 0, "+movedown\n",				"-movedown\n"};
	eth.actions[ACTION_JUMP]		= (ethAction_t){0, 0, "+moveup\n",				"-moveup\n"};
	eth.actions[ACTION_PRONE]		= (ethAction_t){0, 0, "+prone\n",				"-prone\n"};
	eth.actions[ACTION_RUN]			= (ethAction_t){0, 0, "+sprint; +forward\n",	"-sprint; -forward\n"};
}

// Set an action if not already set.
void setAction(int action, int state) {
	if (state && !eth.actions[action].state)
		forceAction(action, state);
	else if (!state && eth.actions[action].state)
		forceAction(action, state);
}
// Use this with caution
void forceAction(int action, int state) {
	if (state) {
		eth.actions[action].state = 1;
		wrap_trap_CG_SendConsoleCommand(eth.actions[action].startAction);
	} else {
		eth.actions[action].state = 0;
		wrap_trap_CG_SendConsoleCommand(eth.actions[action].stopAction);
	}
}

void resetAllActions() {
	int action;
	for (action=0; action<ACTION_MAX; action++)
		forceAction(action, eth.actions[action].defaultState);
}

/*
==============================
filters
==============================
*/

// Name of the real current player not a spec one
qboolean isCurrentPlayer() {
	return strcmp(cgs.clientinfo[cg.snap->ps.clientNum].name, eth.playerName) == 0;
}

// Grenade, dynamite, landmine, satchel, ... are missile
int isMissile(const int entityNum) {
	return cg_entities[entityNum].currentState.eType == ET_MISSILE;
}

int isDynamite(const int entityNum) {
	return isMissile(entityNum)
		&& (cg_entities[entityNum].currentState.weapon == WP_DYNAMITE);
}

int isPlantedDynamite(const int entityNum) {
	return isDynamite(entityNum)
		&& (cg_entities[entityNum].currentState.teamNum < 4);
}

int isDynamiteToArm(const int entityNum) {
	return isDynamite(entityNum)
		&& (cg_entities[entityNum].currentState.teamNum > 4);
}

int isLandmine(const int entityNum) {
	return isMissile(entityNum)
		&& (cg_entities[entityNum].currentState.weapon == WP_LANDMINE);
}

int isLandminePlanted(const int entityNum) {
	return isLandmine(entityNum)
		&& (cg_entities[entityNum].currentState.teamNum < 4);
}

int isGrenade(const int entityNum) {
	return isMissile(entityNum)
		&& ((cg_entities[entityNum].currentState.weapon == WP_GRENADE_LAUNCHER)   // axis grenade
		|| (cg_entities[entityNum].currentState.weapon == WP_GRENADE_PINEAPPLE)); // allies grenade
}

int isSatchel(const int entityNum) {
	return isMissile(entityNum)
		&& (cg_entities[entityNum].currentState.weapon == WP_SATCHEL);
}

int isMortar(const int entityNum) {
	return isMissile(entityNum)
		&& (cg_entities[entityNum].currentState.weapon == WP_MORTAR_SET);
}

int isPanzer(const int entityNum) {
	return isMissile(entityNum)
		&& (cg_entities[entityNum].currentState.weapon == WP_PANZERFAUST);
}
