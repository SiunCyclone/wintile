#include <windows.h>
#include "hook.h"

void hide_taskbar();
void create_window(HINSTANCE);
void setup(HINSTANCE);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  CHAR str[128];

  switch (msg) {
    case WM_CREATE:
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_HOTKEY:
      PostQuitMessage(0);
      break;
    case WM_KEYDOWN:
      if (wParam != VK_RETURN) {
        wsprintf(str, "%x", wParam);
        MessageBox(nullptr, str, nullptr, MB_OK);
      }
      /*
      if (wParam == VK_NONCONVERT) {
        MessageBox(nullptr, TEXT("WM_KEYDOWN called"), nullptr, MB_OK);
      }
      */
      break;
    default:
      return DefWindowProc(hWnd, msg, wParam, lParam);
  }
  return 0;
}

void hide_taskbar() {
  HWND hTaskBar = FindWindow(TEXT("Shell_TrayWnd"), nullptr);
  HWND hStart = FindWindow(TEXT("Button"), nullptr);
  ShowWindow(hTaskBar, SW_HIDE);
  ShowWindow(hStart, SW_HIDE);
}

void show_taskbar() {
  HWND hTaskBar = FindWindow(TEXT("Shell_TrayWnd"), nullptr);
  HWND hStart = FindWindow(TEXT("Button"), nullptr);
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
  wcex.hIcon = nullptr;
  wcex.hCursor = nullptr;
  wcex.hbrBackground = nullptr;
  wcex.lpszMenuName = nullptr;
  wcex.lpszClassName = TEXT("wintile");
  wcex.hIconSm = nullptr;

  if (RegisterClassEx(&wcex) == 0)
    return;

  // Create window
  HWND hWtWnd = CreateWindowEx(0, TEXT("wintile"), TEXT("wintile"), 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance, nullptr);

  if (hWtWnd == nullptr)
    return;

  start_hook(hWtWnd);

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

  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  stop_hook();

  return msg.wParam;
}

