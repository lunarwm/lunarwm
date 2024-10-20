#ifndef MAIN_H
#define MAIN_H

#include <X11/X.h>
#include <stdio.h>
#include <err.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/cursorfont.h>
#include <string>
#include <string>
#include <unordered_map>
#include <iostream>

#define CHECK(X) if (!(X)) { fprintf(stderr, "%s:%d: %s failed\n", __FILE__, __LINE__, #X); }
#define CHECK_EQ(expected, actual)                                           \
  do {                                                                        \
    auto &&_expected = (expected);                                            \
    auto &&_actual = (actual);                                                \
    if (_expected != _actual) {                                               \
      std::cerr << __FILE__ << ":" << __LINE__ << ": Check failed: "         \
                << #expected << " == " << #actual << " (" << _expected        \
                << " vs. " << _actual << ")" << std::endl;                   \
      abort();                                                                \
    }                                                                         \
  } while (0)

class WindowManager {
private:
    Display* dpy;
    int scr;
    Window root;
    Window win;
    std::unordered_map<Window, Window> clients;

    const char* title;
    void grabKey(std::string key, unsigned int mod);
    void OnCreateNotify(const XCreateWindowEvent& e);
    void OnDestroyNotify(const XDestroyWindowEvent& e);
    void OnExpose(const XExposeEvent& e);
    void OnKeyPress(const XKeyEvent& e);
    void OnReperentNotify(const XReparentEvent& e);
    void OnConfigureRequest(const XConfigureRequestEvent& e);
    void OnMapRequest(const XMapRequestEvent& e);
    void Frame(Window w, bool createdBeforeWindowManager);
    void OnUnmapNotify(const XUnmapEvent& e);
    void Unframe(Window w);
    static int OnXError(Display* display, XErrorEvent* e);
    bool OnWMDetected(Display* display, XErrorEvent* e);
    void OnButtonPress(const XButtonEvent& e);

public:
    WindowManager();
    ~WindowManager()
    {
        XCloseDisplay(dpy);
    }
    void run();
};

#endif
