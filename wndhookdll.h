#ifndef WNDHOOKDLL_H
#define WNDHOOKDLL_H

#define DLLAPI extern "C" __attribute__((visibility("default")))

#include <windows.h>

DLLAPI bool start_wndproc_hook(HWND);
DLLAPI bool stop_wndproc_hook();

#endif /* WNDHOOKDLL_H */

