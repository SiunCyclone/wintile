#include <iostream>

#include <string>
#include <map>
#include <list>
#include <functional>
#include <tuple>
#include <memory>
#include <clocale>
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
void maximize();
void close_window();
void quit();

stdfunc call_layout(const stdfunc&);
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
static std::map<std::string, stdfunc> arrange = {
  { "TILE",    call_layout( tile_layout   )},
  { "SPIRAL",  call_layout( spiral_layout )}
};

static std::map<unsigned int, bool> isPressed = {
  { MODKEY,     false },
  { SUBMODKEY,  false }
};

static std::map<unsigned int, stdfunc> callFunc = {
  { 'J',  func_switcher( move_focus(1),   move_window(1)  )},
  { 'K',  func_switcher( move_focus(-1),  move_window(-1) )},
  { 'A',                 move_window(0)                    },
  { 'M',                 maximize                          },
  { 'D',  func_switcher( []{},            close_window    )},
  { 'Q',                 quit                              }
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
    Window(){}
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
    Window& frontW();
    Window& focusedW();
    Window& nextW();
    Window& prevW();
    void push_back(const Window&);
    void insert(const Window&);
    void erase();
    size_t length() const;

  private:
    size_t _length = 0;
    std::list<Window> _list;
    std::list<Window>::iterator _itr;
};

std::unique_ptr<WindowList> showWndList(new WindowList);
std::unique_ptr<WindowList> hideWndList(new WindowList);

