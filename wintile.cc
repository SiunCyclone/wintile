#define UNICODE

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>
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
    auto handle = (dist == 1)  ? showWndList->next() :
                  (dist == -1) ? showWndList->prev() :
                                 showWndList->focused();
    SetForegroundWindow(handle);
    SetFocus(handle);
  };
};

stdfunc swap_window(const int dist) {
  return [=] {
    auto& a = showWndList->focusedW();
    auto& b = (dist == 1)  ? showWndList->nextW() :
              (dist == -1) ? showWndList->prevW() :
                             showWndList->frontW();
    auto aRect = a.getRect();
    auto bRect = b.getRect();

    moveWindow(a, bRect.left, bRect.top, bRect.right-bRect.left, bRect.bottom-bRect.top, TRUE);
    moveWindow(b, aRect.left, aRect.top, aRect.right-aRect.left, aRect.bottom-aRect.top, TRUE);

    showWndList->swap(a, b);
  };
}

stdfunc open_app(const wchar_t* path) {
  return [=] {
    ShellExecute(nullptr, L"open", path, nullptr, nullptr, SW_HIDE);
  };
}

void maximize() {
  static WindowState prevState;

  if (showWndList->focusedW().getState() == WindowState::MAXIMUM) {
    ShowWindow(showWndList->focused(), SW_RESTORE);
    showWndList->focusedW().setState(prevState);
  } else {
    prevState = showWndList->focusedW().getState();
    ShowWindow(showWndList->focused(), SW_MAXIMIZE);
    showWndList->focusedW().setState(WindowState::MAXIMUM);
  }
}

void destroy_window() {
  SendMessage(showWndList->focused(), WM_CLOSE, 0, 0);
}

void call_next_layout() {
  layoutList->next().arrange();
}

void call_prev_layout() {
  layoutList->prev().arrange();
}

void quit() {
  PostQuitMessage(0);
}

void tileleft_impl() {
  size_t length = showWndList->length();
  static auto width = WINDOW_WIDTH / 2;
  auto height = WINDOW_HEIGHT / (length>1 ? length-1 : 1);

  showWndList->init();
  moveWindow(showWndList->focusedW(), 0, 0, width, WINDOW_HEIGHT, TRUE);

  for (size_t i=1; i<length; ++i)
    moveWindow(showWndList->nextW(), width, (i-1)*height, width, height, TRUE);

  move_focus(1)();
}

void spiral_impl() {
  size_t length = showWndList->length();
  static auto mainWidth = WINDOW_WIDTH / 2;

  showWndList->init();
  moveWindow(showWndList->focusedW(), 0, 0, mainWidth, WINDOW_HEIGHT, TRUE);

  auto width = WINDOW_WIDTH / 2;
  auto height = WINDOW_HEIGHT;
  auto x = width;
  auto y = 0;

  for (size_t i=1; i<length; ++i) {
    if (i != length - 1)
      (i % 2 == 0) ? width /= 2 : height /= 2;

    moveWindow(showWndList->nextW(), x, y, width, height, TRUE);

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
        print(wParam);
        showWndList->emplace_front((HWND)wParam, WindowState::NORMAL);

        auto fromId = GetWindowThreadProcessId((HWND)wParam, nullptr);
        auto toId = GetCurrentThreadId();
        AttachThreadInput(fromId, toId, TRUE);
      } else if (lParam == WM_CLOSE)
        showWndList->erase();

      layoutList->focused().arrange();
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
    KBDLLHOOKSTRUCT* tmp = (KBDLLHOOKSTRUCT*)lParam;
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
      hideWndList->emplace_back(hWnd, WindowState::ICON);
    else
      showWndList->emplace_back(hWnd, WindowState::NORMAL);
  }

  return TRUE;
}

bool start_hook(HINSTANCE hInst) {
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

void create_window(HINSTANCE hInstance) {
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

  showWndList->init();
  for (size_t i=0; i<showWndList->length(); ++i) {
    auto handle = (i == 0) ? showWndList->focused() : showWndList->next();
    auto fromId = GetWindowThreadProcessId(handle, nullptr);
    auto toId = GetCurrentThreadId();
    AttachThreadInput(fromId, toId, TRUE);
  }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  MSG msg;

  layoutList->emplace_back( LayoutType::SPIRAL,    spiral_impl   );
  layoutList->emplace_back( LayoutType::TILELEFT,  tileleft_impl );
  layoutList->init();

  create_window(hInstance);
  hide_taskbar();
  get_all_window();
  layoutList->focused().arrange();
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

