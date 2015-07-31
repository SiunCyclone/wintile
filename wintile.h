#include <iostream>

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <tuple>
#include <memory>
#include <clocale>
#include <windows.h>

#define MODKEY    VK_NONCONVERT
#define SUBMODKEY VK_LSHIFT

using stdfunc = std::function<void()>;
using wndtype = std::tuple<HWND>;
using uintvec = std::vector<unsigned int>;

/* function declarations */
HWND getHandle(wndtype wnd);

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

class Window final {
  public:
    Window(const HWND handle, const std::string state) : _handle(handle), _state(state){}
    HWND getHandle();
    std::string getState();
  private:
    HWND _handle;
    std::string _state;
    // NORMAL
    // ICON
    // MAXIMUM
    // FLOAT
};
class WindowList final {
  public:
    void next();
    void prev();
    void add(const Window&);
    void remove(Window);
    std::vector<Window> getShowList();
    std::vector<Window> getHideList();
    unsigned int length(const std::string);
  private:
    std::vector<Window> _showWndList;
    std::vector<Window> _hideWndList;
};
std::unique_ptr<WindowList> wndList(new WindowList);

