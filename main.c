#include <windows.h>
#include "hook.h"

void hide_taskbar(void);
void create_window(HINSTANCE);
void setup(HINSTANCE);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_CREATE:
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_HOTKEY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hWnd, msg, wParam, lParam);
  }
  return 0;
}

void hide_taskbar(void) {
  HWND hTaskBar = FindWindow(TEXT("Shell_TrayWnd"), NULL);
  HWND hStart = FindWindow(TEXT("Button"), NULL);
  ShowWindow(hTaskBar, SW_HIDE);
  ShowWindow(hStart, SW_HIDE);
}

void show_taskbar(void) {
  HWND hTaskBar = FindWindow(TEXT("Shell_TrayWnd"), NULL);
  HWND hStart = FindWindow(TEXT("Button"), NULL);
  ShowWindow(hTaskBar, SW_SHOW);
  ShowWindow(hStart, SW_SHOW);
}

void create_window(HINSTANCE hInstance) {
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = 0;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = NULL;
  wcex.hCursor = NULL;
  wcex.hbrBackground = NULL;
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = TEXT("wintile");
  wcex.hIconSm = NULL;

  if (RegisterClassEx(&wcex) == 0)
    return;

  // Create window
  HWND hWtWnd = CreateWindowEx(0, TEXT("wintile"), TEXT("wintile"), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

  if (hWtWnd == NULL)
    return;

  RegisterHotKey(hWtWnd, 0, MOD_CONTROL, 'Q');
}

void setup(HINSTANCE hInstance) {
  hide_taskbar();
  create_window(hInstance);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  MSG msg;

  //setup(hInstance);
  create_window(hInstance);

  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return msg.wParam;
}

