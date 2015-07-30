#define UNICODE

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <tuple>
#include <clocale>
#include <windows.h>

#define MODKEY    VK_NONCONVERT
#define SUBMODKEY VK_LSHIFT

using stdfunc = std::function<void()>;
using wndtype = std::tuple<HWND>;
using uintvec = std::vector<unsigned int>;

template <class T>
void print(T str) {
  std::wcout << str << std::endl;
}

/* function declarations */
HWND getHandle(wndtype);
bool start_hook(HINSTANCE, HWND);
bool stop_hook();
void show_taskbar();
void hide_taskbar();
void create_window(HINSTANCE);
void get_all_window();
stdfunc func_switcher(const stdfunc&, const stdfunc&);
stdfunc move_focus(const int);
stdfunc move_window(const int);
void maximize();
void close_window();
void quit();
stdfunc call_layout(const stdfunc&);
void tile_layout();
void spiral_layout();

/* variables */
HHOOK hhk;
HWND clientWnd;
static const unsigned int WINDOW_WIDTH = GetSystemMetrics(SM_CXSCREEN);
static const unsigned int WINDOW_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
std::vector<wndtype> onWndList;
std::vector<wndtype> offWndList;
int focusIndex;
std::string layout = "TILE";
static std::map<std::string, stdfunc> arrange = {
  { "TILE",    call_layout( tile_layout   )},
  { "SPIRAL",  call_layout( spiral_layout )}
};
static std::map<std::string, bool> isPressed = {
  { "MOD",     false },
  { "SUBMOD",  false }
};
static std::map<unsigned int, stdfunc> callFunc = {
  { 'J',  func_switcher( move_focus(1),   move_window(2)  )},
  { 'K',  func_switcher( move_focus(-1),  move_window(-2) )},
  { 'M',                 maximize                          },
  { 'D',  func_switcher( []{},            close_window    )},
  { 'Q',                 quit                              }
};

/* function implementations */
HWND getHandle(wndtype wnd) {
  return std::get<0>(wnd);
}

stdfunc func_switcher(const stdfunc& func, const stdfunc& sub_func) {
  return [=] {
    !isPressed["SUBMOD"] ? func() : sub_func();
  };
};

stdfunc move_focus(const int value) {
  return [=] {
    // XXX preserve other place
    int length = onWndList.size();
    focusIndex += value;
    if (focusIndex >= length)
      focusIndex = 0;
    else if (focusIndex < 0)
      focusIndex = length - 1;

    print(focusIndex);
    auto handle = getHandle(onWndList[focusIndex]);

    SetFocus(handle);
  };
};

stdfunc move_window(const int value) {
  return [=] {
    print("move_window");
    print(value);
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
    focusIndex = 0;
    func();
  };
}

void tile_layout() {
  auto length = onWndList.size();
  auto width = WINDOW_WIDTH / 2;
  auto height = WINDOW_HEIGHT / (length>1 ? length-1 : 1);

  MoveWindow(getHandle(onWndList[focusIndex]), 0, 0, width, WINDOW_HEIGHT, TRUE);

  for (unsigned int i=1; i<length; ++i)
    MoveWindow(getHandle(onWndList[i]), width, (i-1)*height, width, height, TRUE);
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

    static auto switch_flag = [&](const std::string name) {
      isPressed[name] = isKeyDown;
      return CallNextHookEx(hhk, code, wParam, lParam);
    };

    if (vkCode == MODKEY)
      switch_flag("MOD");
    else if (vkCode == SUBMODKEY)
      switch_flag("SUBMOD");

    if (isPressed["MOD"] && isKeyDown && callFunc.count(vkCode) == 1)
      PostMessage(clientWnd, WM_KEYDOWN, vkCode, lParam);
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
      offWndList.push_back(std::make_tuple(hWnd));
      return TRUE;
    }

    onWndList.push_back(std::make_tuple(hWnd));
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
  for (unsigned int i=0; i<onWndList.size(); ++i) {
    auto fromId = GetWindowThreadProcessId(getHandle(onWndList[i]), nullptr);
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

