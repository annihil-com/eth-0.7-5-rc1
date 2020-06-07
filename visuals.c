// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2005 *nixCoders team - don't forget to credits us
// $Id$

#include "eth.h"

void addChams(refEntity_t* refEnt, ethEntity_t* entity) {
	
	if (cv_glow.integer == 1) {
		refEntity_t original = *refEnt;
		wrap_trap_R_AddRefEntityToScene(&original);
	}
	
	switch (cv_chams.integer) {
		case 1:
			if (entity->isEnemy) {
				if (entity->isVisible) {
					if (cv_glow.integer)
						refEnt->customShader = eth.redGlowShader;
					else
						refEnt->customShader = eth.redShader;
				} else {
					if (cv_glow.integer)
						refEnt->customShader = eth.tealGlowShader;
					else
						refEnt->customShader = eth.tealShader;
				}
			} else {
				if (entity->isVisible) {
					if (cv_glow.integer)
						refEnt->customShader = eth.blueGlowShader;
					else
						refEnt->customShader = eth.blueShader;
				} else {
					if (cv_glow.integer)
						refEnt->customShader = eth.cyanGlowShader;
					else
						refEnt->customShader = eth.cyanShader;
				}
			}
			break;
		case 2:
			if (entity->isEnemy) {
				if (entity->isVisible) {
					if (cv_glow.integer)
						refEnt->customShader = eth.tealGlowShader;
					else
						refEnt->customShader = eth.tealShader;
				} else {
					if (cv_glow.integer)
						refEnt->customShader = eth.cyanGlowShader;
					else
						refEnt->customShader = eth.cyanShader;
				}
			} else {
				if (entity->isVisible) {
					if (cv_glow.integer)
						refEnt->customShader = eth.greenGlowShader;
					else
						refEnt->customShader = eth.greenShader;
				} else {
					if (cv_glow.integer)
						refEnt->customShader = eth.yellowGlowShader;
					else
						refEnt->customShader = eth.yellowShader;
				}
			}
			break;
		case 3:
			if (entity->isEnemy) {
				if (entity->isVisible) {
					if (cv_glow.integer)
						refEnt->customShader = eth.redGlowShader;
					else
						refEnt->customShader = eth.redShader;
				} else {
					if (cv_glow.integer)
						refEnt->customShader = eth.tealGlowShader;
					else
						refEnt->customShader = eth.tealShader;
				}
			}
			break;
	}
}

void setEspColors(ethEntity_t* entity) {
	switch (cv_chams.integer) {
		case 1:
			if (entity->isEnemy) {
				if (entity->isVisible)
					VectorCopy(colorRed, entity->espColor);
				else
					VectorCopy(colorMagenta, entity->espColor);
			} else {
				if (entity->isVisible)
					VectorCopy(colorBlue, entity->espColor);
				else
					VectorCopy(colorCyan, entity->espColor);
			}
			break;
		case 2:
			if (entity->isEnemy) {
				if (entity->isVisible)
					VectorCopy(colorMagenta, entity->espColor);
				else
					VectorCopy(colorCyan, entity->espColor);
			} else {
				if (entity->isVisible)
					VectorCopy(colorGreen, entity->espColor);
				else
					VectorCopy(colorYellow, entity->espColor);
			}
			break;
		case 3:
			if (entity->isEnemy) {
				if (entity->isVisible)
					VectorCopy(colorRed, entity->espColor);
				else
					VectorCopy(colorMagenta, entity->espColor);
			} else {
				if (entity->isVisible)
					VectorCopy(colorLtGrey, entity->espColor);
				else
					VectorCopy(colorWhite, entity->espColor);
			}
			break;
		default:
			VectorCopy(colorWhite, entity->espColor);
			break;
	}
}

void drawEspEntities() {
	int entityNum;
	
	// Player esp
	for (entityNum=0; entityNum<MAX_CLIENTS; entityNum++) {
		ethEntity_t* entity = &eth.entities[entityNum];
		
		if ((cv_espdist.integer != 0) && (entity->distance > cv_espdist.integer))
			continue;
		if (!entity->isInScreen || !entity->isValidPlayer)
			continue;

		vec4_t bgColor;
		VectorCopy(colorBlack, bgColor);
		bgColor[3] = entity->espColor[3];
		// Name esp
		if (cv_espname.integer) {
			char *str = cgs.clientinfo[entityNum].name;
			if (cv_espname_bg.integer)
			    wrap_CG_FillRect((entity->screenX - ((Q_PrintStrlen(str) * 8) >> 1) - 2), entity->screenY-39, (Q_PrintStrlen(str) * 8)+4 ,12, bgColor);
			wrap_CG_DrawStringExt((entity->screenX - ((Q_PrintStrlen(str) * 8) >> 1)), entity->screenY-40, str, entity->espColor, (cv_espname.integer == 1), qtrue, 8, 12, 50);
		}
		// Weapon, rank and class esp
		if (cv_espicon.integer) {
			if(cv_espicon_bg.integer)
			    wrap_CG_FillRect( entity->screenX-(55/2) - 1 , entity->screenY-26, 57 ,17, bgColor);
		    wrap_trap_R_SetColor(entity->espColor);
			wrap_CG_DrawPic( entity->screenX-(55/2), entity->screenY-25, 15, 15, eth.classIcons[cgs.clientinfo[entityNum].cls]);
			wrap_CG_DrawPic( entity->screenX-(55/2)+15, entity->screenY-25, 15, 15, eth.rankIcons[cgs.clientinfo[entityNum].rank]);
			wrap_CG_DrawPic( entity->screenX-(55/2)+30, entity->screenY-25, 25, 15, eth.weaponIcons[cgs.clientinfo[entityNum].weapon]);
		}
	}
	
	// Missile esp
	for (entityNum=MAX_CLIENTS; entityNum<MAX_GENTITIES; entityNum++) {
		ethEntity_t* entity = &eth.entities[entityNum];

		if ((cv_espdist.integer != 0) && (entity->distance > cv_espdist.integer))
			continue;
		if (!entity->isInScreen || !isMissile(entityNum))
			continue;

		// Landmine esp
		if(isLandminePlanted(entityNum)) {
			wrap_CG_DrawStringExt(entity->screenX - 4 , entity->screenY, "L", entity->espColor, qtrue, qtrue, 8, 12, 50);
		// Dynamite esp
		} else if(isDynamite(entityNum)) {
			int time = 30 - ((cg.time - cg_entities[entityNum].currentState.effect1Time) / 1000);
			if (time > 0) {
				char timeStr[8];
				sprintf(timeStr, "D:%i", time);
				wrap_CG_DrawStringExt(entity->screenX - 16, entity->screenY, timeStr, entity->espColor, qtrue, qtrue, 8, 12, 50);
			} else
				wrap_CG_DrawStringExt(entity->screenX - 4, entity->screenY, "D", entity->espColor, qtrue, qtrue, 8, 12, 50);
		// Grenade esp
		} else if (isGrenade(entityNum)) {
			wrap_CG_DrawStringExt(entity->screenX - 4, entity->screenY, "G", entity->espColor, qtrue, qtrue, 8, 12, 50);
			printf("%i: orig: %f\n", cg.time, cg_entities[entityNum].lerpOrigin[0]);
			int count = cg.time;
			for (; count <= cg.time + 100; count += 10) {
				vec3_t dest;
				orig_BG_EvaluateTrajectory(&cg_entities[entityNum].currentState.pos, count, dest, qfalse, cg_entities[entityNum].currentState.effect2Time);
				printf("%f ", dest[0]);
			}
			printf("\n");
		// Satchel esp
		} else if (isSatchel(entityNum)) {
			wrap_CG_DrawStringExt(entity->screenX - 4, entity->screenY, "S", entity->espColor, qtrue, qtrue, 8, 12, 50);
		// Panzer esp
		} else if (isPanzer(entityNum)) {
			wrap_CG_DrawStringExt(entity->screenX - 4, entity->screenY, "P", entity->espColor, qtrue, qtrue, 8, 12, 50);
		// Mortar esp
		} else if (isMortar(entityNum)) {
		/*	centity_t* ce = &cg_entities[entityNum];
			entityState_t* cs = &ce->currentState;
			vec3_t or;
			VectorCopy(cg.refdef.vieworg, or);
			SnapVector(or);*/
			
			
	/*trace_t		tr;
	vec3_t		launchPos, testPos;
	vec3_t		angles;
	vec3_t forward;
	VectorCopy(cg.snap->ps.viewaxis[ROLL], forward);

	VectorCopy( cg.snap->ps.viewangles, angles );
	angles[PITCH] -= 60.f;
	AngleVectors( angles, forward, NULL, NULL );
	VectorCopy( cg.refdef.vieworg, launchPos );
	// check for valid start spot (so you don't throw through or get stuck in a wall)
	VectorMA( launchPos, 32, forward, testPos);

	forward[0] *= 3000*1.1f;
	forward[1] *= 3000*1.1f;
	forward[2] *= 1500*1.1f;

	orig_CG_Trace (&tr, testPos, tv(-4.f, -4.f, 0.f), tv(4.f, 4.f, 6.f), launchPos, entityNum, MASK_MISSILESHOT);

	if( tr.fraction < 1 ) {	// oops, bad launch spot
		VectorCopy( tr.endpos, launchPos );
		SnapVectorTowards( launchPos, testPos );
	}*/
		
			
		/*	printf("%i: orig: %f %f\n", cg.time, ce->lerpOrigin[0], cg.snap->ps.viewangles[0]);
			printf("%f:%f:%f\n", or[0], or[1], or[2]);
			printf("%i %i %i %f:%f:%f %f:%f:%f\n", cs->pos.trType, cs->pos.trTime, cs->pos.trDuration, cs->pos.trBase[0], cs->pos.trBase[1], cs->pos.trBase[2], cs->pos.trDelta[0], cs->pos.trDelta[1], cs->pos.trDelta[2]);
*/
			wrap_CG_DrawStringExt(entity->screenX - 4, entity->screenY, "M", entity->espColor, qtrue, qtrue, 8, 12, 50);
		}
	}
	// TODO: World entities esp
}

int isRadar(centity_t *cent, int *screen) {
	vec3_t vec,rot;
	float distance;

	if(cent->currentState.eFlags & EF_DEAD)
		return 0;

	VectorSubtract(cent->lerpOrigin,cg.refdef.vieworg,vec);
	vec[2] = 0.0f;
	distance = VectorLength(vec)*0.025;
	vectoangles(vec,rot);
	rot[1] = AngleNormalize180(rot[1]-cg.refdefViewAngles[1]);
	AngleVectors(rot,vec,0,0);
	VectorScale(vec,distance,vec);

	if(vec[0]>100.0f)
	   VectorScale(vec,100.0f/vec[0],vec);
	else if(vec[0]<-100.0f)
	   VectorScale(vec,-100.0f/vec[0],vec);
	if(vec[1]>100.0f)
	   VectorScale(vec,100.0f/vec[1],vec);
	else if(vec[1]<-100.0f)
	   VectorScale(vec,-100.0f/vec[1],vec);

	screen[0]=((int)-vec[1]+540)-2;
	screen[1]=((int)-vec[0]+100)-2;
	return 1;
}

void drawRadar() {
	vec4_t radarcolor = { 0.5f, 0.5f, 0.5f, 0.25f };
	vec4_t colorBlackR = { 0.7f, 0.7f, 0.7f, 0.50f };

    wrap_CG_FillRect(440, 0,200, 200, radarcolor);
    wrap_CG_FillRect(490, 0, 1, 200, colorBlackR);
    wrap_CG_FillRect(540, 0, 1, 200, colorBlack);
    wrap_CG_FillRect(590, 0, 1, 200, colorBlackR);
    wrap_CG_FillRect(440, 100, 200, 1, colorBlack);
    wrap_CG_FillRect(440, 50, 200, 1, colorBlackR);
    wrap_CG_FillRect(440, 150, 200, 1, colorBlackR);

	int screen[2];
	int player;
	for (player=0; player < MAX_CLIENTS; player++) {
		if (eth.entities[player].isValidPlayer && isRadar(&cg_entities[player], screen)) {
			vec4_t color;
			VectorCopy(eth.entities[player].espColor, color);
			color[3] = 1;
			// added in order not to cross radar area
		    if (screen[0] < 443) screen[0] = 443;
		    if (screen[0] > 637) screen[0] = 637;
		    if (screen[1] > 195) screen[1] = 195;
		    
			wrap_trap_R_SetColor(color);
		    wrap_CG_DrawPic( screen[0]-2, screen[1]-2, 10, 10, eth.classIcons[cgs.clientinfo[player].cls]);
			wrap_trap_R_SetColor(NULL);
	    }
	}
}

void drawStats() {
	vec4_t backColor = { 0.16f, 0.2f, 0.17f, 0.8f };
	vec4_t borderColor = { 0.5f, 0.5f, 0.5f, 0.5f };
	vec4_t fontColor = { 0.625f, 0.625f, 0.6f, 1.0f };

	char gameMod[20];
	wrap_trap_Cvar_VariableStringBuffer("fs_game", gameMod, sizeof(gameMod));

	int killSpree = cg.time - eth.lastKillTime;
	float ratio = 0;
	if ((float)eth.deathCount > 0)
		ratio = (float)eth.killCount / (float)eth.deathCount;

    char stats[128];
    sprintf(stats, "eth v%s | %s^* | %s | kill: %i | kill/death: %.1f",
    		ETH_VERSION, eth.playerName, gameMod, eth.killCount, ratio);

	if (killSpree < (cv_spreetime.integer * 1000))
		sprintf(stats, "%s | spree: %i kill %.1f/%is", stats, eth.killSpreeCount, (float)killSpree / 1000.0f, cv_spreetime.integer);
	
    int w = wrap_CG_Text_Width_Ext(stats, 0.19f, 0, &cgs.media.limboFont1);
    drawFillRect(4, 4, w + 5, 12 + 2, backColor, qtrue, 1, borderColor);
	wrap_CG_Text_Paint_Ext(6, 4 + 11, 0.19f, 0.19f, fontColor, stats, 0, 0, 0, &cgs.media.limboFont1);
}

void drawAdvert() {
	char str[] = "eth v" ETH_VERSION " - *nixCoders ^0[^3*C^0]";
	wrap_CG_DrawStringExt(320 - ((Q_PrintStrlen(str) * 11) / 2), 50, str, colorRed, qfalse, qtrue, 11, 15, 100);
}

int getEnemySpawnTime() {
	if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_ALLIES)
		return 1 + (cg_redlimbotime.integer - ((cgs.aReinfOffset[TEAM_AXIS] + cg.time - cgs.levelStartTime) % cg_redlimbotime.integer)) / 1000;
	else if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS)
		return 1 + (cg_bluelimbotime.integer - ((cgs.aReinfOffset[TEAM_ALLIES] + cg.time - cgs.levelStartTime) % cg_bluelimbotime.integer)) / 1000;
	else
		return -1;
}

void drawSpawnTimerMiddle() {
	int time = getEnemySpawnTime();
	char str[30];
    sprintf(str, "Enemy spawn in: %i", time);
    
	int f_size;
	if (time < 4) 
		f_size = 4 - time;
    else 
		f_size = 0;

    if(f_size != 0)
		wrap_CG_DrawStringExt(260-(f_size*5)+2, 25+2, str, colorRed, qtrue, qtrue, 8 + f_size, 10 + f_size, 100);
    wrap_CG_DrawStringExt(260-(f_size*4), 25, str, colorYellow, qtrue, qtrue, 8 + f_size, 10 + f_size, 100);
}

void drawSpawnTimerRight() {
	vec4_t timerBackground =	 { 0.16f, 0.2f,	0.17f, 0.8f	};
	vec4_t timerBorder = { 0.5f, 0.5f, 0.5f, 0.5f };
	
	char str[5];
    sprintf(str, "%i", getEnemySpawnTime());
    int w = wrap_CG_Text_Width_Ext(str, 0.19f, 0, &cgs.media.limboFont1);
    int y = (20 + 100 + 32) - 14 - 2;	// from sdk
    int x = 634 - w;
    drawFillRect(x - 2, y, w + 5, 12 + 2, timerBackground, qtrue, 1, timerBorder);
	wrap_CG_Text_Paint_Ext(x, y + 11, 0.19f, 0.19f, colorRed, str, 0, 0, 0, &cgs.media.limboFont1);
}

void drawFillRect(int x, int y, int width, int height, vec4_t color, qboolean border, int borderSize, vec4_t borderColor) {
	wrap_CG_FillRect(x, y, width, height, color);
	if (border)
		wrap_CG_DrawRect_FixedBorder(x, y, width, height, borderSize, borderColor );
}

void playSpreeSound() {
	if ((cg.time - eth.lastKillTime) < (cv_spreetime.integer * 1000)) {
		// Define spree levels range - TODO: Dirty. find a better way to play with level range
		typedef struct { int start; int end; } spreeLevel_t;
		#define SPREE_LEVEL_SIZE 4
		spreeLevel_t spreeLevels[SPREE_LEVEL_SIZE] = {
			{ SOUND_DOUBLEKILL1,	SOUND_PERFECT }	,	// Spree level 1
			{ SOUND_GODLIKE1,		SOUND_TRIPPLEKILL },
			{ SOUND_DOMINATING1,	SOUND_ULTRAKILL3 },
			{ SOUND_MONSTERKILL1,	SOUND_WICKEDSICK }	// Spree level 4
		};
		int spreeLevelMax = SPREE_LEVEL_SIZE - 1;

		// Modify level+sound values to fit to spreeLevels_t and eth.sounds order
		int level = eth.killSpreeCount - 2; // never '< 0' because first time here is with 2 kills
		if (level > spreeLevelMax)
			level = spreeLevelMax;
		int levelSize = spreeLevels[level].end - spreeLevels[level].start;
		int sound = (int)((float)(levelSize + 1) * rand() / (RAND_MAX + 1.0f));
		sound += spreeLevels[level].start;
		printf("level: %i lvlsize: %i rand(): %i %i-%i\n", level, levelSize, sound, spreeLevels[level].start, spreeLevels[level].end);
		wrap_trap_S_StartLocalSound(eth.sounds[sound], CHAN_LOCAL_SOUND);
	}
}

void registerEthMedias() {
	// ----- Simple shaders color -----
	eth.redShader = wrap_trap_R_RegisterShaderNoMip(ART_FX_RED);
	eth.blueShader = wrap_trap_R_RegisterShaderNoMip(ART_FX_BLUE);
	eth.greenShader = wrap_trap_R_RegisterShaderNoMip(ART_FX_GREEN);
	eth.yellowShader = wrap_trap_R_RegisterShaderNoMip(ART_FX_YELLOW);
	// cyan <==> teal  error in pak0.pk3/menu/art/fx_{teal|cyan}
	eth.cyanShader = wrap_trap_R_RegisterShaderNoMip(ART_FX_TEAL);
	eth.tealShader = wrap_trap_R_RegisterShaderNoMip(ART_FX_CYAN);

	// ----- Glow shaders -----
	typedef struct {
		char *name;
		char *map;
		qhandle_t* var;
	} shader_t;
	
	char glowShaderSkel[] = { " \
	%s \n\
	{ \n\
		deformVertexes wave 100 sin 4 0 0 0 \n\
		{ \n\
			map %s.tga \n\
			blendfunc GL_ONE GL_ONE \n\
		} \n\
	}\n"};

	shader_t shaders[] = {
		{ "blueGlowShader",		ART_FX_BLUE,	&eth.blueGlowShader },
		{ "cyanGlowShader",		ART_FX_TEAL,	&eth.cyanGlowShader },
		{ "greenGlowShader",	ART_FX_GREEN,	&eth.greenGlowShader },
		{ "redGlowShader",		ART_FX_RED,		&eth.redGlowShader },
		{ "tealGlowShader",		ART_FX_CYAN,	&eth.tealGlowShader },
		{ "yellowGlowShader",	ART_FX_YELLOW,	&eth.yellowGlowShader },
	};
	
	int count = 0;
	for (; count < (sizeof(shaders) / sizeof(shader_t)); count++) {
		shader_t shader = shaders[count];
		char shaderCmd[strlen(glowShaderSkel) + 30];
		sprintf(shaderCmd, glowShaderSkel, shader.name, shader.map);
		wrap_trap_R_LoadDynamicShader(shader.name, shaderCmd);
		*shader.var = wrap_trap_R_RegisterShaderNoMip(shader.name);
	}

	// ----- Custom sounds -----
	// Misc sound
	eth.sounds[SOUND_HUMILIATION] = wrap_trap_S_RegisterSound("sound/misc/humiliation.wav", qfalse);
	eth.sounds[SOUND_PREPARE] = wrap_trap_S_RegisterSound("sound/misc/prepare.wav", qfalse);
	eth.sounds[SOUND_HEADSHOT1] = wrap_trap_S_RegisterSound("sound/misc/headshot1.wav", qfalse);
	eth.sounds[SOUND_HEADSHOT2] = wrap_trap_S_RegisterSound("sound/misc/headshot2.wav", qfalse);
	// Spree sound
	// level 1
	eth.sounds[SOUND_DOUBLEKILL1] = wrap_trap_S_RegisterSound("sound/spree/level1/doublekill1.wav", qfalse);
	eth.sounds[SOUND_EXCELENT] = wrap_trap_S_RegisterSound("sound/spree/level1/excelent.wav", qfalse);
	eth.sounds[SOUND_IMPRESSIVE] = wrap_trap_S_RegisterSound("sound/spree/level1/impressive.wav", qfalse);
	eth.sounds[SOUND_KILLINGSPREE1] = wrap_trap_S_RegisterSound("sound/spree/level1/killingspree1.wav", qfalse);
	eth.sounds[SOUND_KILLINGSPREE2] = wrap_trap_S_RegisterSound("sound/spree/level1/killingspree2.wav", qfalse);
	eth.sounds[SOUND_PERFECT] = wrap_trap_S_RegisterSound("sound/spree/level1/perfect.wav", qfalse);
	// level 2
	eth.sounds[SOUND_GODLIKE1] = wrap_trap_S_RegisterSound("sound/spree/level2/godlike1.wav", qfalse);
	eth.sounds[SOUND_GODLIKE2] = wrap_trap_S_RegisterSound("sound/spree/level2/godlike2.wav", qfalse);
	eth.sounds[SOUND_HOLYSHIT] = wrap_trap_S_RegisterSound("sound/spree/level2/holyshit.wav", qfalse);
	eth.sounds[SOUND_MULTIKILL1] = wrap_trap_S_RegisterSound("sound/spree/level2/multikill1.wav", qfalse);
	eth.sounds[SOUND_MULTIKILL2] = wrap_trap_S_RegisterSound("sound/spree/level2/multikill2.wav", qfalse);
	eth.sounds[SOUND_MULTIKILL3] = wrap_trap_S_RegisterSound("sound/spree/level2/multikill3.wav", qfalse);
	eth.sounds[SOUND_TRIPPLEKILL] = wrap_trap_S_RegisterSound("sound/spree/level2/tripplekill.wav", qfalse);
	// level 3
	eth.sounds[SOUND_DOMINATING1] = wrap_trap_S_RegisterSound("sound/spree/level3/dominating1.wav", qfalse);
	eth.sounds[SOUND_DOMINATING2] = wrap_trap_S_RegisterSound("sound/spree/level3/dominating2.wav", qfalse);
	eth.sounds[SOUND_ULTRAKILL1] = wrap_trap_S_RegisterSound("sound/spree/level3/ultrakill1.wav", qfalse);
	eth.sounds[SOUND_ULTRAKILL2] = wrap_trap_S_RegisterSound("sound/spree/level3/ultrakill2.wav", qfalse);
	eth.sounds[SOUND_ULTRAKILL3] = wrap_trap_S_RegisterSound("sound/spree/level3/ultrakill3.wav", qfalse);
	// level 4
	eth.sounds[SOUND_MONSTERKILL1] = wrap_trap_S_RegisterSound("sound/spree/level4/monsterkill1.wav", qfalse);
	eth.sounds[SOUND_MONSTERKILL2] = wrap_trap_S_RegisterSound("sound/spree/level4/monsterkill2.wav", qfalse);
	eth.sounds[SOUND_UNREAL] = wrap_trap_S_RegisterSound("sound/spree/level4/unreal.wav", qfalse);
	eth.sounds[SOUND_UNSTOPPABLE1] = wrap_trap_S_RegisterSound("sound/spree/level4/unstoppable1.wav", qfalse);
	eth.sounds[SOUND_UNSTOPPABLE2] = wrap_trap_S_RegisterSound("sound/spree/level4/unstoppable2.wav", qfalse);
	eth.sounds[SOUND_WICKEDSICK] = wrap_trap_S_RegisterSound("sound/spree/level4/wickedsick.wav", qfalse);

	// ----- Rank Icons esp -----
	eth.rankIcons[0] = wrap_trap_R_RegisterShaderNoMip("gfx/loading/pin_neutral");
	eth.rankIcons[1] = wrap_trap_R_RegisterShaderNoMip("gfx/loading/pin_neutral");
	eth.rankIcons[2] = wrap_trap_R_RegisterShaderNoMip("gfx/hud/ranks/rank2");
	eth.rankIcons[3] = wrap_trap_R_RegisterShaderNoMip("gfx/hud/ranks/rank3");
	eth.rankIcons[4] = wrap_trap_R_RegisterShaderNoMip("gfx/hud/ranks/rank4");
	eth.rankIcons[5] = wrap_trap_R_RegisterShaderNoMip("gfx/hud/ranks/rank5");
	eth.rankIcons[6] = wrap_trap_R_RegisterShaderNoMip("gfx/hud/ranks/rank6");
	eth.rankIcons[7] = wrap_trap_R_RegisterShaderNoMip("gfx/hud/ranks/rank7");
	eth.rankIcons[8] = wrap_trap_R_RegisterShaderNoMip("gfx/hud/ranks/rank8");
	eth.rankIcons[9] = wrap_trap_R_RegisterShaderNoMip("gfx/hud/ranks/rank9");
	eth.rankIcons[10] = wrap_trap_R_RegisterShaderNoMip("gfx/hud/ranks/rank10");
	eth.rankIcons[11] = wrap_trap_R_RegisterShaderNoMip("gfx/hud/ranks/rank11");

	// ----- Class Icons esp -----
	eth.classIcons[PC_SOLDIER] = wrap_trap_R_RegisterShaderNoMip("gfx/limbo/ic_soldier");
	eth.classIcons[PC_MEDIC] = wrap_trap_R_RegisterShaderNoMip("gfx/limbo/ic_medic");
	eth.classIcons[PC_ENGINEER] = wrap_trap_R_RegisterShaderNoMip("gfx/limbo/ic_engineer");
	eth.classIcons[PC_FIELDOPS] = wrap_trap_R_RegisterShaderNoMip("gfx/limbo/ic_fieldops");
	eth.classIcons[PC_COVERTOPS] = wrap_trap_R_RegisterShaderNoMip("gfx/limbo/ic_covertops");

	// ----- Weapon Icons esp -----
	eth.weaponIcons[MOD_BROWNING] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_browning_1_select");
	eth.weaponIcons[WP_MORTAR] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_mortar_1_select");
	eth.weaponIcons[WP_AKIMBO_SILENCEDLUGER] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_colt_1_select");
	eth.weaponIcons[WP_AKIMBO_LUGER] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_colt_1_select");
	eth.weaponIcons[WP_AKIMBO_COLT]	 = wrap_trap_R_RegisterShaderNoMip("icons/iconw_colt_1_select");
	eth.weaponIcons[WP_AKIMBO_SILENCEDCOLT] 	= wrap_trap_R_RegisterShaderNoMip("icons/iconw_colt_1_select");
	eth.weaponIcons[WP_NONE] = wrap_trap_R_RegisterShaderNoMip("icons/noammo");
	eth.weaponIcons[WP_SMOKE_MARKER] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_smokegrenade_1_select");
	eth.weaponIcons[WP_SMOKETRAIL] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_smokegrenade_1_select");
	eth.weaponIcons[WP_MAPMORTAR] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_grenade_1_select");
	eth.weaponIcons[WP_ARTY] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_syringe_1_select");
	eth.weaponIcons[WP_K43_SCOPE] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_mauser_1_select");
	eth.weaponIcons[WP_GPG40] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_kar98_1_select");
	eth.weaponIcons[WP_M7] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_m1_garand_1_select");
	eth.weaponIcons[WP_GARAND_SCOPE] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_mauser_1_select");
	eth.weaponIcons[WP_FG42SCOPE] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_fg42_1_select");
	eth.weaponIcons[WP_MORTAR_SET] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_mortar_1_select");
	eth.weaponIcons[WP_SMOKE_BOMB] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_dynamite_1_select");
	eth.weaponIcons[WP_TRIPMINE] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_dynamite_1_select");
	eth.weaponIcons[WP_MOBILE_MG42_SET] 	= wrap_trap_R_RegisterShaderNoMip("icons/iconw_mg42_1_select");
	eth.weaponIcons[WP_SILENCED_COLT] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_colt_1_select");
	eth.weaponIcons[WP_MEDKIT] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_medheal_select");
	eth.weaponIcons[WP_MOBILE_MG42] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_mg42_1_select");
	eth.weaponIcons[WP_THOMPSON] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_thompson_1_select");
	eth.weaponIcons[WP_KNIFE] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_knife_1_select");
	eth.weaponIcons[WP_LUGER] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_luger_1_select");
	eth.weaponIcons[WP_MP40] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_MP40_1_select");
	eth.weaponIcons[WP_GRENADE_LAUNCHER] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_grenade_1_select");
	eth.weaponIcons[WP_PANZERFAUST] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_panzerfaust_1_select");
	eth.weaponIcons[WP_FLAMETHROWER] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_flamethrower_1_select");
	eth.weaponIcons[WP_COLT] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_colt_1_select");
	eth.weaponIcons[WP_GRENADE_PINEAPPLE] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_pineapple_1_select");
	eth.weaponIcons[WP_STEN] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_sten_1_select");
	eth.weaponIcons[WP_MEDIC_SYRINGE] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_syringe_1_select");
	eth.weaponIcons[WP_AMMO] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_ammopack_1_select");
	eth.weaponIcons[WP_SILENCER] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_silencer_1_select");
	eth.weaponIcons[WP_DYNAMITE] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_dynamite_1_select");
	eth.weaponIcons[WP_MEDKIT] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_medheal_1_select");
	eth.weaponIcons[WP_BINOCULARS] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_binoculars_1_select");
	eth.weaponIcons[WP_PLIERS] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_pliers_1_select");
	eth.weaponIcons[WP_KAR98] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_kar98_1_select");
	eth.weaponIcons[WP_CARBINE] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_mauser_1_select");
	eth.weaponIcons[WP_GARAND] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_m1_garand_1_select");
	eth.weaponIcons[WP_LANDMINE] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_landmine_1_select");
	eth.weaponIcons[WP_SATCHEL] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_satchel_1_select");
	eth.weaponIcons[WP_SATCHEL_DET] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_radio_1_select");
	eth.weaponIcons[WP_K43] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_mauser_1_select");
	eth.weaponIcons[WP_FG42] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_fg42_1_select");
	eth.weaponIcons[WP_MEDIC_ADRENALINE] = wrap_trap_R_RegisterShaderNoMip("icons/iconw_syringe_1_select");
}


/*
==============================
 Menus
==============================
*/

typedef struct {
	char *name;
	int value;
} choice_t;

#define MAX_CHOICES 10
typedef struct {
	char *name;
	char *cvarName;
	int *cvarValue;
	choice_t choices[MAX_CHOICES];
} menuItem_t;

static menuItem_t menuItems[] = {
	{ "--- AIMBOT ---", "", NULL, {} },	////////////////////
	{ " Target", "aim",	 &cv_aim.integer, {
		{ "Off", 0 }, { "Head", 1 }, { "Mouth", 2 }, { "Torso", 3 }, { "Chest", 4 }
	} },
	{ " VectorZ correction", "aimvecz", &cv_aimvecz.integer, {
		{ "0", 0 }, { "2", 2 }, { "4", 4 }, { "5", 5 }, { "6", 6 }, { "8", 8 }, { "10", 10 }
	} },
	{ " Shoot dead enemy", "aimdead", &cv_aimdead.integer, {
		{ "Off", 0 }, { "On", 1 }
	} },
	{ "--- VISUAL ---", "", NULL, {} },	////////////////////
	{ " Advert", "advert", &cv_advert.integer, {
		{ "Off", 0 }, { "On", 1 }
	} },
	{ " Chams", "chams", &cv_chams.integer, {
		{ "Off", 0 }, { "T1", 1 }, { "T2", 2 }, { "T3", 3 }
	} },
	{ " Glow", "glow", &cv_glow.integer, {
		{ "Off", 0 }, { "T1", 1 }, { "T2", 2 }
	} },
	{ " Radar", "radar", &cv_radar.integer, {
		{ "Off", 0 }, { "On", 1 }
	} },
	{ " Spawntimer", "spawntimer", &cv_spawntimer.integer, {
		{ "Off", 0 }, { "Top", 1 }, { "Right", 2 }
	} },
	{ " Stats", "stats", &cv_stats.integer, {
		{ "Off", 0 }, { "On", 1 }
	} },
	{ " Wallhack", "wallhack", &cv_wallhack.integer, {
		{ "Off", 0 }, { "On", 1 }
	} },
	{ "--- ESP ---", "", NULL, {} },	////////////////////
	{ " Esp fade", "espfade", &cv_espfade.integer, {
		{ "100%", 100 }, { "80%", 80 }, { "70%", 70 }, { "60%", 60 }, { "50%", 50 }
	} },
	{ " Player name", "espname", &cv_espname.integer, {
		{ "Off", 0 }, { "Uni", 1 }, { "Multi", 2 }
	} },
	{ " Player name background", "espname_bg", &cv_espname_bg.integer, {
		{ "Off", 0 }, { "On", 1 }
	} },
	{ " Player icon", "espicon", &cv_espicon.integer, {
		{ "Off", 0 }, { "On", 1 }
	} },
	{ " Player icon background", "espicon_bg", &cv_espicon_bg.integer, {
		{ "Off", 0 }, { "On", 1 }
	} },
	{ " Distance esp", "espdist", &cv_espdist.integer, {
		{ "Off", 0 }, { "1000", 1000 }, { "1500", 1500 }, { "2000", 2000 }, { "2500", 2500 }, { "3000", 3000 }, { "4000", 4000 }
	} },
	{ "--- SOUND ---", "", NULL, {} },	////////////////////
	{ " Spree sound", "sndspree", &cv_sndspree.integer, {
		{ "Off", 0 }, { "On", 1 }
	} },
	{ " Misc. sound", "sndmisc", &cv_sndmisc.integer, {
		{ "Off", 0 }, { "On", 1 }
	} },
	{ " Spree Time", "spreetime", &cv_spreetime.integer, {
		{ "5", 5 }, { "10", 10 }, { "15", 15 }
	} },
	{ "--- MISC ---", "", NULL, {} },	////////////////////
	{ " Auto shoot", "autoshoot", &cv_autoshoot.integer, {
		{ "Off", 0 }, { "On", 1 }
	} },
	{ " Bunny jumping", "bunnyjump", &cv_bunnyjump.integer, {
		{ "Off", 0 }, { "On", 1 }
	} },
	{ " Sniper zoom removal", "sniperzoom", &cv_sniperzoom.integer, {
		{ "Off", 0 }, { "On", 1 }
	} }
};

#define MENU_ITEM_W 140
#define MENU_ITEM_H	14
#define MENU_ITEM_SPACE	1
#define MENU_ITEM_VALUE_W 80
#define MENU_SCREEN_BORDER 20

vec4_t backColor = { 0.16f, 0.2f, 0.17f, 0.8f };
vec4_t borderColor = { 0.5f, 0.5f, 0.5f, 0.5f };
vec4_t fontColor = { 0.625f, 0.625f, 0.6f, 1.0f	};

void drawMenuItem(menuItem_t menuItem, int x, int y, qboolean highlight) {
	// Draw name
	vec4_t charColor;
	if (menuItem.cvarValue == NULL) // If category
		Vector4Copy(colorWhite, charColor);
	else
		Vector4Copy(highlight ? colorWhite : fontColor, charColor);
	drawFillRect(x, y, MENU_ITEM_W, MENU_ITEM_H, highlight ? fontColor : backColor, qtrue, 1, borderColor);
	wrap_CG_Text_Paint_Ext(x + (MENU_ITEM_H / 2), y + MENU_ITEM_H - 3, 0.20f, 0.20f, charColor, menuItem.name, 0, 0, 0, &cgs.media.limboFont1);

	// If it's a category don't draw value
	if (menuItem.cvarValue == NULL)
		return;

	// Draw value
	// Search for name of this value
	int count = 0;
	while ((menuItem.choices[count].name != NULL) // Check for valid choice
			&& (menuItem.choices[count].value != *menuItem.cvarValue)
			&& (count < MAX_CHOICES)) {
		count++;
	}
	// Get value
	char value[20];
	if ((count == MAX_CHOICES) || (menuItem.choices[count].name == NULL))
		sprintf(value, "%d", *menuItem.cvarValue);
	else
		strncpy(value, menuItem.choices[count].name, sizeof(value));

	x += MENU_ITEM_W + MENU_ITEM_SPACE;
	drawFillRect(x, y, MENU_ITEM_VALUE_W, MENU_ITEM_H, highlight ? fontColor : backColor, qtrue, 1, borderColor);
	int w = wrap_CG_Text_Width_Ext(value, 0.20f, 0, &cgs.media.limboFont1);
	wrap_CG_Text_Paint_Ext(x + (MENU_ITEM_VALUE_W / 2) - (w / 2), y + MENU_ITEM_H - 3, 0.20f, 0.20f, highlight ? colorWhite : fontColor, value, 0, 0, 0, &cgs.media.limboFont1);
}

void drawMenu() {
	static int openCategory = -1;
	int actualCategory = -1;
	int nextOpenCategory = openCategory;
	int y = MENU_SCREEN_BORDER;
	int count = 0;
	for (; count < (sizeof(menuItems) / sizeof(menuItem_t)); count++) {
		qboolean isMouseOn = ((eth.mouseX > MENU_SCREEN_BORDER)
				&& (eth.mouseX < (MENU_SCREEN_BORDER + MENU_ITEM_W + MENU_ITEM_SPACE + MENU_ITEM_VALUE_W))
				&& (eth.mouseY > y)
				&& (eth.mouseY < (y + MENU_ITEM_H)));

		// If category
		if (menuItems[count].cvarValue == NULL) {
			actualCategory = count;
		} else if (actualCategory != openCategory)
			continue;

		drawMenuItem(menuItems[count], MENU_SCREEN_BORDER, y, isMouseOn);
		y += MENU_ITEM_H + MENU_ITEM_SPACE;

		if ((eth.mouseButton == Button1) || (eth.mouseButton == Button3)) {
			if (isMouseOn) {
				// Category click
				if (menuItems[count].cvarValue == NULL) {
					eth.mouseButton = 0;
					if (nextOpenCategory != actualCategory)
						wrap_trap_S_StartLocalSound(cgs.media.sndLimboSelect, CHAN_LOCAL_SOUND);
					nextOpenCategory = actualCategory;
				// Menu item click
				} else if (menuItems[count].cvarValue != NULL) {
					wrap_trap_S_StartLocalSound(cgs.media.sndLimboSelect, CHAN_LOCAL_SOUND);
					// Search position of the current value
					int pos = 0;
					while ((menuItems[count].choices[pos].value != *menuItems[count].cvarValue)
							&& (menuItems[count].choices[pos].name != NULL) // Check for valid choice
							&& (pos < MAX_CHOICES)) {
						pos++;
					}
					// If current value no found take 0
					if (pos == 10)
						pos = 0;
					// Get max choices
					int max = 0;
					while (menuItems[count].choices[max].name != NULL)
						max++;
					// Set choice
					if (eth.mouseButton == Button1)
						pos++;
					else
						pos--;
					// Check bounds
					if (pos >= max)
						pos = 0;
					else if (pos < 0)
						pos = max - 1;
					// Set cvar
					char cmd[50];
					sprintf(cmd, "%s%s %i\n", ETH_CVAR_PREFIX, menuItems[count].cvarName, menuItems[count].choices[pos].value);
					wrap_trap_CG_SendConsoleCommand(cmd);
					eth.mouseButton = 0;
				}
			// Right part of the screen click
			} else if (eth.mouseX > (640 / 2)) {
				eth.mouseButton = 0;
				eth.isMenuOpen = qfalse;
			}
		}
	}
	openCategory = nextOpenCategory;
}
