#include <windows.h>
#define DLLAPI __declspec(dllexport)

#pragma data_seg(".sharedseg")
HHOOK hhk = 0;
#pragma data_seg()

HINSTANCE hInst;

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpReserved) {
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

  return CallNextHookEx(hhk, code, wParam, lParam);
}

DLLAPI BOOL start_hook(HWND hWnd) {
  hhk = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, hInst, 0);

  if (hhk == NULL)
    return FALSE;

  return TRUE;
}

DLLAPI BOOL stop_hook(void) {
  if (UnhookWindowsHookEx(hhk) == 0)
    return FALSE;

  return TRUE;
}

