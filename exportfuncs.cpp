#include <metahook.h>
#include "cvardef.h"

void(*R_ScaleColor)(int* r, int* g, int* b, int a);
void(__fastcall* R_CalcDamageDirection)(void* pthis, int dummy, int x, float y, float z);
cl_enginefunc_t gEngfuncs;

cvar_t* pHUDCVarR = NULL;
cvar_t* pHUDCVarG = NULL;
cvar_t* pHUDCVarB= NULL;

cvar_t* pHUDCVarPainR = NULL;
cvar_t* pHUDCVarPainG = NULL;
cvar_t* pHUDCVarPainB = NULL;

cvar_t* pHUDCVarDizzy = NULL;
cvar_t* pHUDCVarDizzyStep = NULL;

bool bIsInFadeOut = false;
int iStepCounter = 0;
int iNowR = 0;
int iNowG = 0;
int iNowB = 0;

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

	pHUDCVarDizzy = gEngfuncs.pfnRegisterVariable("hud_color_dizzy", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
	pHUDCVarDizzyStep = gEngfuncs.pfnRegisterVariable("hud_color_dizzy_step", "8000", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);

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
int GetRGBStep(int& c, int cT)
{
	return c + (cT - c) / pHUDCVarDizzyStep->value * (max(0, pHUDCVarDizzyStep->value) - iStepCounter + 1);
}
int GetSafeColorCVar(cvar_t* cvar)
{
	if (cvar->value > 255 || cvar->value < 0)
		gEngfuncs.Cvar_SetValue(cvar->name, min(max(cvar->value, 0), 255));
	return cvar->value;
}
void __fastcall HookedCalcDamageDirection(void*pThis, int dummy, int x, float y, float z)
{
	if (!bIsInFadeOut)
		bIsInFadeOut = true;
	iStepCounter = max(0, pHUDCVarDizzyStep->value);
	if (iNowR != 250 && iNowG != 0 && iNowB != 0)
	{
		iNowR = GetSafeColorCVar(pHUDCVarPainR);
		iNowG = GetSafeColorCVar(pHUDCVarPainG);
		iNowB = GetSafeColorCVar(pHUDCVarPainB);
	}
	R_CalcDamageDirection(pThis, dummy, x, y, z);
}
void HookedColorScale(int* r, int* g, int* b, int a)
{
	//正常状态
	if (*r == 100 && *g == 130 && *b == 200)
	{
		//fadeout
		if (iStepCounter <= 0)
		{
			bIsInFadeOut = false;
			iStepCounter = 0;
		}
		//挨打抖动
		if (bIsInFadeOut)
		{
			if(iStepCounter > 0)
				iStepCounter--;
			if (iStepCounter < pHUDCVarDizzyStep->value)
			{
				iNowR = GetRGBStep(iNowR, GetSafeColorCVar(pHUDCVarR));
				iNowG = GetRGBStep(iNowG, GetSafeColorCVar(pHUDCVarG));
				iNowB = GetRGBStep(iNowB, GetSafeColorCVar(pHUDCVarB));
			}
		}
		else 
		{
			iNowR = GetSafeColorCVar(pHUDCVarR);
			iNowG = GetSafeColorCVar(pHUDCVarG);
			iNowB = GetSafeColorCVar(pHUDCVarB);
		}
		*r = iNowR;
		*g = iNowG;
		*b = iNowB;
	}
	//濒死状态
	else if (*r == 250 && *g == 0 && *b == 0)
	{
		*r = GetSafeColorCVar(pHUDCVarPainR);
		*g = GetSafeColorCVar(pHUDCVarPainG);
		*b = GetSafeColorCVar(pHUDCVarPainB);
	}
	//还有一个255 16 16手电筒暂时不替换
	R_ScaleColor(r, g, b, a);
}