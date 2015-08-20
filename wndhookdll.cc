#define UNICODE

#include <iostream>
#include <windows.h>

#include "wndhookdll.h"

HHOOK hkWnd __attribute__ ((section(".hook"))) = 0;

HINSTANCE hInst;
HWND clientWnd;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdReason, LPVOID lpReserved) {
  switch (fdReason) {
    case DLL_PROCESS_ATTACH:
      hInst = hInstDLL;
      break;
  }

  return TRUE;
}

LRESULT CALLBACK CallWndProc(int code, WPARAM wParam, LPARAM lParam) {
  std::cout << "CallWndProc" << std::endl;
  if (code < 0)
    return CallNextHookEx(hkWnd, code, wParam, lParam);
  else if (code == HC_ACTION)
    PostMessage(clientWnd, WM_APP, wParam, lParam);

  return CallNextHookEx(hkWnd, code, wParam, lParam);
}

LRESULT CALLBACK ShellProc(int code, WPARAM wParam, LPARAM lParam) {
  std::cout << "ShellProc" << std::endl;
  if (code < 0)
    return CallNextHookEx(hkWnd, code, wParam, lParam);
  else if (code == HC_ACTION)
    PostMessage(clientWnd, WM_APP, wParam, lParam);

  return CallNextHookEx(hkWnd, code, wParam, lParam);
}

DLLAPI bool start_wndproc_hook(HWND hWnd) {
  hkWnd = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, hInst, 0);
  clientWnd = hWnd;

  if (hkWnd == nullptr) {
    std::cout << "hkWnd is nullptr" << std::endl;
    return false;
  }

  return true;
}

DLLAPI bool stop_wndproc_hook() {
  if (UnhookWindowsHookEx(hkWnd) == 0) {
    std::cout << "Unhook hkWnd is failed" << std::endl;
    return false;
  }

  return true;
}

