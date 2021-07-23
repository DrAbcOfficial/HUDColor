extern cl_enginefunc_t gEngfuncs;
extern void(*R_ScaleColor)(int* r, int* g, int* b, int a);
extern void(__fastcall* R_CalcDamageDirection)(void* pthis, int dummy, int x, float y, float z);

#define Sig_NotFound(name) Sys_ErrorEx("Could not found: %s\nEngine buildnum£º%d", #name, g_dwEngineBuildnum);
#define Sig_AddrNotFound(name) if(!addr) Sig_NotFound(name)
#define Sig_Length(a) (sizeof(a)-1)

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion);
void HUD_Init(void);
int HUD_Redraw(float time, int intermission);
void HookedColorScale(int* r, int* g, int* b, int a);
void __fastcall HookedCalcDamageDirection(void* pThis, int dummy, int x, float y, float z);
void Sys_ErrorEx(const char* fmt, ...);