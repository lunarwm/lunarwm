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
#include "main.h"

class WindowManager {
private:
    Display* dpy;
    int scr;
    Window root;
    Window win;
    std::unordered_map<Window, Window> clients;
    const char* title = "LunarWM";
        void grabKey(std::string key, unsigned int mod)
    {
        KeySym sym = XStringToKeysym(key.c_str());
        KeyCode code = XKeysymToKeycode(dpy, sym);
        XGrabKey(dpy, code, mod, root, 0, GrabModeAsync, GrabModeAsync);
        XSync(dpy, 0);
    }

    void OnCreateNotify(const XCreateWindowEvent& e)
    {
        /* TODO: Implement */
    }

    void OnDestroyNotify(const XDestroyWindowEvent& e)
    {
        /* There's nothing needed to do */
    }

    void OnExpose(const XExposeEvent& e)
    {
        /* TODO: Implement */
    }

    void OnKeyPress(const XKeyEvent& e)
    {
        XAllowEvents(dpy, ReplayPointer, CurrentTime);
        XSync(dpy, 0);
    }

    void OnReperentNotify(const XReparentEvent& e)
    {
        /* TODO: Implement */
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

    void OnMapRequest(const XMapRequestEvent& e)
    {
        Frame(e.window);
        XMapWindow(dpy, e.window);
    }

    /* Bording, handling input, crashes etc. */
    void Frame(Window w)
    {
        /* TODO Make border customisable */
        const unsigned int BORDER_WIDTH = 3;
        const unsigned long BORDER_COLOR = 0xff0000;
        const unsigned long BG_COLOR = 0x0000ff;

        XWindowAttributes x_window_attrs;
        CHECK(XGetWindowAttributes(dpy, w, &x_window_attrs));

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

    void OnUnmapNotify(const XUnmapEvent& e)
    {
        /* If window not in clients ignore request */
        if (!clients.count(e.window))
        {
            return;
        }

        Unframe(e.window);
    }

    void Unframe(Window w)
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


public:
    WindowManager() {
        /* Open the display */
        if ((dpy = XOpenDisplay(NULL)) == NULL)
            errx(1, "Cannot open display");

        /* Get the default screen and root window */ 
        scr = DefaultScreen(dpy);
        root = RootWindow(dpy, scr);

        /* Create a window with a black border and white background */
        win = XCreateSimpleWindow(dpy, root, POSX, POSY, WIDTH, HEIGHT, BORDER, BlackPixel(dpy, scr), WhitePixel(dpy, scr));

        /* Request the X server to send events */
        XSelectInput(dpy, win, ExposureMask | KeyPressMask | CreateNotify | DestroyNotify | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | PointerMotionHintMask | ConfigureRequest);

        /* Tells X to send `ButtonPress` events on the root. */
        XGrabButton(dpy, Button1, 0, root, 0, ButtonPressMask, GrabModeSync,
                GrabModeAsync, NULL, NULL);

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
    };

int main(int argc, char* argv[])
{
    WindowManager wm;
    wm.run();
    return 0;
}
