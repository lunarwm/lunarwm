#include <X11/X.h>
#include <stdio.h>
#include <err.h>
#include <X11/Xlib.h>
#include "main.h"
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/cursorfont.h>
#include <string>

class WindowManager {
private:
    Display* dpy;
    int scr;
    Window root;
    Window win;

public:
    WindowManager() {
        // Open the display
        if ((dpy = XOpenDisplay(NULL)) == NULL)
            errx(1, "Cannot open display");

        // Get the default screen and root window
        scr = DefaultScreen(dpy);
        root = RootWindow(dpy, scr);

        // Create a window with a black border and white background
        win = XCreateSimpleWindow(dpy, root, POSX, POSY, WIDTH, HEIGHT, BORDER, BlackPixel(dpy, scr), WhitePixel(dpy, scr));

        // Request the X server to send events
        XSelectInput(dpy, win, ExposureMask | KeyPressMask | CreateNotify | DestroyNotify | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | PointerMotionHintMask | ConfigureRequest);

        // Tells X to send `ButtonPress` events on the root.
        XGrabButton(dpy, Button1, 0, root, 0, ButtonPressMask, GrabModeSync,
                GrabModeAsync, NULL, NULL);

        // Sync the changes.
        XSync(dpy, 0);

        // Map display to window
        XMapWindow(dpy, win);

        // Set window title
        XStoreName(dpy, win, title);

        Cursor cursor = XCreateFontCursor(dpy, XC_left_ptr);

        // Define cursor for root window
        XDefineCursor(dpy, root, cursor);
    }

    void run() {
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
            }
            XSync(dpy, 0);
        }
    }

    void grabKey(std::string key, unsigned int mod) {
        KeySym sym = XStringToKeysym(key.c_str());
        KeyCode code = XKeysymToKeycode(dpy, sym);
        XGrabKey(dpy, code, mod, root, 0, GrabModeAsync, GrabModeAsync);
        XSync(dpy, 0);
    }

private:
    void OnCreateNotify(const XCreateWindowEvent& e)
    {
        // TODO: Implement
    }

    void OnDestroyNotify(const XDestroyWindowEvent& e)
    {
        // TODO: Implement
    }

    void OnExpose(const XExposeEvent& e)
    {
        // TODO: Implement
    }

    void OnKeyPress(const XKeyEvent& e)
    {
        XAllowEvents(dpy, ReplayPointer, CurrentTime);
        XSync(dpy, 0);
    }

    void OnReperentNotify(const XReparentEvent& e)
    {
        // TODO: Implement
    }

    void OnConfigureRequest(const XConfigureRequestEvent& e)
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

};

int main(int argc, char* argv[])
{
    WindowManager wm;
    wm.run();
    return 0;
}
