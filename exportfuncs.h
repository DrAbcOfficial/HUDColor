extern cl_enginefunc_t gEngfuncs;

#define Sig_NotFound(name) Sys_ErrorEx("Could not found: %s\nEngine buildnum£º%d", #name, g_dwEngineBuildnum);
#define Sig_FuncNotFound(name) if(!name) Sig_NotFound(name)
#define Sig_AddrNotFound(name) if(!addr) Sig_NotFound(name)
#define Sig_Length(a) (sizeof(a)-1)
#define Search_Pattern(sig) g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, sig, Sig_Length(sig));

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion);
void HUD_Init(void);
void HookedColorScale(int* r, int* g, int* b, int a);
void Sys_ErrorEx(const char* fmt, ...);