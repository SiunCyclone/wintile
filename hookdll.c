#include <windows.h>
#define DLLAPI __declspec(dllexport)

#pragma data_seg("SHARED_SEG")
HHOOK hKeyHook = 0;
#pragma data_seg()

HINSTANCE hInst;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved) {
  switch (dwReason) {

  }

  return TRUE;
}

LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode < 0)
    return CallNextHookEx(hKeyHook, nCode, wParam, lParam);

  return CallNextHookEx(hKeyHook, nCode, wParam, lParam);
}

DLLAPI int start_hook(HWND hWnd) {
  hKeyHook = SetWindowsHookEx(WH_KEYBOARD, KeyHookProc, hInst, 0);

  if (hKeyHook == NULL)
    return FALSE;

  return TRUE;
}

DLLAPI int stop_hook(void) {
  if (UnhookWindowsHookEx(hKeyHook) == 0)
    return FALSE; // Error to unhook

  return TRUE;
}

