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
#define clamp(n,a,b) min(max(n,a),b)
#define max3(a,b,c) max(a,max(b,c))
#define min3(a,b,c) min(a,min(b,c))

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
	if (pHUDCVarDizzy->value > 0)
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
void RGBToHSV(int r, int g, int b, float& h, float& s, float& v)
{
	float fr = r / 255.0, fg = g / 255.0, fb = b / 255.0;
	float max = max3(fr, fg, fb);
	float min = min3(fr, fg, fb);
	float range = max - min;
	//H
	if (range <= 0)
		h = 0;
	else if (max == r)
		h = 60 * (fg - fb) / range + (g >= b ? 0 : 360);
	else if (max == g)
		h = 60 * (fb - fr) / range + 120;
	else
		h = 60 * (fr - fg) / range + 240;
	if (abs(h) >= 360)
		h = fmod(h, 360);
	//S
	s = max <= 0 ? 0 : range / max;
	//V
	v = max <= 0 ? 0 : max;
}
void HSVToRGB(float h, float s, float v, int& r, int& g, int& b)
{
	//0<=h<360
	//0<=s<=1
	//0<=v<=1
	h = fmod(h, 360);
	s = clamp(s, 0, 1);
	v = clamp(v, 0, 1);
	float section = h / 60;
	float c = v * s;
	float x = c * (1 - abs(fmod(section, 2) - 1));
	float hr = 0, hg = 0, hb = 0;
	switch ((int)section) {
		case 0:hr = c, hg = x, hb = 0; break;
		case 1:hr = x; hg = c; hb = 0; break;
		case 2:hr = 0; hg = c; hb = x; break;
		case 3:hr = 0; hg = x; hb = c; break;
		case 4:hr = x; hg = 0; hb = c; break;
		case 5:hr = c; hg = 0; hb = x; break;
	}
	float m = v - c;
	r = (hr + m) * 255;
	g = (hg + m) * 255;
	b = (hb + m) * 255;
}
void ForwardHSVColor()
{
	pNowHSVColor.h += pTargetHSVColor.h - pNowHSVColor.h * (1 - iStepCounter / GetTotalStep(pHUDCVarDizzyTime));
	pNowHSVColor.s += pTargetHSVColor.s - pNowHSVColor.s * (1 - iStepCounter / GetTotalStep(pHUDCVarDizzyTime));
	pNowHSVColor.v += pTargetHSVColor.v - pNowHSVColor.v * (1 - iStepCounter / GetTotalStep(pHUDCVarDizzyTime));
}
void ForwardRGBColor()
{
	pNowColor.r += pHUDCVarR->value - pNowColor.r * (iStepCounter / GetTotalStep(pHUDCVarDizzyTime));
	pNowColor.g += pHUDCVarG->value - pNowColor.g * (iStepCounter / GetTotalStep(pHUDCVarDizzyTime));
	pNowColor.b += pHUDCVarB->value - pNowColor.b * (iStepCounter / GetTotalStep(pHUDCVarDizzyTime));
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
						RGBToHSV(pNowColor.r, pNowColor.g, pNowColor.b, pNowHSVColor.h, pNowHSVColor.s, pNowHSVColor.v);
						RGBToHSV(GetSafeColorCVar(pHUDCVarR), GetSafeColorCVar(pHUDCVarG), GetSafeColorCVar(pHUDCVarB),
							pTargetHSVColor.h, pTargetHSVColor.s, pTargetHSVColor.v);
						ForwardHSVColor();
						//gEngfuncs.Con_Printf("H: %f S: %f V:%f\n", pNowHSVColor.h, pNowHSVColor.s, pNowHSVColor.v);
						HSVToRGB(pNowHSVColor.h, pNowHSVColor.s, pNowHSVColor.v, pNowColor.r, pNowColor.g, pNowColor.b);
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