#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <windows.h>

#define MODKEY VK_NONCONVERT

template <class T>
void print(T str) {
  std::cout << str << std::endl;
}

bool start_hook(HINSTANCE, HWND);
bool stop_hook();
void show_taskbar();
void hide_taskbar();
void create_window(HINSTANCE);
void setup(HINSTANCE);
std::function<void()> move_focus(int);
std::function<void()> move_window(int);
void quit();

HHOOK hhk;
HWND hClientWnd;
static std::map<std::string, bool> isPressed = {
  { "MOD",    false },
  { "SHIFT",  false },
  { "CTRL",   false },
  { "ALT",    false },
};
static std::map<unsigned int, std::function<void()>> callFunc = {
  {            'J',  move_focus(1)   },
  {            'K',  move_focus(-1)  },
  { VK_SHIFT + 'J',  move_window(1)  },
  { VK_SHIFT + 'K',  move_window(-1) },
  {            'Q',  quit            }
};

std::function<void()> move_focus(int value) {
  return [=] {
    std::cout << value << std::endl;
  };
};

std::function<void()> move_window(int value) {
  return [=] {
    std::cout << value << std::endl;
  };
}

void quit() {
  PostQuitMessage(0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_KEYDOWN: {
      std::cout << "called WM_KEYDOWN" << std::endl;
      callFunc[wParam]();
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
    bool isKeyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);

    static auto switch_flag = [&](std::string name) {
      isPressed[name] = isKeyDown;
      return CallNextHookEx(hhk, code, wParam, lParam);
    };

    if (vkCode == MODKEY)
      switch_flag("MOD");
    switch (vkCode) {
      case VK_SHIFT:
        switch_flag("SHIFT");
        break;
      case VK_CONTROL:
        switch_flag("CTRL");
        break;
      case VK_MENU:
        switch_flag("ALT");
        break;
      default:
        break;
    }

    if (isPressed["MOD"] && isKeyDown && callFunc.count(vkCode) == 1)
      PostMessage(hClientWnd, WM_KEYDOWN, vkCode, lParam);
  }

  return CallNextHookEx(hhk, code, wParam, lParam);
}

bool start_hook(HINSTANCE hInst, HWND hWnd) {
  hhk = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, hInst, 0);
  hClientWnd = hWnd;

  if (hhk == nullptr) {
    MessageBox(nullptr, TEXT("Error in start_hook() : hhk is nullptr"), nullptr, MB_OK);
    return FALSE;
  }

  return TRUE;
}

bool stop_hook() {
  if (UnhookWindowsHookEx(hhk) == 0) {
    MessageBox(nullptr, TEXT("Error in stop_hook()"), nullptr, MB_OK);
    return FALSE;
  }

  return TRUE;
}

void show_taskbar() {
  HWND hTaskBar = FindWindow(TEXT("Shell_TrayWnd"), nullptr);
  HWND hStart = FindWindow(TEXT("Button"), nullptr);
  ShowWindow(hTaskBar, SW_SHOW);
  ShowWindow(hStart, SW_SHOW);
}

void hide_taskbar() {
  HWND hTaskBar = FindWindow(TEXT("Shell_TrayWnd"), nullptr);
  HWND hStart = FindWindow(TEXT("Button"), nullptr);
  ShowWindow(hTaskBar, SW_HIDE);
  ShowWindow(hStart, SW_HIDE);
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
  show_taskbar();

  return msg.wParam;
}

