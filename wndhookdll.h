#ifndef WNDHOOKDLL_H
#define WNDHOOKDLL_H

#define DLLAPI extern "C" __attribute__((visibility("default")))

#include <windows.h>

DLLAPI bool start_wnd_hook(HWND);
DLLAPI bool stop_wnd_hook();

#endif /* WNDHOOKDLL_H */

