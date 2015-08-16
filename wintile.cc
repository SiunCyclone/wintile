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

RECT Window::getRect() const {
  return _rect;
}

void Window::setState(const WindowState& state) {
  _state = state;
}

void Window::setRect() {
  GetWindowRect(_handle, &_rect);
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

Window& WindowList::focusedW() {
  return *_itr;
}

Window& WindowList::frontW() {
  _itr = _list.begin();
  return _list.front();
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

void WindowList::swap(Window& a, Window& b) {
  auto tmp = a;
  a = b;
  b = tmp;
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
    auto handle = (dist == 1) ?  showWndList->next() :
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

stdfunc open_app(const char* path) {
  return [=] { system(path); };
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
  static auto width = WINDOW_WIDTH / 2;
  auto height = WINDOW_HEIGHT / (length>1 ? length-1 : 1);

  showWndList->init();
  moveWindow(showWndList->focusedW(), 0, 0, width, WINDOW_HEIGHT, TRUE);

  for (size_t i=1; i<length; ++i)
    moveWindow(showWndList->nextW(), width, (i-1)*height, width, height, TRUE);

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
      hideWndList->push_back(Window(hWnd, WindowState::ICON));
    else
      showWndList->push_back(Window(hWnd, WindowState::NORMAL));
  }

  return TRUE;
}

bool start_hook(HINSTANCE hInst) {
  hhk = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, hInst, 0);
  return (hhk == nullptr) ? false : true;
}

bool stop_hook() {
  return (UnhookWindowsHookEx(hhk) == 0) ? false : true;
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

