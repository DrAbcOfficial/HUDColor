#include <metahook.h>
#include "exportfuncs.h"
#include <APIProxy.h>

cl_exportfuncs_t gExportfuncs;
mh_interface_t *g_pInterface;
metahook_api_t *g_pMetaHookAPI;
mh_enginesave_t *g_pMetaSave;


HINSTANCE g_hInstance, g_hThisModule, g_hEngineModule;
PVOID g_dwEngineBase;
DWORD g_dwEngineSize;
DWORD g_dwEngineBuildnum;
int g_iEngineType;

#define R_SCALE_COLOR "\x66\x0F\x6E\x4C\x24\x10\x8B\x4C\x24\x04\x0F\x5B\xC9\x66\x0F\x6E\x01\xF3\x0F\x5E\x0D\x2A\x2A\x2A\x2A\x0F\x5B\xC0\xF3\x0F\x59\xC1\xF3\x0F\x2C\xC0\x89\x01\x8B\x4C\x24\x08\x66\x0F\x6E\x01\x0F\x5B\xC0\xF3\x0F\x59\xC1\xF3\x0F\x2C\xC0\x89\x01\x8B\x4C\x24\x0C\x66\x0F\x6E\x01\x0F\x5B\xC0\xF3\x0F\x59\xC1\xF3\x0F\x2C\xC0\x89\x01\xC3"
#define R_CALCDAMAGE_DIRECTION "\x55\x8B\xEC\x83\xE4\xF8\xF3\x0F\x10\x65\x08\x83\xEC\x44\xF3\x0F\x10\x55\x10\x0F\x57\xC0\xF3\x0F\x10\x5D\x0C"
void IPluginsV3::Init(metahook_api_t *pAPI, mh_interface_t *pInterface, mh_enginesave_t *pSave)
{
	g_pInterface = pInterface;
	g_pMetaHookAPI = pAPI;
	g_pMetaSave = pSave;
}

void IPluginsV3::Shutdown(void)
{
}

void IPluginsV3::LoadEngine(cl_enginefunc_t* pEngfuncs)
{
	g_iEngineType = g_pMetaHookAPI->GetEngineType();
	g_dwEngineBuildnum = g_pMetaHookAPI->GetEngineBuildnum();
	g_hEngineModule = g_pMetaHookAPI->GetEngineModule();
	g_dwEngineBase = g_pMetaHookAPI->GetEngineBase();
	g_dwEngineSize = g_pMetaHookAPI->GetEngineSize();

	memcpy(&gEngfuncs, pEngfuncs, sizeof(gEngfuncs));
}

void IPluginsV3::LoadClient(cl_exportfuncs_t* pExportFunc)
{
	memcpy(&gExportfuncs, pExportFunc, sizeof(gExportfuncs));

	if (g_iEngineType == ENGINE_SVENGINE)
	{
		PUCHAR ClientBase = (PUCHAR)GetModuleHandleA("client.dll");
		if (ClientBase)
		{
			auto ClientSize = g_pMetaHookAPI->GetModuleSize((HMODULE)ClientBase);
			DWORD addr = (DWORD)g_pMetaHookAPI->SearchPattern((void*)ClientBase, ClientSize, R_SCALE_COLOR, sizeof(R_SCALE_COLOR) - 1);
			Sig_AddrNotFound(R_ScaleColor);
			R_ScaleColor = (decltype(R_ScaleColor))addr;

			addr = (DWORD)g_pMetaHookAPI->SearchPattern((void*)ClientBase, ClientSize, R_CALCDAMAGE_DIRECTION, sizeof(R_CALCDAMAGE_DIRECTION) - 1);
			Sig_AddrNotFound(R_CalcDamageDirection);
			R_CalcDamageDirection = (decltype(R_CalcDamageDirection))addr;
		}
		else
			Sig_NotFound("client.dll");
	}
	else
		Sig_NotFound("Sven Co-op");
	
	g_pMetaHookAPI->InlineHook((void*)R_ScaleColor, HookedColorScale, (void**)&R_ScaleColor);
	g_pMetaHookAPI->InlineHook((void*)R_CalcDamageDirection, HookedCalcDamageDirection, (void**)&R_CalcDamageDirection);

	pExportFunc->Initialize = Initialize;
	pExportFunc->HUD_Init = HUD_Init;
	pExportFunc->HUD_Redraw = HUD_Redraw;
}

void IPluginsV3::ExitGame(int iResult)
{
}

EXPOSE_SINGLE_INTERFACE(IPluginsV3, IPluginsV3, METAHOOK_PLUGIN_API_VERSION_V3);