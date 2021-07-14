#include <metahook.h>
#include "exportfuncs.h"

cl_exportfuncs_t gExportfuncs;
mh_interface_t *g_pInterface;
metahook_api_t *g_pMetaHookAPI;
mh_enginesave_t *g_pMetaSave;
DWORD g_dwEngineBuildnum;
int g_iEngineType;

#define R_SCALE_COLOR "\x66\x0F\x6E\x4C\x24\x10\x8B\x4C\x24\x04\x0F\x5B\xC9\x66\x0F\x6E\x01\xF3\x0F\x5E\x0D\x2A\x2A\x2A\x2A\x0F\x5B\xC0\xF3\x0F\x59\xC1\xF3\x0F\x2C\xC0\x89\x01\x8B\x4C\x24\x08\x66\x0F\x6E\x01\x0F\x5B\xC0\xF3\x0F\x59\xC1\xF3\x0F\x2C\xC0\x89\x01\x8B\x4C\x24\x0C\x66\x0F\x6E\x01\x0F\x5B\xC0\xF3\x0F\x59\xC1\xF3\x0F\x2C\xC0\x89\x01\xC3"
void(*R_ScaleColor)(int& r, int& g, int& b, int a);

void IPlugins::Init(metahook_api_t *pAPI, mh_interface_t *pInterface, mh_enginesave_t *pSave)
{
	g_pInterface = pInterface;
	g_pMetaHookAPI = pAPI;
	g_pMetaSave = pSave;
}

void IPlugins::Shutdown(void)
{
}

void IPlugins::LoadEngine(void)
{
	g_iEngineType = g_pMetaHookAPI->GetEngineType();
	g_dwEngineBuildnum = g_pMetaHookAPI->GetEngineBuildnum();
}

void IPlugins::LoadClient(cl_exportfuncs_t *pExportFunc)
{
	if (g_iEngineType == ENGINE_SVENGINE)
	{
		PUCHAR ClientBase = (PUCHAR)GetModuleHandleA("client.dll");
		if (ClientBase)
		{
			auto ClientSize = g_pMetaHookAPI->GetModuleSize((HMODULE)ClientBase);
			DWORD addr = (DWORD)g_pMetaHookAPI->SearchPattern((void*)ClientBase, ClientSize, R_SCALE_COLOR, sizeof(R_SCALE_COLOR) - 1);
			R_ScaleColor = (decltype(R_ScaleColor))addr;
			Sig_FuncNotFound(R_ScaleColor);
		}
		else
			Sig_NotFound("client.dll");
	}
	else
		Sig_NotFound("Sven Co-op");

	g_pMetaHookAPI->InlineHook(R_ScaleColor, HookedColorScale, (void*&)R_ScaleColor);

	memcpy(&gExportfuncs, pExportFunc, sizeof(gExportfuncs));
	pExportFunc->Initialize = Initialize;
	pExportFunc->HUD_Init = HUD_Init;
}

void IPlugins::ExitGame(int iResult)
{
}

EXPOSE_SINGLE_INTERFACE(IPlugins, IPlugins, METAHOOK_PLUGIN_API_VERSION);