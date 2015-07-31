#include <iostream>

#include <string>
#include <map>
#include <vector>
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
using uintvec = std::vector<unsigned int>;

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
  { 'J',  func_switcher( move_focus(1),   move_window(2)  )},
  { 'K',  func_switcher( move_focus(-1),  move_window(-2) )},
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

enum struct WindowFlag {
  SHOW,
  HIDE
};

class Window final {
  public:
    Window(){}
    Window(const HWND& handle, const WindowState& state) : _handle(handle), _state(state){}
    HWND getHandle() const;
    WindowState getState() const;
    void setState(const WindowState& state);

  private:
    HWND _handle;
    WindowState _state;
};

class WindowList final {
  public:
    HWND focused();
    HWND next(const WindowFlag&);
    HWND prev(const WindowFlag&);
    void add(const Window&);
    void remove(const Window&);
    unsigned int length(const WindowFlag&);

  private:
    void move_index(const int, const WindowFlag&);

    int _index = 0;
    std::vector<Window> _showWndList;
    std::vector<Window> _hideWndList;
    unsigned int _showLength = 0;
    unsigned int _hideLength = 0;
};

std::unique_ptr<WindowList> wndList(new WindowList);

