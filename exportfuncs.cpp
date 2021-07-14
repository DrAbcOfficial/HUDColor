#include <metahook.h>
#include "cvardef.h"

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

void HookedColorScale(int* r, int* g, int* b, int a)
{
	if (*r == 100 && *g == 130 && *b == 200)
	{
		if(pHUDCVarR->value > 255)
			gEngfuncs.Cvar_SetValue("hud_color_r", 255);
		if (pHUDCVarR->value < 0)
			gEngfuncs.Cvar_SetValue("hud_color_r", 0);
		*r = pHUDCVarR->value;

		if (pHUDCVarG->value > 255)
			gEngfuncs.Cvar_SetValue("hud_color_g", 255);
		if (pHUDCVarG->value < 0)
			gEngfuncs.Cvar_SetValue("hud_color_g", 0);
		*g = pHUDCVarG->value;

		if (pHUDCVarB->value > 255)
			gEngfuncs.Cvar_SetValue("hud_color_b", 255);
		if (pHUDCVarB->value < 0)
			gEngfuncs.Cvar_SetValue("hud_color_b", 0);
		*b = pHUDCVarB->value;
	}
	else if (*r == 250 && *g == 0 && *b == 0)
	{
		if (pHUDCVarPainR->value > 255)
			gEngfuncs.Cvar_SetValue("hud_color_pain_r", 255);
		if (pHUDCVarPainR->value < 0)
			gEngfuncs.Cvar_SetValue("hud_color_r", 0);
		*r = pHUDCVarPainR->value;

		if (pHUDCVarPainG->value > 255)
			gEngfuncs.Cvar_SetValue("hud_color_pain_g", 255);
		if (pHUDCVarPainG->value < 0)
			gEngfuncs.Cvar_SetValue("hud_color_pain_g", 0);
		*g = pHUDCVarPainG->value;

		if (pHUDCVarPainB->value > 255)
			gEngfuncs.Cvar_SetValue("hud_color_pain_b", 255);
		if (pHUDCVarPainB->value < 0)
			gEngfuncs.Cvar_SetValue("hud_color_pain_b", 0);
		*b = pHUDCVarPainB->value;
	}

	float x = (float)a / 255;
	*r = (int)(*r * x);
	*g = (int)(*g * x);
	*b = (int)(*b * x);
}