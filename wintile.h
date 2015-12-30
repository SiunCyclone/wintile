#ifndef WINTILE_H
#define WINTILE_H

#include <array>
#include <exception>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <windows.h>

using stdfunc = std::function<void()>;

constexpr auto MODKEY    = VK_NONCONVERT;
constexpr auto SUBMODKEY = VK_LSHIFT;
auto WINDOW_WIDTH  = GetSystemMetrics(SM_CXSCREEN);
auto WINDOW_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

/* function declarations */
void update_alias_list();
stdfunc func_switcher(const stdfunc&, const stdfunc&);
stdfunc move_focus(const int);
stdfunc swap_window(const int);
stdfunc transfer_window(const int);
stdfunc open_app(const wchar_t*);
void switch_maximum();
void switch_float();
void destroy_window();
void call_next_layout();
void call_prev_layout();
void quit();

void tileleft_impl();
void spiral_impl();

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK LLKeyboardProc(int, WPARAM, LPARAM);
BOOL CALLBACK EnumWndProc(HWND, LPARAM);
LRESULT CALLBACK BarWndProc(HWND, UINT, WPARAM, LPARAM);

void start_hook(const HINSTANCE);
void stop_hook();
void show_taskbar();
void hide_taskbar();
void create_window(const HINSTANCE);
void get_all_window();
void init(const HINSTANCE);
void cleanup();

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
class win32api_error : public std::runtime_error {
  public:
    using runtime_error::runtime_error;
};

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
      SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_THICKFRAME);
      _list.emplace_front(hWnd, state);
      ++_length;
    }
    void emplace_back(const HWND& hWnd, const WindowState& state) {
      SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_THICKFRAME);
      _list.emplace_back(hWnd, state);
      ++_length;
    }
    void insert(const Window& window) {
      SetWindowLongPtr(window.getHandle(), GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_THICKFRAME);
      _itr = _list.insert(_itr, window);
      ++_length;
    }
    void erase() {
      _itr = _list.erase(_itr);
      --_length;
    }
    std::list<Window>::iterator get_itr() {
      return _itr;
    }
    void set_itr(std::list<Window>::iterator itr) {
      _itr = itr;
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
    Desktop(const int id) : _id(id),
                            _layoutList(new LayoutList),
                            _showWndList(new WindowList),
                            _hideWndList(new WindowList) {}
    std::shared_ptr<LayoutList> getLayoutList()  { return _layoutList;  }
    std::shared_ptr<WindowList> getShowWndList() { return _showWndList; }
    std::shared_ptr<WindowList> getHideWndList() { return _hideWndList; }
    void save()    { save_restore_impl("save"); }
    void restore() { save_restore_impl("restore"); }
    int id()       { return _id; }

  private:
    void save_restore_impl(std::string type) {
      if (_showWndList->length() > 0) {
        auto rect = _showWndList->focusedW().getRect();
        std::function<int()> left;
        std::function<int()> top;

        if (type == "save") {
          left = [&] { return WINDOW_WIDTH + rect.left; };
          top = [&] { return WINDOW_HEIGHT + rect.top; };
        } else if (type == "restore") {
          left = [&] { return rect.left; };
          top = [&] { return rect.top; };
        }

        for (auto i=0; i<_showWndList->length(); ++i) {
          SetWindowPos(_showWndList->focused(), 0, left(), top(), 0, 0, SWP_NOSIZE);
          rect = _showWndList->nextW().getRect();
        }

        move_focus(0)();
      }
    }

    int _id;
    std::shared_ptr<LayoutList> _layoutList;
    std::shared_ptr<WindowList> _showWndList;
    std::shared_ptr<WindowList> _hideWndList;
};

class Bar final {
  public:
    void create(const HINSTANCE hInstance) {
      WNDCLASSEX wcex;
      auto className = TEXT("WintileBarClass");

      wcex.cbSize = sizeof(WNDCLASSEX);
      wcex.style = 0;
      wcex.lpfnWndProc = BarWndProc;
      wcex.cbClsExtra = 0;
      wcex.cbWndExtra = 0;
      wcex.hInstance = hInstance;
      wcex.hIcon = nullptr;
      wcex.hCursor = nullptr;
      wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
      wcex.lpszMenuName = nullptr;
      wcex.lpszClassName = className;
      wcex.hIconSm = nullptr;

      if (RegisterClassEx(&wcex) == 0)
        return;

      _handle = CreateWindowEx(WS_EX_TOOLWINDOW,
                               className,
                               TEXT("wintileBar"),
                               WS_VISIBLE | WS_POPUP,
                               0, 0, _width, _height,
                               0,
                               nullptr,
                               hInstance,
                               nullptr);

      if (_handle == nullptr)
        return;
    }
    void paint(std::shared_ptr<Desktop> desktop) {
      HDC hdc = GetDC(_handle);

      auto defBkColor = RGB(0, 0, 0);
      auto defTextColor = RGB(190, 190, 190);
      auto curBkColor = RGB(110, 120, 140);
      auto curTextColor = RGB(230, 230, 230);

      std::wstring str;
      auto w = 24;

      /* Paint 1~9 tag except current-desktop-tag */
      auto pen        = CreatePen(PS_SOLID, 1, defBkColor);
      auto brush      = CreateSolidBrush(defBkColor);
      SetBkColor(hdc, defBkColor);
      SetTextColor(hdc, defTextColor);
      SelectObject(hdc, pen);
      SelectObject(hdc, brush);
      for (int i=1; i<10; ++i) {
        if (i == desktop->id()+1)
          continue;

        Rectangle(hdc, w*i, -1, w*(i+1), _height);
        str = std::to_wstring(i);
        TextOut(hdc, w*i+8, 1, str.c_str(), str.length());
      }

      /* Paint current-desktop-tag */
      auto i = desktop->id()+1;
      pen   = CreatePen(PS_SOLID, 1, curBkColor);
      brush = CreateSolidBrush(curBkColor);
      SetBkColor(hdc, curBkColor);
      SetTextColor(hdc, curTextColor);
      SelectObject(hdc, pen);
      SelectObject(hdc, brush);

      Rectangle(hdc, w*i, -1, w*(i+1), _height);
      str = std::to_wstring(i);
      TextOut(hdc, w*i+8, 1, str.c_str(), str.length());

      DeleteObject(pen);
      DeleteObject(brush);
      ReleaseDC(_handle, hdc);
    }
    int getWidth()  { return _width; }
    int getHeight() { return _height; }

  private:
    HWND _handle;
    int _width = WINDOW_WIDTH;
    int _height = 20;
};
std::unique_ptr<Bar> bar(new Bar);

class DesktopList final {
  public:
    DesktopList() {
      for (auto i=0; i<_length; ++i)
        _list[i] = std::make_shared<Desktop>(i);
    }
    std::shared_ptr<Desktop> focused() { return _list[_index]; }
    stdfunc swap_desktop(int index) {
      return [=] {
        if ((_index - index) != 0) {
          std::cout << "index(Desktop _id)" << index << std::endl;
          focused()->save();

          _index = index;
          bar->paint(focused());
          update_alias_list();

          focused()->restore();
        }
      };
    }

  private:
    static const size_t _length = 9;
    unsigned int _index = 0;
    std::array<std::shared_ptr<Desktop>, _length> _list;
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
  { 'J',        func_switcher( move_focus(1),              swap_window(1)         )},
  { 'K',        func_switcher( move_focus(-1),             swap_window(-1)        )},
  { 'D',        func_switcher( []{},                       destroy_window         )},
  { VK_RETURN,  func_switcher( []{},                       open_app(terminalPath) )},
  { VK_SPACE,   func_switcher( call_next_layout,           call_prev_layout       )},
  { '1',        func_switcher( deskList->swap_desktop(0),  transfer_window(0)     )},
  { '2',        func_switcher( deskList->swap_desktop(1),  transfer_window(1)     )},
  { '3',        func_switcher( deskList->swap_desktop(2),  transfer_window(2)     )},
  { '4',        func_switcher( deskList->swap_desktop(3),  transfer_window(3)     )},
  { '5',        func_switcher( deskList->swap_desktop(4),  transfer_window(4)     )},
  { '6',        func_switcher( deskList->swap_desktop(5),  transfer_window(5)     )},
  { '7',        func_switcher( deskList->swap_desktop(6),  transfer_window(6)     )},
  { '8',        func_switcher( deskList->swap_desktop(7),  transfer_window(7)     )},
  { '9',        func_switcher( deskList->swap_desktop(8),  transfer_window(8)     )},
  { 'A',                       swap_window(0)                                      },
  { 'I',                       open_app(browserPath)                               },
  { 'M',                       switch_maximum                                      },
  { 'F',                       switch_float                                        },
  { 'Q',                       quit                                                }
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

