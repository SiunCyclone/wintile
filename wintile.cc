#define UNICODE

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <tuple>
#include <memory>
#include <clocale>
#include <windows.h>

#include "wintile.h"

template <class T>
void print(T str) {
  std::wcout << str << std::endl;
}

HWND Window::getHandle() const {
  return _handle;
}

WindowState Window::getState() const {
  return _state;
}

void Window::setState(const WindowState& state) {
  _state = state;
}

HWND WindowList::focused() {
  return _showWndList[_index].getHandle();
}

HWND WindowList::next(const WindowFlag& flag) {
  move_index(1, flag);
  return (flag == WindowFlag::SHOW) ? _showWndList[_index].getHandle() : _hideWndList[_index].getHandle();
}

HWND WindowList::prev(const WindowFlag& flag) {
  move_index(-1, flag);
  return (flag == WindowFlag::SHOW) ? _showWndList[_index].getHandle() : _hideWndList[_index].getHandle();
}

void WindowList::add(const Window& window) {
  if (window.getState() == WindowState::ICON) {
    _hideWndList.push_back(window);
    ++_hideLength;
  } else {
    _showWndList.push_back(window);
    ++_showLength;
  }
}

void WindowList::remove(const Window&) {

}

unsigned int WindowList::length(const WindowFlag& flag) {
  return (flag == WindowFlag::SHOW) ? _showLength : _hideLength;
}

void WindowList::move_index(const int dist, const WindowFlag& flag) {
  _index += dist;

  int length = (flag == WindowFlag::SHOW) ? _showLength : _hideLength;
  if (_index < 0)
    _index = length - 1;
  else if (_index >= length)
    _index = 0;
}

/* function implementations */
stdfunc func_switcher(const stdfunc& func, const stdfunc& sub_func) {
  return [=] {
    !isPressed[SUBMODKEY] ? func() : sub_func();
  };
};

stdfunc move_focus(const int dist) {
  return [=] {
    auto flag = WindowFlag::SHOW;
    auto handle = (dist == 1) ? wndList->next(flag) : wndList->prev(flag);
    SetForegroundWindow(handle);
    SetFocus(handle);
  };
};

stdfunc move_window(const int dist) {
  return [=] {
  };
}

void maximize() {

}

void close_window() {

}

void quit() {
  PostQuitMessage(0);
}

stdfunc call_layout(const stdfunc& func) {
  return [=] {
    func();
  };
}

void tile_layout() {
  auto length = wndList->length(WindowFlag::SHOW);
  auto width = WINDOW_WIDTH / 2;
  auto height = WINDOW_HEIGHT / (length>1 ? length-1 : 1);

  MoveWindow(wndList->focused(), 0, 0, width, WINDOW_HEIGHT, TRUE);

  for (unsigned int i=1; i<length; ++i)
    MoveWindow(wndList->next(WindowFlag::SHOW), width, (i-1)*height, width, height, TRUE);
}

void spiral_layout() {

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_KEYDOWN:
      callFunc[wParam]();
      break;
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

    if (vkCode == MODKEY || vkCode == SUBMODKEY) {
      isPressed[vkCode] = isKeyDown;
      return CallNextHookEx(hhk, code, wParam, lParam);
    }

    if (isPressed[MODKEY] && isKeyDown && callFunc.count(vkCode) == 1) {
      PostMessage(clientWnd, WM_KEYDOWN, vkCode, lParam);
      return 1;
    }
  }

  return CallNextHookEx(hhk, code, wParam, lParam);
}


BOOL CALLBACK EnumWndProc(HWND hWnd, LPARAM lParam) {
  if (IsWindowVisible(hWnd)) {
    static wchar_t buf[128];
    std::wstring str;

    GetWindowText(hWnd, buf, 128);
    str = buf;
    if (str == L"")
      return TRUE;

    std::wstring title = str;

    GetClassName(hWnd, buf, 128);
    str = buf;
    if (str == L"Progman" || str == L"MainWindowClass")
      return TRUE;

    print(title);
    print(hWnd);
    print("");

    if (IsIconic(hWnd)) {
      wndList->add(Window(hWnd, WindowState::ICON));
      return TRUE;
    }

    wndList->add(Window(hWnd, WindowState::NORMAL));
  }

  return TRUE;
}

bool start_hook(HINSTANCE hInst) {
  hhk = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, hInst, 0);

  if (hhk == nullptr) {
    print("Error in start_hook() : hhk is nullptr");
    return false;
  }

  return true;
}

bool stop_hook() {
  if (UnhookWindowsHookEx(hhk) == 0) {
    print("Error in stop_hook()");
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
}

void get_all_window() {
  EnumWindows(EnumWndProc, (LPARAM)nullptr);

  for (unsigned int i=0; i<wndList->length(WindowFlag::SHOW); ++i) {
    auto handle = (i == 0) ? wndList->focused() : wndList->next(WindowFlag::SHOW);
    auto fromId = GetWindowThreadProcessId(handle, nullptr);
    auto toId = GetCurrentThreadId();
    AttachThreadInput(fromId, toId, TRUE);
  }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  MSG msg;

  std::setlocale(LC_ALL, "");
  create_window(hInstance);
  hide_taskbar();
  get_all_window();
  arrange[layout]();
  start_hook(hInstance);

  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  stop_hook();
  //show_taskbar();

  return msg.wParam;
}

