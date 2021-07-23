#include <metahook.h>
#include <cmath>
#include "cvardef.h"

void(*R_ScaleColor)(int* r, int* g, int* b, int a);
void(__fastcall* R_CalcDamageDirection)(void* pthis, int dummy, int x, float y, float z);
cl_enginefunc_t gEngfuncs;

typedef struct
{
	int  r;
	int  g;
	int  b;
}COLOR_RGB;
typedef struct
{
	float  h;
	float  s;
	float  v;
}COLOR_HSV;

cvar_t* pHUDCVarR = NULL;
cvar_t* pHUDCVarG = NULL;
cvar_t* pHUDCVarB= NULL;

cvar_t* pHUDCVarPainR = NULL;
cvar_t* pHUDCVarPainG = NULL;
cvar_t* pHUDCVarPainB = NULL;

cvar_t* pHUDCVarDizzy = NULL;
cvar_t* pHUDCVarDizzyTime = NULL;

bool bIsInFadeOut = false;
int iStepCounter = 0;
COLOR_RGB pNowColor = {0,0,0};
COLOR_HSV pNowHSVColor = { 0,0,0 };
COLOR_HSV pTargetHSVColor = { 0,0,0 };
float flNextFadeTime = 0;

#define FADE_INTERVAL 0.01
#define GetTotalStep(cvar) max(0, cvar->value / FADE_INTERVAL)

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
	pHUDCVarDizzyTime = gEngfuncs.pfnRegisterVariable("hud_color_dizzy_time", "0.5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);

	gExportfuncs.HUD_Init();
}
int HUD_Redraw(float time, int intermission)
{
	if (bIsInFadeOut && iStepCounter > 0 && flNextFadeTime <= time)
	{
		iStepCounter--;
		flNextFadeTime = time + FADE_INTERVAL;
		if (iStepCounter <= 0)
		{
			bIsInFadeOut = false;
			iStepCounter = 0;
		}
	}
	return gExportfuncs.HUD_Redraw(time, intermission);
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
int GetSafeColorCVar(cvar_t* cvar)
{
	if (cvar->value > 255 || cvar->value < 0)
		gEngfuncs.Cvar_SetValue(cvar->name, min(max(cvar->value, 0), 255));
	return cvar->value;
}
void __fastcall HookedCalcDamageDirection(void*pThis, int dummy, int x, float y, float z)
{
	if (pHUDCVarDizzy->value >= 1)
	{
		if (!bIsInFadeOut)
			bIsInFadeOut = true;
		iStepCounter = GetTotalStep(pHUDCVarDizzyTime);
		if (pNowColor.r != 250 && pNowColor.g != 0 && pNowColor.b != 0)
		{
			pNowColor.r = GetSafeColorCVar(pHUDCVarPainR);
			pNowColor.g = GetSafeColorCVar(pHUDCVarPainG);
			pNowColor.b = GetSafeColorCVar(pHUDCVarPainB);
		}
	}
	R_CalcDamageDirection(pThis, dummy, x, y, z);
}
//rgb2hsv and hsv2rgb
//https://github.com/alexkuhl/colorspace-conversion-library/blob/master/colorspace_conversion_library.hpp
void rgb_to_hsv(int r, int g, int b, float& h, float& s, float& v)
{
	float fr = r / 255.0, fg = g / 255.0, fb = b / 255.0;
	int imax = max(max(r, g), b);
	int imin = min(min(r, g), b);
	float fmax = imax / 255.0;
	float fmin = imin / 255.0;
	float multiplier = (imin == imax) ? 0.0 : 60 / (fmax - fmin);
	if (r == imax)
		h = fmod((multiplier * (fg - fb) + 360),360);
	else if (g == imax)
		h = multiplier * (fb - fr) + 120;
	else
		h = multiplier * (fr - fg) + 240;
	if (imax == 0)
		s = 0;
	else
		s = 1 - (fmin / fmax);
	v = fmax;
}
void hsv_to_rgb(float h, float s, float v, int& r, int& g, int& b)
{
	h /= 60;
	int hi = (int)h % 6;//大于360度情况
	float f = h - hi;
	int p = round(v * (1 - s) * 254);
	int q = round(v * (1 - f * s) * 254);
	int t = round(v * (1 - (1 - f) * s) * 254);
	int iv = round(v * 254);
	switch (hi)
	{
		case 0: r = iv; g = t; b = p;break;
		case 1: r = q; g = iv; b = p;break;
		case 2: r = p; g = iv; b = t;break;
		case 3: r = p; g = q; b = iv;break;
		case 4: r = t; g = p; b = iv;break;
		case 5: r = iv; g = p; b = q;break;
	}
}
void ForwardHSVColor()
{
	pNowHSVColor.h += pTargetHSVColor.h - pNowHSVColor.h * (1 - iStepCounter / GetTotalStep(pHUDCVarDizzyTime));
	pNowHSVColor.s += pTargetHSVColor.s - pNowHSVColor.s * (1 - iStepCounter / GetTotalStep(pHUDCVarDizzyTime));
	pNowHSVColor.v += pTargetHSVColor.v - pNowHSVColor.v * (1 - iStepCounter / GetTotalStep(pHUDCVarDizzyTime));
}
void ForwardRGBColor()
{
	pNowColor.r += pHUDCVarR->value - pNowColor.r * (1 - iStepCounter / GetTotalStep(pHUDCVarDizzyTime));
	pNowColor.g += pHUDCVarG->value - pNowColor.g * (1 - iStepCounter / GetTotalStep(pHUDCVarDizzyTime));
	pNowColor.b += pHUDCVarB->value - pNowColor.b * (1 - iStepCounter / GetTotalStep(pHUDCVarDizzyTime));
}
void HookedColorScale(int* r, int* g, int* b, int a)
{
	//正常状态
	if (*r == 100 && *g == 130 && *b == 200)
	{
		//挨打抖动
		if (bIsInFadeOut)
		{
			if (iStepCounter < GetTotalStep(pHUDCVarDizzyTime))
			{
				switch ((int)pHUDCVarDizzy->value)
				{
					case 0: break;
					case 2: {
						rgb_to_hsv(pNowColor.r, pNowColor.g, pNowColor.b, pNowHSVColor.h, pNowHSVColor.s, pNowHSVColor.v);
						rgb_to_hsv(GetSafeColorCVar(pHUDCVarR), GetSafeColorCVar(pHUDCVarG), GetSafeColorCVar(pHUDCVarB),
							pTargetHSVColor.h, pTargetHSVColor.s, pTargetHSVColor.v);
						ForwardHSVColor();
						//gEngfuncs.Con_Printf("H: %f S: %f V:%f\n", pNowHSVColor.h, pNowHSVColor.s, pNowHSVColor.v);
						hsv_to_rgb(pNowHSVColor.h, pNowHSVColor.s, pNowHSVColor.v, pNowColor.r, pNowColor.g, pNowColor.b);
						break;
					}
					default: {
						ForwardRGBColor();
						break;
					}
				}
			}
		}
		else 
		{
			pNowColor.r = GetSafeColorCVar(pHUDCVarR);
			pNowColor.g = GetSafeColorCVar(pHUDCVarG);
			pNowColor.b = GetSafeColorCVar(pHUDCVarB);
		}
		*r = pNowColor.r;
		*g = pNowColor.g;
		*b = pNowColor.b;
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