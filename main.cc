#include <iostream>
#include <windows.h>

HHOOK hhk;
HWND hClientWnd;
BOOL modIsPressed = false;

void hide_taskbar();
void create_window(HINSTANCE);
void setup(HINSTANCE);
BOOL start_hook(HINSTANCE, HWND);
BOOL stop_hook();

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
    case WM_KEYDOWN: {
      std::cout << wParam << std::endl;

      break;
    }
    default:
      return DefWindowProc(hWnd, msg, wParam, lParam);
  }
  return 0;
}

LRESULT CALLBACK LLKeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
  if (code < 0)
    return CallNextHookEx(hhk, code, wParam, lParam);

  if (code == HC_ACTION) {
    KBDLLHOOKSTRUCT* tmp = (KBDLLHOOKSTRUCT*)lParam;
    DWORD vkCode = tmp->vkCode;

    BOOL isKeyDown = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;

    if (vkCode == VK_NONCONVERT) {
      modIsPressed = isKeyDown;
      return CallNextHookEx(hhk, code, wParam, lParam);
    } else if (modIsPressed && isKeyDown)
      PostMessage(hClientWnd, WM_KEYDOWN, vkCode, lParam);
  }

  return CallNextHookEx(hhk, code, wParam, lParam);
}

BOOL start_hook(HINSTANCE hInst, HWND hWnd) {
  hhk = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, hInst, 0);
  hClientWnd = hWnd;

  if (hhk == nullptr) {
    MessageBox(nullptr, TEXT("Error in start_hook() : hhk is nullptr"), nullptr, MB_OK);
    return FALSE;
  }

  return TRUE;
}

BOOL stop_hook() {
  if (UnhookWindowsHookEx(hhk) == 0) {
    MessageBox(nullptr, TEXT("Error in stop_hook()"), nullptr, MB_OK);
    return FALSE;
  }

  return TRUE;
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

  HWND hWtWnd = CreateWindowEx(0, TEXT("wintile"), TEXT("wintile"), 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance, nullptr);

  if (hWtWnd == nullptr)
    return;

  start_hook(hInstance, hWtWnd);

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

