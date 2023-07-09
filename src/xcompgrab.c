#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/composite.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <GL/glx.h>
#include "xcompgrab.h"
Display *gDisplay;
int gScreen;
Window gRoot;
int gLastXError;
int gLastXErrorReq;
static xcb_connection_t *connection;
static void (*BindTexImageEXT) (Display *display, GLXDrawable drawable, int buffer, const int *attrib_list);
static GLXFBConfig gFbConfig;
xcb_atom_t ATOM_UTF8_STRING;
xcb_atom_t ATOM_STRING;
xcb_atom_t ATOM_TEXT;
xcb_atom_t ATOM_COMPOUND_TEXT;
xcb_atom_t ATOM_WM_NAME;
xcb_atom_t ATOM_WM_CLASS;
xcb_atom_t ATOM_WM_TRANSIENT_FOR;
xcb_atom_t ATOM__NET_WM_NAME;
xcb_atom_t ATOM__NET_SUPPORTING_WM_CHECK;
xcb_atom_t ATOM__NET_CLIENT_LIST;

xcb_atom_t get_atom(const char *name)
{
	xcb_intern_atom_cookie_t atom_c =
		xcb_intern_atom(connection, 1, strlen(name), name);
	xcb_intern_atom_reply_t *atom_r =
		xcb_intern_atom_reply(connection, atom_c, NULL);
	xcb_atom_t a = atom_r->atom;
	free(atom_r);
	return a;
}

void xcomp_gather_atoms()
{
	ATOM_UTF8_STRING = get_atom("UTF8_STRING");
	ATOM_STRING = get_atom("STRING");
	ATOM_TEXT = get_atom("TEXT");
	ATOM_COMPOUND_TEXT = get_atom("COMPOUND_TEXT");
	ATOM_WM_NAME = get_atom("WM_NAME");
	ATOM_WM_CLASS = get_atom( "WM_CLASS");
	ATOM__NET_WM_NAME = get_atom("_NET_WM_NAME");
	ATOM__NET_SUPPORTING_WM_CHECK =
		get_atom("_NET_SUPPORTING_WM_CHECK");
	ATOM__NET_CLIENT_LIST = get_atom("_NET_CLIENT_LIST");
	ATOM_WM_TRANSIENT_FOR = get_atom("WM_TRANSIENT_FOR");
}
xcb_get_property_reply_t *xcomp_property_sync(xcb_window_t win, xcb_atom_t atom)
{
	if (atom == XCB_ATOM_NONE)
		return NULL;

	xcb_generic_error_t *err = NULL;
	// Read properties up to 4096*4 bytes
	xcb_get_property_cookie_t prop_cookie =
		xcb_get_property(connection, 0, win, atom, 0, 0, 4096);
	xcb_get_property_reply_t *prop =
		xcb_get_property_reply(connection, prop_cookie, &err);
	if (err != NULL || xcb_get_property_value_length(prop) == 0) {
		free(prop);
		return NULL;
	}

	return prop;
}

int xcomp_window_atom_str(xcb_window_t win, xcb_atom_t atom, char *str, int maxlen)
{

	xcb_get_property_reply_t *cls =
		xcomp_property_sync(win, atom);
	if (!cls)
		return 1;

	// WM_CLASS is formatted differently from other strings, it's two null terminated strings.
	// Since we want the first one, let's just let copy run strlen.
	strncpy(str,(const char *)xcb_get_property_value(cls), maxlen - 1 );
	str[maxlen - 1] = 0;
	free(cls);
	return 0;
}


int xcomp_window_atom_compare_str(xcb_window_t win, xcb_atom_t atom, const char *str)
{

	int ret;
	xcb_get_property_reply_t *cls =
		xcomp_property_sync(win, atom);
	if (!cls)
		return 1;

	// WM_CLASS is formatted differently from other strings, it's two null terminated strings.
	// Since we want the first one, let's just let copy run strlen.
	const char *val = (const char *)xcb_get_property_value(cls);
	if(atom == ATOM_WM_CLASS)
	{
		const char *val2 = val + strlen(val) + 1;
		if(!strcmp(val2, str))
		{
			free(cls);
			return 0;
		}
	}
	ret = strcmp(val,str);
	free(cls);
	return ret;
}

int xcomp_window_atom_compare_win(xcb_window_t win, xcb_atom_t atom, Window wnd)
{

	int ret;
	xcb_get_property_reply_t *cls =
		xcomp_property_sync(win, atom);
	if (!cls)
		return 1;

	// WM_CLASS is formatted differently from other strings, it's two null terminated strings.
	// Since we want the first one, let's just let copy run strlen.
	ret =  (wnd != *(xcb_window_t*)xcb_get_property_value(cls));
	free(cls);
	return ret;
}

Window xcomp_find_top_level_window(xcb_atom_t atom, const char *value, int index)
{
	Window ret = 0;
	int skip = 0;
	Window wnd = atoi(value);
	if(wnd)
		printf("Integer window %x\n", wnd);
	else
		printf("String atom %s\n", value);

	// EWMH top level window listing is not supported.
	if (ATOM__NET_CLIENT_LIST == XCB_ATOM_NONE)
		return 0;

	xcb_screen_iterator_t screen_iter =
		xcb_setup_roots_iterator(xcb_get_setup(connection));
	for (; screen_iter.rem > 0; xcb_screen_next(&screen_iter)) {
		xcb_generic_error_t *err = NULL;
		// Read properties up to 4096*4 bytes
		xcb_get_property_cookie_t cl_list_cookie =
			xcb_get_property(connection, 0, screen_iter.data->root,
					 ATOM__NET_CLIENT_LIST, 0, 0, 4096);
		xcb_get_property_reply_t *cl_list =
			xcb_get_property_reply(connection, cl_list_cookie, &err);
		if (err != NULL) {
			goto done;
		}

		uint32_t len = xcb_get_property_value_length(cl_list) /
			       sizeof(xcb_window_t);
		for (uint32_t i = 0; i < len; i++)
		{
			xcb_window_t win = ((xcb_window_t *)xcb_get_property_value(cl_list))[i];

			if((wnd && !xcomp_window_atom_compare_win(win, atom, wnd)) || !xcomp_window_atom_compare_str(win, atom, value) )
			{
				if(skip++ == index)
				{
					ret = win;
					goto done;
				}
			}
			
		}

	done:
		free(cl_list);
	}

	return ret;
}

Window xcomp_find_root_window(xcb_atom_t atom, Window atom_value, int index, int *x, int *y, int *width, int *height)
{
	Window *tree = NULL;
	unsigned int ntree = 0;
	Window root = gRoot, nwin = gRoot;
	int i;
	int iwin = 0;

	XQueryTree(gDisplay, gRoot, &root, &nwin, &tree, &ntree);
	if(!tree)
		return 0;
	nwin = 0;
	for(i = 0; i < ntree; i++)
	{
		XWindowAttributes attrs;
		XGetWindowAttributes(gDisplay, tree[i], &attrs);
		if( attrs.map_state == IsViewable && !xcomp_window_atom_compare_win(tree[i], atom, atom_value))
		{
		printf("dump %x\n", tree[i]);
		if(iwin++ == index)
		{
			nwin = tree[i];
			*x = attrs.x;
			*y = attrs.y;
			*width = attrs.width;
			*height = attrs.height;
			break;
		}
		}
	}
	XFree(tree);
	return nwin;
}

static int X11ErrorHandler(Display * d, XErrorEvent * e)
{
	char buffer[256];
	gLastXError = e->error_code;
	XGetErrorText(d, e->error_code, buffer, sizeof(buffer)-1 );
	fprintf(stderr, "X11 Error: %d %d %d %x, %s\n", e->error_code, e->request_code, e->minor_code, e->resourceid, buffer );
	return 0;
}

void xcomp_init_display(void)
{
	XInitThreads();
	gDisplay = XOpenDisplay(NULL);
	XSetErrorHandler(X11ErrorHandler);
	gScreen = DefaultScreen(gDisplay);
	gRoot = DefaultRootWindow(gDisplay);

    connection = XGetXCBConnection(gDisplay);//xcb_connect(NULL, NULL);
    xcb_generic_error_t *err = NULL;

    xcb_composite_query_version_cookie_t comp_ver_cookie = xcb_composite_query_version(connection, 0, 2);
    xcb_composite_query_version_reply_t *comp_ver_reply = xcb_composite_query_version_reply(connection, comp_ver_cookie, &err);
    if (comp_ver_reply)
    {
        if (comp_ver_reply->minor_version < 2) {
            fprintf(stderr, "query composite failure: server returned v%d.%d\n", comp_ver_reply->major_version, comp_ver_reply->minor_version);
            free(comp_ver_reply);
            return;
        }
        free(comp_ver_reply);
    }
    else if (err)
    {
        fprintf(stderr, "xcb error: %d\n", err->error_code);
        free(err);
        return;
    }

    const xcb_setup_t *setup = xcb_get_setup(connection);
    xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = screen_iter.data; // unused?
    BindTexImageEXT = (void*)glXGetProcAddress("glXBindTexImageEXT");
	GLXFBConfig *configs = NULL;
	int i = 0;
	int visual_attribs_layered[] = {
			GLX_RENDER_TYPE, GLX_RGBA_BIT,
			GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
			GLX_DOUBLEBUFFER, True,
			GLX_RED_SIZE, 8,
			GLX_GREEN_SIZE, 8,
			GLX_BLUE_SIZE, 8,
			GLX_ALPHA_SIZE, 8,
			GLX_DEPTH_SIZE, 24,
			None
			};
	configs = glXChooseFBConfig(gDisplay, gScreen, visual_attribs_layered, &i);
	gFbConfig = configs[0];
	xcomp_gather_atoms();
}

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

int xcomp_register_window(struct compwindow_data *data, const char *atom, const char *value, int index)
{
	if(!atom[0])
		data->w = atoi(value);
	else
		data->w = xcomp_find_top_level_window(get_atom(atom), value, index);
	if(!data->w)
	{
		strcpy(data->title, "No window");
		return 0;
	}
    int win_h, win_w, win_d;
    xcb_generic_error_t *err = NULL, *err2 = NULL;
    xcb_get_geometry_cookie_t gg_cookie = xcb_get_geometry(connection, data->w);
    xcb_get_geometry_reply_t *gg_reply = xcb_get_geometry_reply(connection, gg_cookie, &err);
    if (gg_reply) {
        win_w = gg_reply->width;
        win_h = gg_reply->height;
        win_d = gg_reply->depth;
        data->width = win_w, data->height = win_h;
        free(gg_reply);
    } else {
        if (err) {
            fprintf(stderr, "get geometry: XCB error %d\n", err->error_code);
            free(err);
        }
		if(!data->w)
			strcpy(data->title, "Bad window");
        return data->w = 0;
    }
    strcpy(data->title, "unknown");
    xcomp_window_atom_str(data->w, ATOM__NET_WM_NAME, data->title, 255 );
	return (int)data->w;
}

int xcomp_register_popup(struct compwindow_data *data, int parent, int index)
{
//	pthread_mutex_lock(&m);
	int i = 0;
	Window w = parent;
	while((i++ <= index) && w)
		w = xcomp_find_root_window(ATOM_WM_TRANSIENT_FOR, w, 0, &data->x, &data->y, &data->width, &data->height);
	if(w)
	{
	//	pthread_mutex_unlock(&m);
		return data->w = w;
	}
//	w = parent, i = 0;
	//while((i++ <= index) && w)
	//	w = xcomp_find_root_window(ATOM_WM_TRANSIENT_FOR, w, w == parent, &data->x, &data->y, &data->width, &data->height);
	printf("popupfb %x %d\n", parent, index);
	data->w = xcomp_find_root_window(ATOM_WM_TRANSIENT_FOR, parent, index, &data->x, &data->y, &data->width, &data->height);
	//pthread_mutex_unlock(&m);
	return data->w;
}


static const int pixmap_config[] = {GLX_BIND_TO_TEXTURE_RGBA_EXT,
                             True,
                             GLX_DRAWABLE_TYPE,
                             GLX_PIXMAP_BIT,
                             GLX_BIND_TO_TEXTURE_TARGETS_EXT,
                             GLX_TEXTURE_2D_BIT_EXT,
                             GLX_DOUBLEBUFFER,
                             False,
                             GLX_Y_INVERTED_EXT,
                             GLX_DONT_CARE,
                             None};
void xcomp_reset_texture(struct compwindow_data *data)
{
	glBindTexture(GL_TEXTURE_2D, data->last_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	data->glxpixmap = 0;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	
}


void xcomp_update_texture(struct compwindow_data *data, int texture) {
	fprintf(stderr, "%d\n", data->w);
//	static int has_texture;
//	if(!has_texture) has_texture = 1;
//	else return;
	data->last_texture = texture;
	if(!data->w)
	{
		xcomp_reset_texture(data);
		return;
	}
    xcb_window_t req_win_id = data->w;
    // request redirection of window
    xcb_composite_redirect_window(connection, req_win_id, XCB_COMPOSITE_REDIRECT_AUTOMATIC);

    // create a pixmap
    xcb_pixmap_t win_pixmap = xcb_generate_id(connection);
    xcb_composite_name_window_pixmap(connection, req_win_id, win_pixmap);
	GLint attrs[] = {
		GLX_TEXTURE_FORMAT_EXT,
		GLX_TEXTURE_FORMAT_RGBA_EXT,
		GLX_TEXTURE_TARGET_EXT,
		GLX_TEXTURE_2D_EXT,
		None,
	};
	gLastXError = 0;
	data->glxpixmap = glXCreatePixmap(gDisplay, gFbConfig, win_pixmap, attrs);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		GLint const Swizzle[] = {GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA};
	if(gLastXError)
	{
		data->glxpixmap = 0;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		return;
	}
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, Swizzle[0]);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, Swizzle[1]);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, Swizzle[2]);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, Swizzle[3]);

	 
	BindTexImageEXT(gDisplay, data->glxpixmap, GLX_FRONT_LEFT_EXT, NULL);
}

#if 0

int main(int argc, char **argv)
{
	struct compwindow_data data;
	xcomp_init_display();
	printf("%x\n",xcomp_register_popup(&data, atoi(argv[1]), 0));
	printf("%x\n",xcomp_register_popup(&data, atoi(argv[1]), 1));
	printf("%x\n",xcomp_register_popup(&data, atoi(argv[1]), 2));

}
#endif