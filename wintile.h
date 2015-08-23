#ifndef WINTILE_H
#define WINTILE_H

#include <iostream>

#include <string>
#include <map>
#include <list>
#include <vector>
#include <functional>
#include <memory>
#include <windows.h>

const unsigned int MODKEY    = VK_NONCONVERT;
const unsigned int SUBMODKEY = VK_LSHIFT;
const unsigned int WINDOW_WIDTH  = GetSystemMetrics(SM_CXSCREEN);
const unsigned int WINDOW_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

using stdfunc = std::function<void()>;

/* function declarations */
template<typename R, typename List> R& next_itr_cir(typename List::iterator&, List&);
template<typename R, typename List> R& prev_itr_cir(typename List::iterator&, List&);

stdfunc func_switcher(const stdfunc&, const stdfunc&);
stdfunc move_focus(const int);
stdfunc swap_window(const int);
stdfunc open_app(const wchar_t*);
void maximize();
void destroy_window();
void call_next_layout();
void call_prev_layout();
void quit();

void tileleft_impl();
void spiral_impl();

bool start_hook(HINSTANCE, HWND);
bool stop_hook();
void show_taskbar();
void hide_taskbar();
void create_window(HINSTANCE);
void get_all_window();

/* class declarations */
enum struct WindowState {
  NORMAL,
  ICON,
  MAXIMUM,
  FLOAT,
};

class Window final {
  public:
    Window(const HWND& handle, const WindowState& state) : _handle(handle), _state(state){}
    HWND getHandle() const;
    WindowState getState() const;
    RECT getRect() const;
    void setState(const WindowState&);
    void updateRect();

  private:
    HWND _handle;
    WindowState _state;
    RECT _rect;
};

class WindowList final {
  public:
    void init();
    HWND focused();
    HWND next();
    HWND prev();
    Window& focusedW();
    Window& frontW();
    Window& nextW();
    Window& prevW();
    void emplace_front(const HWND&, const WindowState&);
    void emplace_back(const HWND&, const WindowState&);
    void insert(const Window&);
    void erase();
    void swap(Window&, Window&);
    size_t length() const;

  private:
    size_t _length = 0;
    std::list<Window> _list;
    std::list<Window>::iterator _itr;
};

enum struct LayoutType {
  TILELEFT,
  SPIRAL
};

class Layout final {
  public:
    Layout(const LayoutType& name, const stdfunc& func) : _name(name), _func(func){}
    LayoutType getName();
    void arrange();

  private:
    LayoutType _name;
    stdfunc _func;
};

class LayoutList final {
  public:
    void init();
    Layout& focused();
    Layout& next();
    Layout& prev();
    void emplace_back(const LayoutType&, void (*)());

  private:
    std::vector<Layout> _list;
    std::vector<Layout>::iterator _itr;
};

/* variables */
HHOOK hookKey = 0;
HWND clientWnd;

std::unique_ptr<LayoutList> layoutList(new LayoutList);

std::unique_ptr<WindowList> showWndList(new WindowList);
std::unique_ptr<WindowList> hideWndList(new WindowList);

std::map<unsigned int, bool> isPressed = {
  { MODKEY,     false },
  { SUBMODKEY,  false }
};

wchar_t terminalPath[256] = L"\"C:/msys32/msys2_shell.bat\"";
wchar_t browserPath[256] = L"\"C:/Program Files/Mozilla Firefox/firefox.exe\"";
std::map<unsigned int, stdfunc> callFunc = {
  { 'J',        func_switcher( move_focus(1),         swap_window(1)         )},
  { 'K',        func_switcher( move_focus(-1),        swap_window(-1)        )},
  { 'D',        func_switcher( []{},                  destroy_window         )},
  { VK_RETURN,  func_switcher( []{},                  open_app(terminalPath) )},
  { VK_SPACE,   func_switcher( call_next_layout,      call_prev_layout       )},
  { 'A',                       swap_window(0)                                 },
  { 'I',                       open_app(browserPath)                          },
  { 'M',                       maximize                                       },
  { 'Q',                       quit                                           }
};

auto convertUTF16toUTF8 = [] (const wchar_t* wbuf, const unsigned int maxSize) -> std::string {
  int bufSize = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, nullptr, 0, nullptr, nullptr);
  char buf[maxSize];
  WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, bufSize, nullptr, nullptr);
  return std::string(buf);
};

auto moveWindow = [] (Window& window, const int x, const int y, const int w, const int h, const BOOL flag) {
  MoveWindow(window.getHandle(), x, y, w, h, flag);
  window.updateRect();
};

#endif /* WINTILE_H */

