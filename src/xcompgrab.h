#include <X11/Xlib.h>

extern Display *gDisplay;
extern int gScreen;
extern Window gRoot;
struct compwindow_data
{
Window w;
unsigned long glxpixmap;
Pixmap pixmap;
int x, y, width, height;
int last_texture;
};
#ifdef __cplusplus
extern "C" {
#endif
void gprint(const char *str);
void xcomp_init_display(void);
int xcomp_register_window( struct compwindow_data *data, const char *cname, int index);
void xcomp_update_texture(struct compwindow_data *data, int texture);
void xsend_window_activate(Window w, unsigned int flags, int x, int y);
void xsend_window_mouse(Window w, unsigned int buttons, unsigned int flags, int x, int y );
void xsend_window_keyboard(Window w, unsigned long keyCode, int press);

#ifdef __cplusplus
}
#endif