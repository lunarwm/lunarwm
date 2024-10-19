/* Thanks so much to https://jichu4n.com/posts/how-x-window-managers-work-and-how-to-write-one-part-i */

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
#include "main.hpp"

void WindowManager::grabKey(std::string key, unsigned int mod)
{
    KeySym sym = XStringToKeysym(key.c_str());
    KeyCode code = XKeysymToKeycode(dpy, sym);
    XGrabKey(dpy, code, mod, root, 0, GrabModeAsync, GrabModeAsync);
    XSync(dpy, 0);
}

void WindowManager::OnCreateNotify(const XCreateWindowEvent &e)
{
    /* TODO: Implement */
}

void WindowManager::OnDestroyNotify(const XDestroyWindowEvent &e)
{
    /* There's nothing needed to do */
}

void WindowManager::OnExpose(const XExposeEvent &e)
{
    /* TODO: Implement */
}

void WindowManager::OnKeyPress(const XKeyEvent &e)
{
    XAllowEvents(dpy, ReplayPointer, CurrentTime);
    XSync(dpy, 0);
}

void WindowManager::OnReperentNotify(const XReparentEvent &e)
{
    /* TODO: Implement */
}

void WindowManager::OnConfigureRequest(const XConfigureRequestEvent &e)
{
    XWindowChanges changes;
    changes.x = e.x;
    changes.y = e.y;
    changes.width = e.width;
    changes.height = e.height;
    changes.border_width = e.border_width;
    changes.sibling = e.above;
    changes.stack_mode = e.detail;
    XConfigureWindow(dpy, e.window, e.value_mask, &changes);
}

void WindowManager::OnMapRequest(const XMapRequestEvent &e)
{
    Frame(e.window, 0);
    XMapWindow(dpy, e.window);
}

/* Border, handling input, crashes etc. */
void WindowManager::Frame(Window w, bool createdBeforeWindowManager)
{
    /* TODO Make border customisable */
    const unsigned int BORDER_WIDTH = 3;
    const unsigned long BORDER_COLOR = 0xff0000;
    const unsigned long BG_COLOR = 0x0000ff;

    XWindowAttributes x_window_attrs;
    CHECK(XGetWindowAttributes(dpy, w, &x_window_attrs));

    if (createdBeforeWindowManager)
    {
        if (x_window_attrs.override_redirect ||
            x_window_attrs.map_state != IsViewable)
        {
            return;
        }
    }

    const Window frame = XCreateSimpleWindow(
        dpy,
        root,
        x_window_attrs.x,
        x_window_attrs.y,
        x_window_attrs.width,
        x_window_attrs.height,
        BORDER_WIDTH,
        BORDER_COLOR,
        BG_COLOR);

    XSelectInput(
        dpy,
        frame,
        SubstructureRedirectMask | SubstructureNotifyMask);
    XAddToSaveSet(dpy, w);
    XReparentWindow(
        dpy,
        w,
        frame,
        0, 0);
    XMapWindow(dpy, frame);
    clients[w] = frame;

    /* Move windows with alt + left button. */
    XGrabButton(dpy, Button1, Mod1Mask, frame, True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    /* Resize windows with alt + right button. */
    XGrabButton(dpy, Button3, Mod1Mask, frame, True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    /* Kill windows with alt + f4. */
    grabKey("F4", Mod1Mask);

    /* Switch windows with alt + tab. */
    grabKey("Tab", Mod1Mask);
}

void WindowManager::OnUnmapNotify(const XUnmapEvent &e)
{
    /* If window not in clients or is root ignore request */
    if (!clients.count(e.window) or e.event == root)
    {
        return;
    }

    Unframe(e.window);
}

void WindowManager::Unframe(Window w)
{
    XUnmapWindow(dpy, clients[w]);
    XReparentWindow(
        dpy,
        w,
        root,
        0, 0);
    XRemoveFromSaveSet(dpy, w);
    XDestroyWindow(dpy, clients[w]);
    clients.erase(w);
}

WindowManager::WindowManager()
{
    /* Open the display */
    if ((dpy = XOpenDisplay(NULL)) == NULL)
        errx(1, "Cannot open display");

    /* Get the default screen and root window */
    scr = DefaultScreen(dpy);
    root = RootWindow(dpy, scr);

    /* Create a window with a black border and white background */
    win = XCreateSimpleWindow(dpy, root, 200, 200, 800, 600, 4, BlackPixel(dpy, scr), WhitePixel(dpy, scr));

    /* Request the X server to send events */
    XSelectInput(dpy, win, ExposureMask | KeyPressMask | CreateNotify | DestroyNotify | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | PointerMotionHintMask | ConfigureRequest);

    /* Tells X to send `ButtonPress` events on the root. */
    XGrabButton(dpy, Button1, 0, root, 0, ButtonPressMask, GrabModeSync,
                GrabModeAsync, NULL, NULL);

    /*
    if (OnWMDetected(dpy, NULL))
    {
        std::cout << "Detected another window manager on display " << XDisplayString(dpy) << "\n";
        return;
    }
    */
    XSetErrorHandler(&WindowManager::OnXError);


    /* Sync the changes. */
    XSync(dpy, 0);

    /* Map display to window */
    XMapWindow(dpy, win);

    /* Set window title */
    XStoreName(dpy, win, title);

    Cursor cursor = XCreateFontCursor(dpy, XC_left_ptr);

    /* Define cursor for root window */
    XDefineCursor(dpy, root, cursor);
}

void WindowManager::run()
{
    /* Grab X server to prevent changes */
    XGrabServer(dpy);
    /* Query and frame existing top-level windows */
    Window returned_root, returned_parent;
    Window *top_level_windows;
    unsigned int num_top_level_windows;
    CHECK(XQueryTree(
        dpy,
        root,
        &returned_root,
        &returned_parent,
        &top_level_windows,
        &num_top_level_windows));
    CHECK_EQ(returned_root, root);
    for (unsigned int i = 0; i < num_top_level_windows; ++i)
    {
        Frame(top_level_windows[i], 1);
    }
    XFree(top_level_windows);
    XUngrabServer(dpy);

    XEvent e;
    for (;;)
    {
        XNextEvent(dpy, &e);
        switch (e.type)
        {
        default:
            puts("Unexpected event");
            break;
        case CreateNotify:
            OnCreateNotify(e.xcreatewindow);
            break;
        case DestroyNotify:
            OnDestroyNotify(e.xdestroywindow);
            break;
        case ReparentNotify:
            OnReperentNotify(e.xreparent);
            break;
        case Expose:
            OnExpose(e.xexpose);
            break;
        case KeyPress:
            OnKeyPress(e.xkey);
            break;
        case ConfigureRequest:
            OnConfigureRequest(e.xconfigurerequest);
            break;
        case UnmapNotify:
            OnUnmapNotify(e.xunmap);
            break;
        case MapRequest:
            OnMapRequest(e.xmaprequest);
            break;
        }
    }
    XSync(dpy, 0);
}


bool WindowManager::OnWMDetected(Display* display, XErrorEvent* e)
{
    CHECK_EQ(static_cast<int>(e->error_code), BadAccess);
    bool wmdetected = true;
    return wmdetected;
}

int WindowManager::OnXError(Display* display, XErrorEvent* e) {
  const int MAX_ERROR_TEXT_LENGTH = 1024;
  char error_text[MAX_ERROR_TEXT_LENGTH];
  XGetErrorText(display, e->error_code, error_text, sizeof(error_text));
  std::cout << "Received X error:\n"
             << "    Error code: " << int(e->error_code)
             << " - " << error_text << "\n"
             << "    Resource ID: " << e->resourceid;
}

int main(int argc, char *argv[])
{
    WindowManager wm;
    wm.run();
    return 0;
}
