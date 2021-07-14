#include <metahook.h>
#include "cvardef.h"

void(*R_ScaleColor)(int* r, int* g, int* b, int a);
cl_enginefunc_t gEngfuncs;

cvar_t* pHUDCVarR = NULL;
cvar_t* pHUDCVarG = NULL;
cvar_t* pHUDCVarB= NULL;

cvar_t* pHUDCVarPainR = NULL;
cvar_t* pHUDCVarPainG = NULL;
cvar_t* pHUDCVarPainB = NULL;

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion)
{
	memcpy(&gEngfuncs, pEnginefuncs, sizeof(gEngfuncs));
	return gExportfuncs.Initialize(pEnginefuncs, iVersion);
}

void HUD_Init(void)
{
	pHUDCVarR = gEngfuncs.pfnRegisterVariable("hud_color_r", "100", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
	pHUDCVarG = gEngfuncs.pfnRegisterVariable("hud_color_g", "130", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
	pHUDCVarB = gEngfuncs.pfnRegisterVariable("hud_color_b", "200", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);

	pHUDCVarPainR = gEngfuncs.pfnRegisterVariable("hud_color_pain_r", "250", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
	pHUDCVarPainG = gEngfuncs.pfnRegisterVariable("hud_color_pain_g", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
	pHUDCVarPainB = gEngfuncs.pfnRegisterVariable("hud_color_pain_b", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);

	gExportfuncs.HUD_Init();
}

void Sys_ErrorEx(const char* fmt, ...)
{
	char msg[4096] = { 0 };

	va_list argptr;

	va_start(argptr, fmt);
	_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	if (gEngfuncs.pfnClientCmd)
		gEngfuncs.pfnClientCmd("escape\n");

	MessageBox(NULL, msg, "Fatal Error", MB_ICONERROR);
	TerminateProcess((HANDLE)(-1), 0);
}

int GetColorFromCVar(cvar_t* cvar, int* c)
{
	if (*c > 255 || *c < 0)
		gEngfuncs.Cvar_SetValue(cvar->name, min(max(cvar->value, 0), 255));
	return cvar->value;
}

void HookedColorScale(int* r, int* g, int* b, int a)
{
	//正常状态
	if (*r == 100 && *g == 130 && *b == 200)
	{
		*r = GetColorFromCVar(pHUDCVarR, r);
		*g = GetColorFromCVar(pHUDCVarG, g);
		*b = GetColorFromCVar(pHUDCVarB, b);
	}
	//濒死状态
	else if (*r == 250 && *g == 0 && *b == 0)
	{
		*r = GetColorFromCVar(pHUDCVarPainR, r);
		*g = GetColorFromCVar(pHUDCVarPainG, g);
		*b = GetColorFromCVar(pHUDCVarPainB, b);
	}
	//还有一个255 16 16

	R_ScaleColor(r, g, b, a);
}