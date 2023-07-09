#include <X11/Xlib.h>

extern Display *gDisplay;
extern int gScreen;
extern Window gRoot;
extern int gLastXError;
extern int gLastXErrorReq;
struct compwindow_data
{
Window w;
unsigned long glxpixmap;
Pixmap pixmap;
int x, y, width, height;
int last_texture;
char title[256];
};
#ifdef __cplusplus
extern "C" {
#endif
void gprint(const char *str);
void xcomp_init_display(void);
int xcomp_register_window( struct compwindow_data *data, const char *atom, const char *value, int index);
int xcomp_register_popup(struct compwindow_data *data, int parent, int index);
void xcomp_update_texture(struct compwindow_data *data, int texture);
void xcomp_reset_texture(struct compwindow_data *data);
void xsend_window_activate(struct compwindow_data *data, unsigned int flags, int x, int y);
void xsend_window_mouse(struct compwindow_data *data, unsigned int state, unsigned int flags, int x, int y );
void xsend_window_keyboard(struct compwindow_data *data, unsigned long keyCode, int press, unsigned int state);

#ifdef __cplusplus
}
#endif