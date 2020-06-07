// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2005 *nixCoders team - don't forget to credits us
// $Id$

#include "eth.h"

// All functions here take from sdk

/*
==============================
from game/bg_misc.c
==============================
*/

void wrap_BG_CalculateSpline_r(splinePath_t* spline, vec3_t out1, vec3_t out2, float tension) {
	vec3_t points[18];
	int i;
	int count = spline->numControls + 2;
	vec3_t dist;

	VectorCopy( spline->point.origin, points[0] );
	for( i = 0; i < spline->numControls; i++ ) {
		VectorCopy( spline->controls[i].origin, points[i+1] );
	}
	if(!spline->next) {
		return;
//		Com_Error( ERR_DROP, "Spline (%s) with no target referenced", spline->point.name );
	}
	VectorCopy( spline->next->point.origin, points[i+1] );


	while(count > 2) {
		for( i = 0; i < count-1; i++ ) {
			VectorSubtract( points[i+1], points[i], dist );
			VectorMA(points[i], tension, dist, points[i]);
		}
		count--;
	}

	VectorCopy( points[0], out1 );
	VectorCopy( points[1], out2 );
}

void wrap_BG_EvaluateTrajectory(const trajectory_t *tr, int atTime, vec3_t result, qboolean isAngle, int splinePath) {
	float		deltaTime;
	float		phase;
	vec3_t		v;

	splinePath_t* pSpline;
	vec3_t vec[2];
	qboolean backwards = qfalse;
	float deltaTime2;

	//printf("EVALUATETRAJECTORY: %i\n", tr->trType);
	switch( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
	case TR_GRAVITY_PAUSED:	//----(SA)	
		VectorCopy( tr->trBase, result );
		break;
	case TR_LINEAR:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = sin( deltaTime * M_PI * 2 );
		VectorMA( tr->trBase, phase, tr->trDelta, result );
		break;
//----(SA)	removed
	case TR_LINEAR_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		if ( deltaTime < 0 ) {
			deltaTime = 0;
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_GRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5 * DEFAULT_GRAVITY * deltaTime * deltaTime;		// FIXME: local gravity...
		break;
	// Ridah
	case TR_GRAVITY_LOW:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5 * (DEFAULT_GRAVITY * 0.3) * deltaTime * deltaTime;		// FIXME: local gravity...
		break;
	// done.
//----(SA)	
	case TR_GRAVITY_FLOAT:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5 * (DEFAULT_GRAVITY * 0.2) * deltaTime;
		break;
//----(SA)	end
	// RF, acceleration
	case TR_ACCELERATE:		// trDelta is the ultimate speed
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		// phase is the acceleration constant
		phase = VectorLength( tr->trDelta ) / (tr->trDuration*0.001);
		// trDelta at least gives us the acceleration direction
		VectorNormalize2( tr->trDelta, result );
		// get distance travelled at current time
		VectorMA( tr->trBase, phase * 0.5 * deltaTime * deltaTime, result, result );
		break;
	case TR_DECCELERATE:	// trDelta is the starting speed
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		// phase is the breaking constant
		phase = VectorLength( tr->trDelta ) / (tr->trDuration*0.001);
		// trDelta at least gives us the acceleration direction
		VectorNormalize2( tr->trDelta, result );
		// get distance travelled at current time (without breaking)
		VectorMA( tr->trBase, deltaTime, tr->trDelta, v );
		// subtract breaking force
		VectorMA( v, -phase * 0.5 * deltaTime * deltaTime, result, result );
		break;
	case TR_SPLINE:
		if(!(pSpline = wrap_BG_GetSplineData( splinePath, &backwards ))) {
			return;
		}

		deltaTime = tr->trDuration ? (atTime - tr->trTime) / ((float)tr->trDuration) : 0;

		if(deltaTime < 0.f) {
			deltaTime = 0.f;
		} else if(deltaTime > 1.f) {
			deltaTime = 1.f;
		}

		if(backwards) {
			deltaTime = 1 - deltaTime;
		}

/*		if(pSpline->isStart) {
			deltaTime = 1 - sin((1 - deltaTime) * M_PI * 0.5f);
		} else if(pSpline->isEnd) {
			deltaTime = sin(deltaTime * M_PI * 0.5f);
		}*/

		deltaTime2 = deltaTime;

		wrap_BG_CalculateSpline_r( pSpline, vec[0], vec[1], deltaTime );

		if(isAngle) {
			qboolean dampin = qfalse;
			qboolean dampout = qfalse;
			float base1;

			if(tr->trBase[0]) {
//				int pos = 0;
				vec3_t result2;
				splinePath_t* pSp2 = pSpline;

				deltaTime2 += tr->trBase[0] / pSpline->length;

				if(wrap_BG_TraverseSpline( &deltaTime2, &pSp2 )) {

					VectorSubtract( vec[1], vec[0], result );
					VectorMA(vec[0], deltaTime, result, result);	

					wrap_BG_CalculateSpline_r( pSp2, vec[0], vec[1], deltaTime2 );

					VectorSubtract( vec[1], vec[0], result2 );
					VectorMA(vec[0], deltaTime2, result2, result2);

					if(	tr->trBase[0] < 0 ) {
						VectorSubtract( result, result2, result );
					} else {
						VectorSubtract( result2, result, result );
					}
				} else {
					VectorSubtract( vec[1], vec[0], result );
				}
			} else {
				VectorSubtract( vec[1], vec[0], result );
			}

			vectoangles( result, result );

			base1 = tr->trBase[1];
			if(base1 >= 10000 || base1 < -10000) {
				dampin = qtrue;
				if(base1 < 0) {
					base1 += 10000;
				} else {
					base1 -= 10000;
				}
			}

			if(base1 >= 1000 || base1 < -1000) {
				dampout = qtrue;
				if(base1 < 0) {
					base1 += 1000;
				} else {
					base1 -= 1000;
				}
			}

			if(dampin && dampout) {
				result[ROLL] = base1 + ((sin(((deltaTime * 2) - 1) * M_PI * 0.5f) + 1) * 0.5f * tr->trBase[2]);
			} else if(dampin) {
				result[ROLL] = base1 + (sin(deltaTime * M_PI * 0.5f) * tr->trBase[2]);
			} else if(dampout) {
				result[ROLL] = base1 + ((1 - sin((1 - deltaTime) * M_PI * 0.5f)) * tr->trBase[2]);
			} else {
				result[ROLL] = base1 + (deltaTime * tr->trBase[2]);
			}
		} else {
			VectorSubtract( vec[1], vec[0], result );
			VectorMA(vec[0], deltaTime, result, result);	
		}

		break;
	case TR_LINEAR_PATH:
		if(!(pSpline = wrap_BG_GetSplineData( splinePath, &backwards ))) {
			return;
		}

		deltaTime = tr->trDuration ? (atTime - tr->trTime) / ((float)tr->trDuration) : 0;

		if(deltaTime < 0.f) {
			deltaTime = 0.f;
		} else if(deltaTime > 1.f) {
			deltaTime = 1.f;
		}

		if(backwards) {
			deltaTime = 1 - deltaTime;
		}

		if(isAngle) {
			int pos = floor(deltaTime * (MAX_SPLINE_SEGMENTS));
			float frac;

			if(pos >= MAX_SPLINE_SEGMENTS) {
				pos = MAX_SPLINE_SEGMENTS - 1;
				frac = pSpline->segments[pos].length;
			} else {
				frac = ((deltaTime * (MAX_SPLINE_SEGMENTS)) - pos) * pSpline->segments[pos].length;
			}

			if(tr->trBase[0]) {
				VectorMA( pSpline->segments[pos].start, frac, pSpline->segments[pos].v_norm, result );
				VectorCopy( result, v );
				// TODO: BG_LinearPathOrigin2 one more :(
				BG_LinearPathOrigin2( tr->trBase[0], &pSpline, &deltaTime, v, backwards );
				if(	tr->trBase[0] < 0 ) {
					VectorSubtract( v, result, result );
				} else {
					VectorSubtract( result, v, result );
				}

				vectoangles( result, result );
			} else {
				vectoangles( pSpline->segments[pos].v_norm, result );
			}

		} else {
			int pos = floor(deltaTime * (MAX_SPLINE_SEGMENTS));
			float frac;

			if(pos >= MAX_SPLINE_SEGMENTS) {
				pos = MAX_SPLINE_SEGMENTS - 1;
				frac = pSpline->segments[pos].length;
			} else {
				frac = ((deltaTime * (MAX_SPLINE_SEGMENTS)) - pos) * pSpline->segments[pos].length;
			}

			VectorMA( pSpline->segments[pos].start, frac, pSpline->segments[pos].v_norm, result );
		}

		break;
	default:
		//Com_Error( ERR_DROP, "BG_EvaluateTrajectory: unknown trType: %i", tr->trTime );
		break;
	}
}

splinePath_t* wrap_BG_GetSplineData( int number, qboolean* backwards ) {
	if( number < 0 ) {
		*backwards = qtrue;
		number = -number;
	} else {
		*backwards = qfalse;
	}
	number--;

	/*if( number < 0 || number >= numSplinePaths ) { //TODO: find numSplinePaths
		return NULL;
	}

	return &splinePaths[number];*/ return NULL;
}

qboolean wrap_BG_TraverseSpline( float* deltaTime, splinePath_t** pSpline) {
	float dist;

	while( (*deltaTime) > 1 ) {
		(*deltaTime) -= 1;
		dist = (*pSpline)->length * (*deltaTime);

		if(!(*pSpline)->next || !(*pSpline)->next->length) {
			return qfalse;
//			Com_Error( ERR_DROP, "Spline path end passed (%s)", (*pSpline)->point.name );
		}

		(*pSpline) = (*pSpline)->next;
		*deltaTime = dist / (*pSpline)->length;
	}

	while( (*deltaTime) < 0 ) {
		dist = -((*pSpline)->length * (*deltaTime));

		if(!(*pSpline)->prev || !(*pSpline)->prev->length) {
			return qfalse;
//			Com_Error( ERR_DROP, "Spline path end passed (%s)", (*pSpline)->point.name );
		}

		(*pSpline) = (*pSpline)->prev;
		(*deltaTime) = 1 - (dist / (*pSpline)->length);
	}

	return qtrue;
}

/*
==============================
from game/q_shared.c
==============================
*/
int Q_PrintStrlen( const char *string ) {
	int			len;
	const char	*p;

	if( !string ) {
		return 0;
	}

	len = 0;
	p = string;
	while( *p ) {
		if( Q_IsColorString( p ) ) {
			p += 2;
			continue;
		}
		p++;
		len++;
	}

	return len;
}

/*
==============================
from cgame/cg_predict.c
==============================
*/

static	int			cg_numSolidEntities;
static	centity_t	*cg_solidEntities[MAX_ENTITIES_IN_SNAPSHOT];
static	int			cg_numSolidFTEntities;
static	centity_t	*cg_solidFTEntities[MAX_ENTITIES_IN_SNAPSHOT];
static	int			cg_numTriggerEntities;
static	centity_t	*cg_triggerEntities[MAX_ENTITIES_IN_SNAPSHOT];

void wrap_CG_BuildSolidList(void) {
	int			i;
	centity_t	*cent;
	snapshot_t	*snap;
	entityState_t	*ent;

	cg_numSolidEntities = 0;
	cg_numSolidFTEntities = 0;
	cg_numTriggerEntities = 0;

	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		snap = cg.nextSnap;
	} else {
		snap = cg.snap;
	}

	for ( i = 0 ; i < snap->numEntities ; i++ ) {
		cent = &cg_entities[ snap->entities[ i ].number ];
		ent = &cent->currentState;

		// rain - don't clip against temporarily non-solid SOLID_BMODELS
		// (e.g. constructibles); use current state so prediction isn't fubar
		if( cent->currentState.solid == SOLID_BMODEL &&
			( cent->currentState.eFlags & EF_NONSOLID_BMODEL ) ) {
			continue;
		}

		if( ent->eType == ET_ITEM || 
			ent->eType == ET_PUSH_TRIGGER || 
			ent->eType == ET_TELEPORT_TRIGGER || 
			ent->eType == ET_CONCUSSIVE_TRIGGER || 
			ent->eType == ET_OID_TRIGGER 
#ifdef VISIBLE_TRIGGERS
			|| ent->eType == ET_TRIGGER_MULTIPLE
			|| ent->eType == ET_TRIGGER_FLAGONLY
			|| ent->eType == ET_TRIGGER_FLAGONLY_MULTIPLE
#endif // VISIBLE_TRIGGERS
			) {

			cg_triggerEntities[cg_numTriggerEntities] = cent;
			cg_numTriggerEntities++;
			continue;
		}

		if(	ent->eType == ET_CONSTRUCTIBLE ) {
			cg_triggerEntities[cg_numTriggerEntities] = cent;
			cg_numTriggerEntities++;
		}

		if ( cent->nextState.solid) {
/*			if(cg_fastSolids.integer) { // Gordon: "optimization" (disabling until i fix it)
				vec3_t vec;
				float len;

				cg_solidFTEntities[cg_numSolidFTEntities] = cent;
				cg_numSolidFTEntities++;

				// FIXME: use range to bbox, not to origin
				if ( cent->nextState.solid == SOLID_BMODEL ) {
					VectorAdd( cgs.inlineModelMidpoints[ cent->currentState.modelindex ], cent->lerpOrigin, vec );
					VectorSubtract( vec, cg.predictedPlayerEntity.lerpOrigin, vec );
				} else {
					VectorSubtract( cent->lerpOrigin, cg.predictedPlayerEntity.lerpOrigin, vec );
				}
				if((len = DotProduct( vec, vec )) < (512 * 512)) {
					cg_solidEntities[cg_numSolidEntities] = cent;
					cg_numSolidEntities++;
					continue;
				}
			} else*/ {
				cg_solidEntities[cg_numSolidEntities] = cent;
				cg_numSolidEntities++;

				cg_solidFTEntities[cg_numSolidFTEntities] = cent;
				cg_numSolidFTEntities++;
				continue;
			}
		}
	}
}

void wrap_CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, 
					 int skipNumber, int mask ) {
	trace_t	t;

	wrap_trap_CM_BoxTrace ( &t, start, end, mins, maxs, 0, mask );
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	// check all other solid models
	wrap_CG_ClipMoveToEntities (start, mins, maxs, end, skipNumber, mask, qfalse, &t);

	*result = t;
}

void wrap_CG_ClipMoveToEntities(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
		int skipNumber, int mask, int capsule, trace_t *tr) {
	int			i, x, zd, zu;
	trace_t		trace;
	entityState_t	*ent;
	clipHandle_t 	cmodel;
	vec3_t		bmins, bmaxs;
	vec3_t		origin, angles;
	centity_t	*cent;

	for ( i = 0 ; i < cg_numSolidEntities ; i++ ) {
		cent = cg_solidEntities[ i ];
		ent = &cent->currentState;

		if ( ent->number == skipNumber ) {
			continue;
		}

		if ( ent->solid == SOLID_BMODEL ) {
			// special value for bmodel
			cmodel = wrap_trap_CM_InlineModel( ent->modelindex );
//			VectorCopy( cent->lerpAngles, angles );
//			VectorCopy( cent->lerpOrigin, origin );
			wrap_BG_EvaluateTrajectory( &cent->currentState.apos, cg.physicsTime, angles, qtrue, cent->currentState.effect2Time );
			wrap_BG_EvaluateTrajectory( &cent->currentState.pos, cg.physicsTime, origin, qfalse, cent->currentState.effect2Time );
		} else {
			// encoded bbox
			x = (ent->solid & 255);
			zd = ((ent->solid>>8) & 255);
			zu = ((ent->solid>>16) & 255) - 32;

			bmins[0] = bmins[1] = -x;
			bmaxs[0] = bmaxs[1] = x;
			bmins[2] = -zd;
			bmaxs[2] = zu;

			//cmodel = trap_CM_TempCapsuleModel( bmins, bmaxs );
			cmodel = wrap_trap_CM_TempBoxModel( bmins, bmaxs );

			VectorCopy( vec3_origin, angles );
			VectorCopy( cent->lerpOrigin, origin );
		}
		// MrE: use bbox of capsule
		if (capsule) {
			wrap_trap_CM_TransformedCapsuleTrace ( &trace, start, end,
				mins, maxs, cmodel,  mask, origin, angles);
		}
		else {
			wrap_trap_CM_TransformedBoxTrace ( &trace, start, end,
				mins, maxs, cmodel,  mask, origin, angles);
		}

		if (trace.allsolid || trace.fraction < tr->fraction) {
			trace.entityNum = ent->number;
			*tr = trace;
		} else if (trace.startsolid) {
			tr->startsolid = qtrue;
		}
		if ( tr->allsolid ) {
			return;
		}
	}
}


/*
==============================
from cgame/cg_players.c
==============================
*/
qboolean CG_GetTag( int clientNum, char *tagname, orientation_t *or ) {
	clientInfo_t	*ci;
	centity_t		*cent;
	refEntity_t		*refent;
	vec3_t			tempAxis[3];
	vec3_t			org;
	int				i;

	ci = &cgs.clientinfo[ clientNum ];

	if( cg.snap && clientNum == cg.snap->ps.clientNum && cg.renderingThirdPerson ) {
		cent = &cg.predictedPlayerEntity;
	} else {
		cent = &cg_entities[ci->clientNum];
		if (!cent->currentValid)
			return qfalse;		// not currently in PVS
	}

	refent = &cent->pe.bodyRefEnt;

	if( wrap_trap_R_LerpTag( or, refent, tagname, 0 ) < 0 )
		return qfalse;

	VectorCopy( refent->origin, org );

	for( i = 0 ; i < 3 ; i++ ) {
		VectorMA( org, or->origin[i], refent->axis[i], org );
	}

	VectorCopy( org, or->origin );

	// rotate with entity
	MatrixMultiply( refent->axis, or->axis, tempAxis );
	memcpy( or->axis, tempAxis, sizeof(vec3_t) * 3 );

	return qtrue;
}

/*
==============================
from cgame/cg_draw.c
==============================
*/

void wrap_CG_Text_PaintChar_Ext(float x, float y, float w, float h, float scalex, float scaley, float s, float t, float s2, float t2, qhandle_t hShader) {
	w *= scalex;
	h *= scaley;
	wrap_CG_AdjustFrom640( &x, &y, &w, &h );
	wrap_trap_R_DrawStretchPic( x, y, w, h, s, t, s2, t2, hShader );
}

void wrap_CG_Text_Paint_Ext(float x, float y, float scalex, float scaley, vec4_t color, const char *text, float adjust, int limit, int style, fontInfo_t* font) {
	int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;

	scalex *= font->glyphScale;
	scaley *= font->glyphScale;
	if (text) {
		const char *s = text;
		wrap_trap_R_SetColor( color );
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
		len = strlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			glyph = &font->glyphs[(unsigned char)*s];
			if ( Q_IsColorString( s ) ) {
				if( *(s+1) == COLOR_NULL ) {
					memcpy( newColor, color, sizeof(newColor) );
				} else {
					memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
					newColor[3] = color[3];
				}
				wrap_trap_R_SetColor( newColor );
				s += 2;
				continue;
			} else {
				float yadj = scaley * glyph->top;
				if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE) {
					int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
					colorBlack[3] = newColor[3];
					wrap_trap_R_SetColor( colorBlack );
					wrap_CG_Text_PaintChar_Ext(x + (glyph->pitch * scalex) + ofs, y - yadj + ofs, glyph->imageWidth, glyph->imageHeight, scalex, scaley, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);
					colorBlack[3] = 1.0;
					wrap_trap_R_SetColor( newColor );
				}
				wrap_CG_Text_PaintChar_Ext(x + (glyph->pitch * scalex), y - yadj, glyph->imageWidth, glyph->imageHeight, scalex, scaley, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);
				x += (glyph->xSkip * scalex) + adjust;
				s++;
				count++;
			}
		}
		wrap_trap_R_SetColor( NULL );
	}
}

/*
==============================
from cgame/cg_drawtools.c
==============================
*/
void wrap_CG_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	// scale for screen sizes
	*x *= cgs.screenXScale;
	*y *= cgs.screenYScale;
	*w *= cgs.screenXScale;
	*h *= cgs.screenYScale;
}

void wrap_CG_DrawChar( int x, int y, int width, int height, int ch ) {
	int row, col;
	float frow, fcol;
	float size;
	float	ax, ay, aw, ah;

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	ax = x;
	ay = y;
	aw = width;
	ah = height;
	wrap_CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	row = ch>>4;
	col = ch&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

	wrap_trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol + size,
			frow + size, cgs.media.charsetShader );
}

void wrap_CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader ) {
	float	s0;
	float	s1;
	float	t0;
	float	t1;

	if( width < 0 ) {	// flip about vertical
		width  = -width;
		s0 = 1;
		s1 = 0;
	}
	else {
		s0 = 0;
		s1 = 1;
	}

	if( height < 0 ) {	// flip about horizontal
		height= -height;
		t0 = 1;
		t1 = 0;
	}
	else {
		t0 = 0;
		t1 = 1;
	}
	
	wrap_CG_AdjustFrom640(&x, &y, &width, &height);
	wrap_trap_R_DrawStretchPic(x, y, width, height, s0, t0, s1, t1, hShader);
}

void wrap_CG_DrawSides_NoScale( float x, float y, float w, float h, float size ) {
	wrap_CG_AdjustFrom640( &x, &y, &w, &h );
	wrap_trap_R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
	wrap_trap_R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
}

void wrap_CG_DrawTopBottom_NoScale( float x, float y, float w, float h, float size ) {
	wrap_CG_AdjustFrom640( &x, &y, &w, &h );
	wrap_trap_R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
	wrap_trap_R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
}

void wrap_CG_DrawRect_FixedBorder(float x, float y, float width, float height, int border, const float *color) {
	wrap_trap_R_SetColor( color );
	wrap_CG_DrawTopBottom_NoScale( x, y, width, height, border );
	wrap_CG_DrawSides_NoScale( x, y, width, height, border );
	wrap_trap_R_SetColor( NULL );
}

void wrap_CG_DrawStringExt( int x, int y, const char *string, const float *setColor, 
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars ) {
	vec4_t		color;
	const char	*s;
	int			xx;
	int			cnt;

	if (maxChars <= 0)
		maxChars = 32767; // do them all!
	
	// draw the drop shadow
	if (shadow) {
		color[0] = color[1] = color[2] = 0;
		color[3] = setColor[3];
		wrap_trap_R_SetColor( color );
		s = string;
		xx = x;
		cnt = 0;
		while ( *s && cnt < maxChars) {
			if ( Q_IsColorString( s ) ) {
				s += 2;
				continue;
			}
			wrap_CG_DrawChar( xx + 1, y + 1, charWidth, charHeight, *s );
			cnt++;
			xx += charWidth;
			s++;
		}
	}

	// draw the colored text
	s = string;
	xx = x;
	cnt = 0;
	wrap_trap_R_SetColor( setColor );
	while ( *s && cnt < maxChars) {
		if ( Q_IsColorString( s ) ) {
			if ( !forceColor ) {
				if( *(s+1) == COLOR_NULL ) {
					memcpy( color, setColor, sizeof(color) );
				} else {
					memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
					color[3] = setColor[3];
				}
				wrap_trap_R_SetColor( color );
			}
			s += 2;
			continue;
		}
		wrap_CG_DrawChar( xx, y, charWidth, charHeight, *s );
		xx += charWidth;
		cnt++;
		s++;
	}
	wrap_trap_R_SetColor( NULL );
}

void wrap_CG_DrawInformation( qboolean forcerefresh ) {
	static int lastcalled = 0;

	if( lastcalled && (wrap_trap_Milliseconds() - lastcalled < 500) ) {
		return;
	}
	lastcalled = wrap_trap_Milliseconds();

	if( cg.snap ) {
		return;		// we are in the world, no need to draw information
	}

	//CG_DrawConnectScreen( qfalse, forcerefresh );

	// OSP - Server MOTD window
/*	if(cg.motdWindow == NULL) {
		CG_createMOTDWindow();
	}
	if(cg.motdWindow != NULL) {
		CG_windowDraw();
	}*/
	// OSP*/
}

void wrap_CG_FillRect( float x, float y, float width, float height, const float *color) {
    wrap_trap_R_SetColor( color );
    wrap_CG_AdjustFrom640( &x, &y, &width, &height );
    wrap_trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 1, cgs.media.whiteShader);
    wrap_trap_R_SetColor( NULL );
}

int wrap_CG_Text_Width_Ext(const char *text, float scale, int limit, fontInfo_t* font) {
	int count, len;
	glyphInfo_t *glyph;
	const char *s = text;
	float out, useScale = scale * font->glyphScale;
	
	out = 0;
	if( text ) {
		len = strlen( text );
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			if ( Q_IsColorString(s) ) {
				s += 2;
				continue;
			} else {
				glyph = &font->glyphs[(unsigned char)*s];
				out += glyph->xSkip;
				s++;
				count++;
			}
		}
	}

	return out * useScale;
}

/*
==============================
from cgame/cg_syscalls.c
==============================
*/
void wrap_trap_Cvar_Set(const char *var_name, const char *value) {
	orig_syscall(CG_CVAR_SET, var_name, value);
}

void wrap_trap_Cvar_Register(vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags) {
	orig_syscall(CG_CVAR_REGISTER, vmCvar, varName, defaultValue, flags);
}

void wrap_trap_Cvar_Update( vmCvar_t *vmCvar) {
	orig_syscall(CG_CVAR_UPDATE, vmCvar);
}

void wrap_trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize) {
	orig_syscall(CG_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, bufsize);
}

void wrap_trap_CG_SendConsoleCommand(const char *text) {
	orig_syscall(CG_SENDCONSOLECOMMAND, text);
}

int wrap_trap_Milliseconds(void) {
	return orig_syscall(CG_MILLISECONDS);
}

void wrap_trap_PumpEventLoop(void) {
	if( !cgs.initing )
		return;
	orig_syscall(CG_PUMPEVENTLOOP);
}

#define PASSFLOAT( x ) (*(int*)&x)
void wrap_trap_R_DrawStretchPic(float x, float y, float w, float h, 
							   float s1, float t1, float s2, float t2, qhandle_t hShader) {
	orig_syscall(CG_R_DRAWSTRETCHPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), hShader);
}

void wrap_trap_R_SetColor( const float *rgba ) {
	orig_syscall( CG_R_SETCOLOR, rgba );
}

void wrap_trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
                       const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask) {
	orig_syscall(CG_CM_BOXTRACE, results, start, end, mins, maxs, model, brushmask);
}

qboolean wrap_trap_Key_IsDown(int keynum) {
	return orig_syscall(CG_KEY_ISDOWN, keynum);
}

void wrap_trap_R_AddRefEntityToScene(const refEntity_t *re) {
	orig_syscall( CG_R_ADDREFENTITYTOSCENE, re );
}

qhandle_t wrap_trap_R_RegisterShaderNoMip(const char *name) {
	wrap_CG_DrawInformation(qtrue);
	wrap_trap_PumpEventLoop();
	return orig_syscall(CG_R_REGISTERSHADERNOMIP, name);
}

qboolean wrap_trap_R_LoadDynamicShader(const char *shadername, const char *shadertext) {
	return orig_syscall( CG_R_LOADDYNAMICSHADER, shadername, shadertext );
}

int wrap_trap_R_LerpTag(orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex) {
	return orig_syscall(CG_R_LERPTAG, tag, refent, tagName, startIndex);
}

sfxHandle_t	wrap_trap_S_RegisterSound( const char *sample, qboolean compressed ) {
	wrap_CG_DrawInformation(qtrue);
	wrap_trap_PumpEventLoop();
	return orig_syscall( CG_S_REGISTERSOUND, sample, qfalse /* compressed */ );
}

void wrap_trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx ) {
	orig_syscall( CG_S_STARTSOUND, origin, entityNum, entchannel, sfx, 127 /* Gordon: default volume always for the moment*/ );
}

void wrap_trap_S_StartLocalSound(sfxHandle_t sfx, int channelNum) {
	orig_syscall( CG_S_STARTLOCALSOUND, sfx, channelNum, 230 /* Gordon: default volume always for the moment*/ );
}

clipHandle_t wrap_trap_CM_InlineModel( int index ) {
	return orig_syscall(CG_CM_INLINEMODEL, index);
}

clipHandle_t wrap_trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs ) {
	return orig_syscall(CG_CM_TEMPBOXMODEL, mins, maxs);
}

void wrap_trap_CM_TransformedCapsuleTrace( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins,
		const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles) {
	orig_syscall(CG_CM_TRANSFORMEDCAPSULETRACE, results, start, end, mins, maxs, model, brushmask, origin, angles);
}

void wrap_trap_CM_TransformedBoxTrace(trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins,
		const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles) {
	orig_syscall(CG_CM_TRANSFORMEDBOXTRACE, results, start, end, mins, maxs, model, brushmask, origin, angles);
}
