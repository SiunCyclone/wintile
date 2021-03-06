#define UNICODE

#include <iostream>
#include <windows.h>

#include "wndhookdll.h"

HHOOK hookWnd __attribute__ ((section(".hook"))) = 0;
HHOOK hookShell __attribute__ ((section(".hook"))) = 0;

HINSTANCE hInst;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdReason, LPVOID lpReserved) {
  switch (fdReason) {
    case DLL_PROCESS_ATTACH:
      hInst = hInstDLL;
      break;
  }

  return TRUE;
}

LRESULT CALLBACK CallWndProc(int code, WPARAM wParam, LPARAM lParam) {
  if (code < 0)
    return CallNextHookEx(hookWnd, code, wParam, lParam);
  else if (code == HC_ACTION) {
    CWPSTRUCT* tmp = reinterpret_cast<CWPSTRUCT*>(lParam);
    UINT message = tmp->message;
    WPARAM wP = tmp->wParam;
    LPARAM lP = tmp->lParam;

    if (message == WM_CLOSE) {
      auto hwndTarget = FindWindow(L"WintileClass", NULL);
      if (hwndTarget != NULL)
        PostMessage(hwndTarget, WM_APP, wP, message);
    }

    if (message == WM_MOUSEACTIVATE) {
      auto hwndTarget = FindWindow(L"WintileClass", NULL);
      if (hwndTarget != NULL)
        PostMessage(hwndTarget, WM_MOUSEACTIVATE, wP, lP);
    }
  }

  return CallNextHookEx(hookWnd, code, wParam, lParam);
}

LRESULT CALLBACK ShellProc(int code, WPARAM wParam, LPARAM lParam) {
  if (code < 0)
    return CallNextHookEx(hookShell, code, wParam, lParam);
  else if (code == HSHELL_WINDOWCREATED) {
    auto hwndTarget = FindWindow(L"WintileClass", NULL);
    if (hwndTarget != NULL)
      PostMessage(hwndTarget, WM_APP, wParam, code);
  }

  return CallNextHookEx(hookShell, code, wParam, lParam);
}

DLLAPI bool start_wnd_hook() {
  hookWnd = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, hInst, 0);
  hookShell = SetWindowsHookEx(WH_SHELL, ShellProc, hInst, 0);

  if (hookWnd == NULL) {
    std::cout << "hookWnd is NULL" << std::endl;
    return false;
  }

  if (hookShell == NULL) {
    std::cout << "hookShell is NULL" << std::endl;
    return false;
  }

  return true;
}

DLLAPI bool stop_wnd_hook() {
  if (UnhookWindowsHookEx(hookWnd) == 0) {
    std::cout << "Failed to unhook hookWnd" << std::endl;
    return false;
  }

  if (UnhookWindowsHookEx(hookShell) == 0) {
    std::cout << "Failed to unhook hookShell" << std::endl;
    return false;
  }

  return true;
}

