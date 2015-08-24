#define UNICODE

#include <functional>
#include <iostream>
#include <string>
#include <windows.h>

#include "wintile.h"
#include "wndhookdll.h"

template <typename T>
void print(T str) {
  std::cout << str << std::endl;
}

/* function implementations */
stdfunc func_switcher(const stdfunc& func, const stdfunc& sub_func) {
  return [=] {
    !isPressed[SUBMODKEY] ? func() : sub_func();
  };
};

stdfunc move_focus(const int dist) {
  return [=] {
    auto handle = (dist == 1)  ? deskList->focused().getShowWnd()->next() :
                  (dist == -1) ? deskList->focused().getShowWnd()->prev() :
                                 deskList->focused().getShowWnd()->focused();
    SetForegroundWindow(handle);
    SetFocus(handle);
  };
};

stdfunc swap_window(const int dist) {
  return [=] {
    auto& a = deskList->focused().getShowWnd()->focusedW();
    auto& b = (dist == 1)  ? deskList->focused().getShowWnd()->nextW() :
              (dist == -1) ? deskList->focused().getShowWnd()->prevW() :
                             deskList->focused().getShowWnd()->frontW();
    auto aRect = a.getRect();
    auto bRect = b.getRect();

    moveWindow(a, bRect.left, bRect.top, bRect.right-bRect.left, bRect.bottom-bRect.top, TRUE);
    moveWindow(b, aRect.left, aRect.top, aRect.right-aRect.left, aRect.bottom-aRect.top, TRUE);

    auto tmp = a;
    a = b;
    b = tmp;
  };
}

stdfunc open_app(const wchar_t* path) {
  return [=] {
    ShellExecute(nullptr, L"open", path, nullptr, nullptr, SW_HIDE);
  };
}

void maximize() {
  static WindowState prevState;

  if (deskList->focused().getShowWnd()->focusedW().getState() == WindowState::MAXIMUM) {
    ShowWindow(deskList->focused().getShowWnd()->focused(), SW_RESTORE);
    deskList->focused().getShowWnd()->focusedW().setState(prevState);
  } else {
    prevState = deskList->focused().getShowWnd()->focusedW().getState();
    ShowWindow(deskList->focused().getShowWnd()->focused(), SW_MAXIMIZE);
    deskList->focused().getShowWnd()->focusedW().setState(WindowState::MAXIMUM);
  }
}

void destroy_window() {
  SendMessage(deskList->focused().getShowWnd()->focused(), WM_CLOSE, 0, 0);
}

void call_next_layout() {
  deskList->focused().getLayout()->next().arrange();
}

void call_prev_layout() {
  deskList->focused().getLayout()->prev().arrange();
}

void quit() {
  PostQuitMessage(0);
}

void tileleft_impl() {
  auto length = deskList->focused().getShowWnd()->length();
  static auto width = WINDOW_WIDTH / 2;
  auto height = WINDOW_HEIGHT / (length>1 ? length-1 : 1);

  deskList->focused().getShowWnd()->init();
  moveWindow(deskList->focused().getShowWnd()->focusedW(), 0, 0, width, WINDOW_HEIGHT, TRUE);

  for (auto i=1; i<length; ++i)
    moveWindow(deskList->focused().getShowWnd()->nextW(), width, (i-1)*height, width, height, TRUE);

  move_focus(1)();
}

void spiral_impl() {
  auto length = deskList->focused().getShowWnd()->length();
  static auto mainWidth = WINDOW_WIDTH / 2;

  deskList->focused().getShowWnd()->init();
  moveWindow(deskList->focused().getShowWnd()->focusedW(), 0, 0, mainWidth, WINDOW_HEIGHT, TRUE);

  auto width = WINDOW_WIDTH / 2;
  auto height = WINDOW_HEIGHT;
  auto x = width;
  auto y = 0;

  for (auto i=1; i<length; ++i) {
    if (i != length - 1)
      (i % 2 == 0) ? width /= 2 : height /= 2;

    moveWindow(deskList->focused().getShowWnd()->nextW(), x, y, width, height, TRUE);

    (i % 2 == 0) ? x += width : y += height;
  }

  move_focus(1)();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_KEYDOWN:
      callFunc[wParam]();
      break;
    case WM_APP: {
      if (lParam == HSHELL_WINDOWCREATED) {
        HWND hWnd = reinterpret_cast<HWND>(wParam);

        print(wParam);
        deskList->focused().getShowWnd()->emplace_front(hWnd, WindowState::NORMAL);

        auto fromId = GetWindowThreadProcessId(hWnd, nullptr);
        auto toId = GetCurrentThreadId();
        AttachThreadInput(fromId, toId, TRUE);
      } else if (lParam == WM_CLOSE)
        deskList->focused().getShowWnd()->erase();

      deskList->focused().getLayout()->focused().arrange();
      break;
    }
    default:
      return DefWindowProc(hWnd, msg, wParam, lParam);
  }

  return 0;
}

LRESULT CALLBACK LLKeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
  if (code < 0)
    return CallNextHookEx(hookKey, code, wParam, lParam);
  else if (code == HC_ACTION) {
    KBDLLHOOKSTRUCT* tmp = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    DWORD vkCode = tmp->vkCode;
    bool isKeyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);

    if (vkCode == MODKEY || vkCode == SUBMODKEY) {
      isPressed[vkCode] = isKeyDown;
      return CallNextHookEx(hookKey, code, wParam, lParam);
    }

    if (isPressed[MODKEY] && isKeyDown && callFunc.count(vkCode) == 1) {
      PostMessage(clientWnd, WM_KEYDOWN, vkCode, lParam);
      return 1;
    }
  }

  return CallNextHookEx(hookKey, code, wParam, lParam);
}

BOOL CALLBACK EnumWndProc(HWND hWnd, LPARAM lParam) {
  if (IsWindowVisible(hWnd)) {
    static const unsigned int strMaxSize = 256;
    wchar_t wbuf[strMaxSize];
    std::string str;

    GetWindowText(hWnd, wbuf, strMaxSize);
    str = convertUTF16toUTF8(wbuf, strMaxSize);
    if (str == "")
      return TRUE;

    print(str);

    GetClassName(hWnd, wbuf, strMaxSize);
    str = convertUTF16toUTF8(wbuf, strMaxSize);
    if (str == "Progman" || str == "MainWindowClass")
      return TRUE;

    print(hWnd);
    print("");

    if (IsIconic(hWnd))
      deskList->focused().getHideWnd()->emplace_back(hWnd, WindowState::ICON);
    else
      deskList->focused().getShowWnd()->emplace_back(hWnd, WindowState::NORMAL);
  }

  return TRUE;
}

bool start_hook(const HINSTANCE hInst) {
  hookKey = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, hInst, 0);

  if (hookKey == nullptr) {
    print("hookKey is nullptr");
    return false;
  }

  return true;
}

bool stop_hook() {
  if (UnhookWindowsHookEx(hookKey) == 0) {
    print("Unhook hookKey is failed");
    return false;
  }

  return true;
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

void create_window(const HINSTANCE hInstance) {
  WNDCLASSEX wcex;
  auto className = TEXT("WintileClass");

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
  wcex.lpszClassName = className;
  wcex.hIconSm = nullptr;

  if (RegisterClassEx(&wcex) == 0)
    return;

  clientWnd = CreateWindowEx(0, className, TEXT("wintile"), 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance, nullptr);

  if (clientWnd == nullptr)
    return;

  start_wnd_hook(clientWnd);
}

void get_all_window() {
  EnumWindows(EnumWndProc, (LPARAM)nullptr);

  deskList->focused().getShowWnd()->init();
  for (auto i=0; i<deskList->focused().getShowWnd()->length(); ++i) {
    auto handle = (i == 0) ? deskList->focused().getShowWnd()->focused() : deskList->focused().getShowWnd()->next();
    auto fromId = GetWindowThreadProcessId(handle, nullptr);
    auto toId = GetCurrentThreadId();
    AttachThreadInput(fromId, toId, TRUE);
  }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  MSG msg;

  create_window(hInstance);
  hide_taskbar();
  get_all_window();
  deskList->focused().getLayout()->focused().arrange();
  start_hook(hInstance);

  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  stop_hook();
  stop_wnd_hook();

  //show_taskbar();

  return msg.wParam;
}

