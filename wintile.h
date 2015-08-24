#ifndef WINTILE_H
#define WINTILE_H

#include <array>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <windows.h>

constexpr auto MODKEY    = VK_NONCONVERT;
constexpr auto SUBMODKEY = VK_LSHIFT;
auto WINDOW_WIDTH  = GetSystemMetrics(SM_CXSCREEN);
auto WINDOW_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

using stdfunc = std::function<void()>;

/* function declarations */
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

bool start_hook(const HINSTANCE);
bool stop_hook();
void show_taskbar();
void hide_taskbar();
void create_window(const HINSTANCE);
void get_all_window();

/* template implementations */
template<typename R, typename Itr, typename List>
R& next_itr_cir(Itr& itr, List& list) {
  ++itr;
  if (itr == list.end())
    itr = list.begin();
  return *itr;
}

template<typename R, typename Itr, typename List>
R& prev_itr_cir(Itr& itr, List& list) {
  if (itr == list.begin())
    itr = list.end();
  --itr;
  return *itr;
}

/* class implementations */
enum struct WindowState {
  NORMAL,
  ICON,
  MAXIMUM,
  FLOAT,
};

class Window final {
  public:
    Window(const HWND& handle, const WindowState& state) : _handle(handle),
                                                           _state(state) {}
    HWND getHandle()                  const { return _handle;                 }
    WindowState getState()            const { return _state;                  }
    RECT getRect()                    const { return _rect;                   }
    void setState(const WindowState& state) { _state = state;                 }
    void updateRect()                       { GetWindowRect(_handle, &_rect); }

  private:
    HWND _handle;
    WindowState _state;
    RECT _rect;
};

class WindowList final {
  public:
    void init()           { _itr = _list.begin();                     }
    size_t length() const { return _length;                           }
    HWND focused()        { return focusedW().getHandle();            }
    HWND next()           { return nextW().getHandle();               }
    HWND prev()           { return prevW().getHandle();               }
    Window& focusedW()    { return *_itr;                             }
    Window& frontW()      { return *(_itr = _list.begin());           }
    Window& nextW()       { return next_itr_cir<Window>(_itr, _list); }
    Window& prevW()       { return prev_itr_cir<Window>(_itr, _list); }
    void emplace_front(const HWND& hWnd, const WindowState& state) {
      _list.emplace_front(hWnd, state);
      ++_length;
    }
    void emplace_back(const HWND& hWnd, const WindowState& state) {
      _list.emplace_back(hWnd, state);
      ++_length;
    }
    void insert(const Window& window) {
      _itr = _list.insert(_itr, window);
      ++_length;
    }
    void erase() {
      _itr = _list.erase(_itr);
      --_length;
    }

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
    Layout(const LayoutType& name, const stdfunc& func) : _name(name),
                                                          _func(func) {}
    LayoutType getName() const { return _name; }
    void arrange()       const { _func();      }

  private:
    LayoutType _name;
    stdfunc _func;
};

class LayoutList final {
  public:
    LayoutList()                  { init();                                         }
    void init()                   { _itr = _list.begin();                           }
    const Layout& focused() const { return *_itr;                                   }
    const Layout& next()          { return next_itr_cir<const Layout>(_itr, _list); }
    const Layout& prev()          { return prev_itr_cir<const Layout>(_itr, _list); }

  private:
    static const std::array<Layout, 2> _list;
    std::array<Layout, 2>::const_iterator _itr;
};

const std::array<Layout, 2> LayoutList::_list = {{
  Layout( LayoutType::SPIRAL,    spiral_impl   ),
  Layout( LayoutType::TILELEFT,  tileleft_impl )
}};

class Desktop final {
  public:
    Desktop() {}
    Desktop(const char id) : _id(id),
                             _layoutList(new LayoutList),
                             _showWndList(new WindowList),
                             _hideWndList(new WindowList) {}
    std::unique_ptr<LayoutList>& getLayout()  { return _layoutList;  }
    std::unique_ptr<WindowList>& getShowWnd() { return _showWndList; }
    std::unique_ptr<WindowList>& getHideWnd() { return _hideWndList; }
    void save() {

    }
    void restore() {

    }

  private:
    char _id;
    std::unique_ptr<LayoutList> _layoutList;
    std::unique_ptr<WindowList> _showWndList;
    std::unique_ptr<WindowList> _hideWndList;
};

class DesktopList final {
  public:
    DesktopList() {
      for (auto i=0; i<_length; ++i)
        _list[i] = Desktop(i);

      _itr = _list.begin();
    }
    Desktop& focused() const { return *_itr; }
    void move_focus(int index) {

    }

  private:
    static const size_t _length = 9;
    std::array<Desktop, _length> _list;
    std::array<Desktop, _length>::iterator _itr;
};

/* variables */
HHOOK hookKey = 0;
HWND clientWnd;

std::unique_ptr<DesktopList> deskList(new DesktopList);

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

