#include <iostream>

#include <string>
#include <map>
#include <list>
#include <functional>
#include <tuple>
#include <memory>
#include <windows.h>

static const unsigned int MODKEY    = VK_NONCONVERT;
static const unsigned int SUBMODKEY = VK_LSHIFT;
static const unsigned int WINDOW_WIDTH  = GetSystemMetrics(SM_CXSCREEN);
static const unsigned int WINDOW_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

using stdfunc = std::function<void()>;

/* function declarations */
stdfunc func_switcher(const stdfunc&, const stdfunc&);
stdfunc move_focus(const int);
stdfunc move_window(const int);
stdfunc open_app(const char*);
void maximize();
void destroy_window();
void quit();

void tile_layout();
void spiral_layout();

bool start_hook(HINSTANCE, HWND);
bool stop_hook();
void show_taskbar();
void hide_taskbar();
void create_window(HINSTANCE);
void get_all_window();

/* variables */
HHOOK hhk;

HWND clientWnd;

std::string layout = "TILE";
static std::map<std::string, void (*)()> arrange = {
  { "TILE",    tile_layout   },
  { "SPIRAL",  spiral_layout }
};

static std::map<unsigned int, bool> isPressed = {
  { MODKEY,     false },
  { SUBMODKEY,  false }
};

char terminalPath[256] = "\"C:/msys32/msys2_shell.bat\"";
char browserPath[256] = "\"C:/Program Files/Mozilla Firefox/firefox.exe\"";
static std::map<unsigned int, stdfunc> callFunc = {
  { 'J',        func_switcher( move_focus(1),         move_window(1)         )},
  { 'K',        func_switcher( move_focus(-1),        move_window(-1)        )},
  { 'A',                       move_window(0)                                 },
  { VK_RETURN,  func_switcher( []{},                  open_app(terminalPath) )},
  { 'I',                       open_app(browserPath)                          },
  { 'M',                       maximize                                       },
  { 'D',        func_switcher( []{},                  destroy_window         )},
  { 'Q',                       quit                                           }
};

auto convertUTF16toUTF8 = [=] (const wchar_t* wbuf, const unsigned int maxSize) -> std::string {
  int bufSize = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, nullptr, 0, nullptr, nullptr);
  char buf[maxSize];
  WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, bufSize, nullptr, nullptr);
  return std::string(buf);
};

/* window */
enum struct WindowState {
  NORMAL,
  ICON,
  MAXIMUM,
  FLOAT,
};

struct WindowRect {
  WindowRect(){}
  WindowRect(const int argX, const int argY, const int argW, const int argH) {
    x = argX;
    y = argY;
    w = argW;
    h = argH;
  }

  int x;
  int y;
  int w;
  int h;
};

class Window final {
  public:
    Window(const HWND& handle, const WindowState& state) : _handle(handle), _state(state){}
    HWND getHandle() const;
    WindowState getState() const;
    WindowRect getRect() const;
    void setState(const WindowState&);
    void setRect(const WindowRect&);

  private:
    HWND _handle;
    WindowState _state;
    WindowRect _rect;
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
    void push_back(const Window&);
    void insert(const Window&);
    void erase();
    void swap(Window&, Window&);
    size_t length() const;

  private:
    size_t _length = 0;
    std::list<Window> _list;
    std::list<Window>::iterator _itr;
};

std::unique_ptr<WindowList> showWndList(new WindowList);
std::unique_ptr<WindowList> hideWndList(new WindowList);

