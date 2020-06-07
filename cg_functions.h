#ifndef CG_FUNCTIONS_H_
#define CG_FUNCTIONS_H_

# import "eth.h"

// game/bg_misc.c
void wrap_BG_EvaluateTrajectory(const trajectory_t *tr, int atTime, vec3_t result, qboolean isAngle, int splinePath);
qboolean wrap_BG_TraverseSpline(float* deltaTime, splinePath_t** pSpline);
void wrap_BG_CalculateSpline_r(splinePath_t* spline, vec3_t out1, vec3_t out2, float tension);
splinePath_t* wrap_BG_GetSplineData(int number, qboolean* backwards);

// cgame/cg_draw.c
void wrap_CG_Text_Paint_Ext(float x, float y, float scalex, float scaley, vec4_t color, const char *text, float adjust, int limit, int style, fontInfo_t* font);

// cgame/cg_drawtools.c
void wrap_CG_AdjustFrom640(float *x, float *y, float *w, float *h);
void wrap_CG_DrawChar(int x, int y, int width, int height, int ch);
void wrap_CG_DrawRect_FixedBorder(float x, float y, float width, float height, int border, const float *color);
void wrap_CG_DrawStringExt(int x, int y, const char *string, const float *setColor, qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars);
void wrap_CG_DrawInformation(qboolean forcerefresh);
void wrap_CG_DrawPic(float x, float y, float width, float height, qhandle_t hShader);
void wrap_CG_FillRect(float x, float y, float width, float height, const float *color);
int wrap_CG_Text_Width_Ext(const char *text, float scale, int limit, fontInfo_t* font);

// cgame/cg_players.c
qboolean CG_GetTag(int clientNum, char *tagname, orientation_t *or);
void wrap_trap_Cvar_Set(const char *var_name, const char *value);

// cgame/cg_predict.c
void wrap_CG_BuildSolidList(void);
void wrap_CG_ClipMoveToEntities(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
		int skipNumber, int mask, int capsule, trace_t *tr);
void wrap_CG_Trace(trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, 
		int skipNumber, int mask);

// cgame/cg_syscalls.c
void wrap_trap_Cvar_Register(vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags);
void wrap_trap_Cvar_Update(vmCvar_t *vmCvar );
void wrap_trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize);
void wrap_trap_CG_SendConsoleCommand(const char *text);
int wrap_trap_Milliseconds(void);
void wrap_trap_R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2,
		qhandle_t hShader );
void wrap_trap_R_SetColor( const float *rgba );
void wrap_trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
		const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask);
qboolean wrap_trap_Key_IsDown(int keynum);
void wrap_trap_R_AddRefEntityToScene(const refEntity_t *re);
qhandle_t wrap_trap_R_RegisterShaderNoMip(const char *name);
qboolean wrap_trap_R_LoadDynamicShader(const char *shadername, const char *shadertext);
void wrap_trap_PumpEventLoop(void);
int wrap_trap_R_LerpTag(orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex);
sfxHandle_t	wrap_trap_S_RegisterSound( const char *sample, qboolean compressed );
void wrap_trap_S_StartSound(vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx);
void wrap_trap_S_StartLocalSound(sfxHandle_t sfx, int channelNum);
clipHandle_t wrap_trap_CM_InlineModel( int index );
clipHandle_t wrap_trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs );
void wrap_trap_CM_TransformedCapsuleTrace(trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins,
		const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles);
void wrap_trap_CM_TransformedBoxTrace(trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins,
		const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles);

#endif /*CG_FUNCTIONS_H_*/
