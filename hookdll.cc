#include <windows.h>
#include "hook.h"

__attribute__ ((section(".hookseg"))) HHOOK hhk = 0;

HINSTANCE hInst;
HWND hClientWnd;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpReserved) {
  switch (dwReason) {
    case DLL_PROCESS_ATTACH:
      hInst = hInstDLL;
      break;
    case DLL_PROCESS_DETACH:
      break;
  }

  return TRUE;
}

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
  if (code < 0)
    return CallNextHookEx(hhk, code, wParam, lParam);

  if (code == HC_ACTION)
    PostMessage(hClientWnd, WM_KEYDOWN, wParam, lParam);

  return CallNextHookEx(hhk, code, wParam, lParam);
}

DLLAPI BOOL start_hook(HWND hWnd) {
  hhk = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, hInst, 0);
  hClientWnd = hWnd;

  if (hhk == nullptr) {
    MessageBox(nullptr, TEXT("Error in start_hook() : hhk is nullptr"), nullptr, MB_OK);
    return FALSE;
  }

  MessageBox(nullptr, TEXT("start_hook"), nullptr, MB_OK);
  return TRUE;
}

DLLAPI BOOL stop_hook() {
  if (UnhookWindowsHookEx(hhk) == 0) {
    MessageBox(nullptr, TEXT("Error in stop_hook()"), nullptr, MB_OK);
    return FALSE;
  }

  return TRUE;
}

