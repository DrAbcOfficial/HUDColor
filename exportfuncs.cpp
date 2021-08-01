#include <metahook.h>
#include <cmath>
#include "cvardef.h"

void(*R_ScaleColor)(int* r, int* g, int* b, int a);
void(__fastcall* R_CalcDamageDirection)(void* pthis, int dummy, int x, float y, float z);
cl_enginefunc_t gEngfuncs;

typedef struct
{
	int  r = 0;
	int  g = 0;
	int  b = 0;

	float  h = 0;
	float  s = 0;
	float  v = 0;
}COLOR;

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
int iTotalStep = 0;
float flNextFadeTime = 0;
float flNowDizzyTime = 0;

COLOR pNowColor;
COLOR pTargetColor;

#define FADE_INTERVAL 0.01
#define clamp(n,a,b) min(max(n,a),b)
#define max3(a,b,c) max(a,max(b,c))
#define min3(a,b,c) min(a,min(b,c))

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
	if (bIsInFadeOut && flNextFadeTime <= time)
	{
		iStepCounter--;
		flNextFadeTime = time + FADE_INTERVAL;
		if (iStepCounter <= 0)
		{
			bIsInFadeOut = false;
			iStepCounter = 0;
		}
	}
	if (pHUDCVarDizzyTime->value != flNowDizzyTime)
		iTotalStep = max(0, pHUDCVarDizzyTime->value / FADE_INTERVAL);
	return gExportfuncs.HUD_Redraw(time, intermission);
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
		iStepCounter = iTotalStep;
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
	float flStep = iStepCounter / iTotalStep;
	pNowColor.h += pTargetColor.h - pNowColor.h * (1 - flStep);
	pNowColor.s += pTargetColor.s - pNowColor.s * (1 - flStep);
	pNowColor.v += pTargetColor.v - pNowColor.v * (1 - flStep);
}
void ForwardRGBColor()
{
	float flStep = iStepCounter / iTotalStep;
	pNowColor.r += pHUDCVarR->value - pNowColor.r * flStep;
	pNowColor.g += pHUDCVarG->value - pNowColor.g * flStep;
	pNowColor.b += pHUDCVarB->value - pNowColor.b * flStep;
}
void HookedColorScale(int* r, int* g, int* b, int a)
{
	//正常状态
	if (*r == 100 && *g == 130 && *b == 200)
	{
		//挨打抖动
		if (bIsInFadeOut)
		{
			if (iStepCounter < iTotalStep)
			{
				switch ((int)pHUDCVarDizzy->value)
				{
					case 0: break;
					case 2: {
						RGBToHSV(pNowColor.r, pNowColor.g, pNowColor.b, pNowColor.h, pNowColor.s, pNowColor.v);
						RGBToHSV(GetSafeColorCVar(pHUDCVarR), GetSafeColorCVar(pHUDCVarG), GetSafeColorCVar(pHUDCVarB),
							pTargetColor.h, pTargetColor.s, pTargetColor.v);
						ForwardHSVColor();
						//gEngfuncs.Con_Printf("H: %f S: %f V:%f\n", pNowHSVColor.h, pNowHSVColor.s, pNowHSVColor.v);
						HSVToRGB(pNowColor.h, pNowColor.s, pNowColor.v, pNowColor.r, pNowColor.g, pNowColor.b);
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