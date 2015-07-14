#define DLLAPI extern "C" __attribute__((visibility("default")))

#include <windows.h>

DLLAPI BOOL start_hook(HWND);
DLLAPI BOOL stop_hook();

