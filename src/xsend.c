#include <X11/Xlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "xcompgrab.h"
static unsigned int state;
static Window last_window;
static int last_x_root, last_y_root, last_width, last_height;

void XPos(Window win, int *x, int *y, int *width, int *height)
{
	XWindowAttributes attr;
	int xx = 0, yy = 0;
	unsigned int ntree;
	Window root;
	Window *tree;;
	Window nwin = win;
	gLastXError = 0;
	XGetWindowAttributes(gDisplay, win, &attr);
	if(gLastXError)
		return;
	xx = attr.x;
	yy = attr.y;
	*width = attr.width;
	*height = attr.height;
	do {
		win = nwin;
		tree = NULL;
		XQueryTree(gDisplay, win, &root, &nwin, &tree, &ntree);
		if(tree)
			XFree(tree);
		else return;
		XGetWindowAttributes(gDisplay, nwin, &attr);
		xx += attr.x;
		yy += attr.y;
	}
	while (nwin != gRoot);
	*x = xx;
	*y = yy;
}

#define ACTIVATE_ENTER (1U<<0)
#define ACTIVATE_FOCUS (1U<<1)
#define ACTIVATE_REAL_FOCUS (1U<<2)
#define ACTIVATE_WM (1U<<3)

void window_activate(Window w, unsigned int flags, int x, int y)
{
	Display *dpy = gDisplay;
	XEvent event;
	last_window = w;
	XPos(w, &last_x_root, &last_y_root, &last_width, &last_height);
	if(gLastXError)
		return;
	if(flags & ACTIVATE_FOCUS)
	{
		memset (&event, 0, sizeof (event));
		event.type = FocusIn;


		event.xfocus.window = w;
		event.xfocus.mode = NotifyNormal;
		event.xfocus.detail = NotifyNonlinear;
		if (XSendEvent (dpy, event.xfocus.window, True, FocusChangeMask, &event) == 0)
			fprintf (stderr, "Error to send the event!\n");
	}
	if(flags & ACTIVATE_ENTER)
	{
		memset (&event, 0, sizeof (event));
		event.type = EnterNotify;
		event.xcrossing.same_screen = True;
		event.xcrossing.subwindow = 0;
		event.xcrossing.window = w;
		event.xcrossing.x = x;
		event.xcrossing.y = y;
		event.xcrossing.x_root = last_x_root + x;
		event.xcrossing.y_root = last_y_root + y;
		event.xcrossing.root = gRoot;
		event.xcrossing.state = state;//0xFFFF;
		event.xcrossing.focus = True;
		event.xcrossing.mode = NotifyNormal;
		if (XSendEvent (dpy, event.xcrossing.window, True, EnterWindowMask, &event) == 0)
			fprintf (stderr, "Error to send the event!\n");

	}

}

void pointer_move(Window w, int x, int y, int x_root, int y_root)
{
	Display *dpy = gDisplay;
	XEvent event;
	memset (&event, 0, sizeof (event));
	event.type = MotionNotify;
	event.xmotion.same_screen = True;
	event.xmotion.subwindow = 0;
	event.xmotion.window = w;
	event.xmotion.x = x;
	event.xmotion.y = y;
	event.xmotion.x_root = x_root + x;
	event.xmotion.y_root = y_root + y;
	event.xmotion.root = gRoot;
	event.xmotion.state = state;//0xFFFF;
	//printf("move %x %d %d\n", state, x, y);
	if (XSendEvent (dpy, event.xmotion.window, True, state*0?ButtonMotionMask:PointerMotionMask, &event) == 0)
		fprintf (stderr, "Error to send the event!\n");
	XFlush (dpy);
}

void update_button(Window w, int x, int y, int x_root, int y_root, int button, int b)
{
	Display *dpy = gDisplay;
	XEvent event;
	memset (&event, 0, sizeof (event));
	event.xbutton.button = button;
	event.xbutton.same_screen = True;
	event.xbutton.subwindow = 0;
	event.xbutton.window = w;
	event.xbutton.x = x;
	event.xbutton.y = y;
	event.xbutton.x_root = x_root + x;
	event.xbutton.y_root = y_root + y;
	event.xbutton.root = gRoot;
	event.xbutton.state =state;// 0xFFFF;//0xFFFF;
	event.type = b? ButtonPress : ButtonRelease;
	unsigned int mask = b?ButtonPressMask : ButtonReleaseMask;
	//printf("button %x %d %d\n", state, button, b);
	if (XSendEvent (dpy, event.xbutton.window, True, mask, &event) == 0)
		fprintf (stderr, "Error to send the event!\n");
	XFlush (dpy);
	
}

int clickWindowAt( Window w, int flags,
              int rx, int ry, int depth, int new_state, int x_root, int y_root)
{
	Display *dpy = gDisplay;
    XWindowAttributes attrs;
    gLastXError = 0;
    XGetWindowAttributes( dpy, w, &attrs );
	if(gLastXError)
		return 0;
    if ( attrs.map_state == IsViewable &&
         1 ) {//atts.width >= minSize && atts.height >= minSize ) {
        int x = 0, y = 0;
        if(depth){
            x = rx - attrs.x;
            y = ry - attrs.y;
            x_root += attrs.x;
            y_root += attrs.y;
        }else x = rx, y = ry;
        if(x < 0 || y < 0 || x > attrs.width || y > attrs.height )
            return 0;


        Window root, parent;
        Window* children;
        unsigned int nchildren;

        //Window w2 = 0;
	if(!depth && last_window != w)
		window_activate(w,flags, x, y);
	if(!depth)
		x_root = last_x_root, y_root = last_y_root;
		int clicked = 0;
        if( XQueryTree( dpy, w, &root, &parent, &children, &nchildren ) != 0 ) {
                for( int i = nchildren - 1; i >= 0; --i ) {
                //for( int i = 0; i < nchildren; ++i ) {
                    if(clickWindowAt( children[ i ], flags, x, y, depth + 1, new_state, x_root, y_root ) )
                    {
                        //w2 = children[i];
                        if( children != NULL ) {
                            XFree( children );
                        }
                        return 1;
                        clicked = 1;
                    }
                }

                if( children != NULL ) {
                    XFree( children );
                }
        }
        if(clicked)
           return 1;
//        w = w2;
      //  printf("click %d %d %d %d %d %d %d %d\n", (int)w, attrs.x, attrs.y, x, y, depth, x_root, y_root);
//	XEvent event;

#if 1
    pointer_move( w, x, y, x_root, y_root);

#endif
#if 1
//	update_button(dpy, w, x, y, x_root, y_root, button, b);
	if(new_state != 0xFF00)
	{
	unsigned int bstate = (state & 0x1F00) >> 8;
	unsigned int nstate = (new_state & 0x1f00) >> 8;
	unsigned int dstate = bstate ^ nstate;
	unsigned int button = Button1;
	while(dstate)
	{
		if(dstate & 1)
			update_button(w, x, y, x_root, y_root, button, nstate & 1);
		dstate >>=1, nstate >>= 1;
		button++;
	}
	state = new_state;
	/*if(b)
		state |= 1 << (button + 7);
	else
		state &= ~(1<<button + 7);*/
	}
#endif
#if 1
	if(state)pointer_move(w, x, y, x_root, y_root);
#endif
        return 1;

    }
    return 0;
}


void xsend_window_activate(struct compwindow_data *data, unsigned int flags, int x, int y)
{
	Window w = data->w;
	if(w)window_activate(w, flags, x, y);
	data->x = last_x_root;
	data->y = last_y_root;
}
void xsend_window_mouse(struct compwindow_data *data, unsigned int state, unsigned int flags, int x, int y )
{
	Window w = data->w;
	if(w)clickWindowAt(w, flags, x, y, 0,  state,  0, 0);
	data->x = last_x_root;
	data->y = last_y_root;
}
#include <X11/XKBlib.h>
void xsend_window_keyboard(struct compwindow_data *data, unsigned long keyCode, int press, unsigned int state)
{
	Window w = data->w;
	if(!w)
		return;
      XEvent ev;
      ev.xkey.type = press?KeyPress:KeyRelease;
      ev.xkey.window = w;
      ev.xkey.root = ev.xkey.subwindow = None;
      ev.xkey.time = 0;
      ev.xkey.x = ev.xkey.y = ev.xkey.x_root = ev.xkey.y_root = 0;
      XkbStateRec xkbState;
      XkbGetState(gDisplay, XkbUseCoreKbd, &xkbState);
      ev.xkey.state = state | xkbState.group << 13;
      ev.xkey.keycode = keyCode;
      ev.xkey.same_screen = True;
      XSendEvent(gDisplay, w, True, press?KeyPressMask:KeyReleaseMask, &ev);
      XFlush(gDisplay);
}


#if 0
Display *gDisplay;
Window gRoot;
int main (int argc, char *argv[])
{
	int x = 1;
	int y = 1;

	// Open X display
	gDisplay = XOpenDisplay (NULL);
	gRoot = DefaultRootWindow(gDisplay);
	if (gDisplay == NULL)
	{
		fprintf (stderr, "Can't open display!\n");
		return -1;
	}


//    printf("%x\n", xkbState.group << 13);
//	move(display, 500, 700);

	window_activate(atoi(argv[1]), ACTIVATE_FOCUS|ACTIVATE_ENTER, atoi(argv[2]), atoi(argv[3]));
	clickWindowAt(atoi(argv[1]),3, atoi(argv[2]), atoi(argv[3]), 0, 1 << (8 + atoi(argv[4])), 0, 0);
	for(int i = 0; i < 2; i++)
	{//system("xdotool key XF86Ungrab");
	usleep(1000000);
	}
	clickWindowAt(atoi(argv[1]),3, atoi(argv[2])+100, atoi(argv[3])+100, 0, 0, 0, 0 );
	XCloseDisplay (gDisplay);
	return 0;
}
#endif