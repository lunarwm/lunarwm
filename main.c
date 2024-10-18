#include <X11/X.h>
#include <stdio.h>
#include <err.h>
#include <X11/Xlib.h>
#include "main.h"
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/cursorfont.h>

static Display* dpy;
static int scr;
static Window root;

int main(int argc, char* argv[])
{
    Window win;
    XEvent e;

    /* Open the display */
    if ((dpy = XOpenDisplay(NULL)) == NULL)
        errx(1, "Cannot open display");

    /* Get the default screen and root window */
    scr = DefaultScreen(dpy);
    root = RootWindow(dpy, scr);

    /* Create a window with a black border and white background */
    win = XCreateSimpleWindow(dpy, root, POSX, POSY, WIDTH, HEIGHT, BORDER, BlackPixel(dpy, scr), WhitePixel(dpy, scr));

    /* Request the X server to send events related to SubstructureRedirectMask, ResizeRedirectMask and SubstructureNotifyMask */
    XSelectInput(dpy, win, ExposureMask | KeyPressMask);
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

    for (;;)
    {
        XNextEvent(dpy, &e);
        switch (e.type)
        {
            default:
                puts("Unexpected event");
                break;
            case KeyPress:
                XAllowEvents(dpy, ReplayPointer, CurrentTime);
                XSync(dpy, 0);
                puts("Button pressed!");
                break;
        }
        XSync(dpy, 0);
    }
}

void grabKey(char *key, unsigned int mod) {
    KeySym sym = XStringToKeysym(key);
    KeyCode code = XKeysymToKeycode(dpy, sym);
    XGrabKey(dpy, code, mod, root, 0, GrabModeAsync, GrabModeAsync);
    XSync(dpy, 0);
}