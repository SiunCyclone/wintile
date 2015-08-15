#define UNICODE

#include <iostream>
#include <string>
#include <map>
#include <list>
#include <functional>
#include <tuple>
#include <memory>
#include <windows.h>

#include "wintile.h"

template <class T>
void print(T str) {
  std::cout << str << std::endl;
}

/* class implementations */
HWND Window::getHandle() const {
  return _handle;
}

WindowState Window::getState() const {
  return _state;
}

void Window::setState(const WindowState& state) {
  _state = state;
}

WindowRect Window::getRect() const {
  return _rect;
}

void Window::setRect(const WindowRect& rect) {
  _rect = rect;
}

void WindowList::init() {
  _itr = _list.begin();
}

HWND WindowList::focused() {
  return focusedW().getHandle();
}

HWND WindowList::next() {
  return nextW().getHandle();
}

HWND WindowList::prev() {
  return prevW().getHandle();
}

Window& WindowList::frontW() {
  return *_list.begin();
}

Window& WindowList::focusedW() {
  return *_itr;
}

Window& WindowList::nextW() {
  ++_itr;
  if (_itr == _list.end())
    _itr = _list.begin();
  return *_itr;
}

Window& WindowList::prevW() {
  if (_itr == _list.begin())
    _itr = _list.end();
  --_itr;
  return *_itr;
}

void WindowList::push_back(const Window& window) {
  _list.push_back(window);
  ++_length;
}

void WindowList::insert(const Window& window) {
  _itr = _list.insert(_itr, window);
  ++_length;
}

void WindowList::erase() {
  _itr = _list.erase(_itr);
  --_length;
}

size_t WindowList::length() const {
  return _length;
}

/* function implementations */
stdfunc func_switcher(const stdfunc& func, const stdfunc& sub_func) {
  return [=] {
    !isPressed[SUBMODKEY] ? func() : sub_func();
  };
};

stdfunc move_focus(const int dist) {
  return [=] {
    auto handle = (dist == 1) ? showWndList->next() : showWndList->prev();
    SetForegroundWindow(handle);
    SetFocus(handle);
  };
};

stdfunc move_window(const int dist) {
  return [=] {
    auto tmp = showWndList->focusedW();
    auto tmpRect = tmp.getRect();

    if (dist == 1) {
      showWndList->focusedW() = showWndList->nextW();
      showWndList->prevW().setRect(tmpRect);
    } else if (dist == -1) {
      showWndList->focusedW() = showWndList->prevW();
      showWndList->nextW().setRect(tmpRect);
    } else if (dist == 0) {
      showWndList->focusedW() = showWndList->frontW();
      showWndList->focusedW().setRect(tmpRect);
    }
    auto wnd = showWndList->focusedW();
    auto wndRect = wnd.getRect();
    MoveWindow(wnd.getHandle(), wndRect.x, wndRect.y, wndRect.w, wndRect.h, TRUE);

    if (dist == 1)
      tmp.setRect(showWndList->nextW().getRect());
    else if (dist == -1)
      tmp.setRect(showWndList->prevW().getRect());
    else if (dist == 0) {
      showWndList->init();
      tmp.setRect(showWndList->frontW().getRect());
    }
    (dist == 0) ? showWndList->frontW() = tmp : showWndList->focusedW() = tmp;
    tmpRect = tmp.getRect();
    MoveWindow(tmp.getHandle(), tmpRect.x, tmpRect.y, tmpRect.w, tmpRect.h, TRUE);
  };
}

stdfunc open_app(const char* path) {
  return [=] {
    system(path);
  };
}

void maximize() {

}

void destroy_window() {
  PostMessage(showWndList->focused(), WM_CLOSE, 0, 0);
  showWndList->erase();
  arrange[layout]();
}

void quit() {
  PostQuitMessage(0);
}

void tile_layout() {
  size_t length = showWndList->length();
  auto width = WINDOW_WIDTH / 2;
  auto height = WINDOW_HEIGHT / (length>1 ? length-1 : 1);

  showWndList->init();
  MoveWindow(showWndList->focused(), 0, 0, width, WINDOW_HEIGHT, TRUE);
  showWndList->focusedW().setRect(WindowRect(0, 0, width, WINDOW_HEIGHT));

  for (size_t i=1; i<length; ++i) {
    MoveWindow(showWndList->next(), width, (i-1)*height, width, height, TRUE);
    showWndList->focusedW().setRect(WindowRect(width, (i-1)*height, width, height));
  }

  move_focus(1)();
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
    static const unsigned int strMaxSize = 256;
    static wchar_t wbuf[strMaxSize];
    static char buf[strMaxSize];
    static std::string str;

    static auto isAddWnd = [=](stdfunc func) -> bool {
      static auto convertUTF16toUTF8 = [=] {
        int bufSize = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, nullptr, 0, nullptr, nullptr);
        WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, bufSize, nullptr, nullptr);
        str = buf;
      };

      func();
      convertUTF16toUTF8();
      if (str == "" || str == "Progman" || str == "MainWindowClass")
        return false;

      print(str);

      return true;
    };

    static auto windowText = [&] { GetWindowText(hWnd, wbuf, strMaxSize); };
    static auto className = [&] { GetClassName(hWnd, wbuf, strMaxSize); };
    if (!isAddWnd(windowText) || !isAddWnd(className))
      return TRUE;

    print(hWnd);
    print("");

    if (IsIconic(hWnd)) {
      hideWndList->push_back(Window(hWnd, WindowState::ICON));
      return TRUE;
    }

    showWndList->push_back(Window(hWnd, WindowState::NORMAL));
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

