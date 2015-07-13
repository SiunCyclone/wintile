#ifndef DLLAPI
#define DLLAPI __declspec(dllimport)
#endif

DLLAPI BOOL start_hook(HWND);
DLLAPI BOOL stop_hook(void);

