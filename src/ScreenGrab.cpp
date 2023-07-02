#include "ScreenGrab.hpp"
#include "gdvk_util.h"
#include <GLES3/gl3.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
void ScreenGrab::_register_methods() {
	godot::register_method("update_texture", &ScreenGrab::update_texture);
	godot::register_method("get_width", &ScreenGrab::get_w);
	godot::register_method("get_height", &ScreenGrab::get_h);
	godot::register_method("set_key_state", &ScreenGrab::set_key_state);
}

static Display *dpy;
static Window win;
static XShmSegmentInfo shminfo;
static XImage *img;
static int width = 1920;
static int height = 1080;

static void InitX11()
{
	if( dpy) return;
	dpy = XOpenDisplay(NULL);
	int scr = XDefaultScreen(dpy);
	win =  RootWindow(dpy, scr);
	img = XShmCreateImage(dpy,  DefaultVisual(dpy, scr),DefaultDepth(dpy, scr), ZPixmap, NULL, &shminfo, width, height);
	shminfo.shmid = shmget(IPC_PRIVATE, img->bytes_per_line * img->height,IPC_CREAT | 0777);
	shminfo.shmaddr = img->data =(char*)shmat(shminfo.shmid, 0, 0);
	shminfo.readOnly = False;
	XShmAttach(dpy, &shminfo);
	XShmGetImage(dpy, win, img, 0, 0, AllPlanes);
	    //XGetImage(dpy, win, 0, 0, width, height, AllPlanes, ZPixmap);

}
#include "../drmsend.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#define blog(x,...) printf(__VA_ARGS__)
#define MSG printf
static int fd;
static int InitDRM()
{
	const char *sockname = "/tmp/sock";
	int retval = 0;
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	int connfd = sockfd;

	{
		struct sockaddr_un addr;
		addr.sun_family = AF_UNIX;
		if (strlen(sockname) >= sizeof(addr.sun_path)) {
			MSG("Socket filename '%s' is too long, max %d",
			    sockname, (int)sizeof(addr.sun_path));
			goto cleanup;
		}

		strcpy(addr.sun_path, sockname);
		if (-1 == connect(sockfd, (const struct sockaddr *)&addr,
				  sizeof(addr))) {
			MSG("Cannot connect to unix socket: %d", errno);
			goto cleanup;
		}
	}
	drmsend_response_t resp;
	int fb_fds[OBS_DRMSEND_MAX_FRAMEBUFFERS];
	for (;;) {
		struct msghdr msg = {0};

		struct iovec io = {
			.iov_base = &resp,
			.iov_len = sizeof(resp),
		};
		msg.msg_iov = &io;
		msg.msg_iovlen = 1;

		char cmsg_buf[CMSG_SPACE(sizeof(int) *
					 OBS_DRMSEND_MAX_FRAMEBUFFERS)];
		msg.msg_control = cmsg_buf;
		msg.msg_controllen = sizeof(cmsg_buf);
		struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len =
			CMSG_LEN(sizeof(int) * OBS_DRMSEND_MAX_FRAMEBUFFERS);

		// FIXME blocking, may hang if drmsend dies before sending anything
		const ssize_t recvd = recvmsg(connfd, &msg, 0);
		blog(LOG_DEBUG, "recvmsg = %d", (int)recvd);
		if (recvd <= 0) {
			blog(LOG_ERROR, "cannot recvmsg: %d", errno);
			break;
		}

		if (io.iov_len != sizeof(resp)) {
			blog(LOG_ERROR,
			     "Received metadata size mismatch: %d received, %d expected",
			     (int)io.iov_len, (int)sizeof(resp));
			break;
		}

		if (resp.tag != OBS_DRMSEND_TAG) {
			blog(LOG_ERROR,
			     "Received metadata tag mismatch: %#x received, %#x expected",
			     resp.tag, OBS_DRMSEND_TAG);
			break;
		}

		if (cmsg->cmsg_len !=
		    CMSG_LEN(sizeof(int) * resp.num_framebuffers)) {
			blog(LOG_ERROR,
			     "Received fd size mismatch: %d received, %d expected",
			     (int)cmsg->cmsg_len,
			     (int)CMSG_LEN(sizeof(int) *
					   resp.num_framebuffers));
			break;
		}

		memcpy(fb_fds, CMSG_DATA(cmsg),
		       sizeof(int) * resp.num_framebuffers);
		retval = 1;
		break;
	}
	godot::Godot::print("InitDRM");
	cleanup:
	close(connfd);
	fd = fb_fds[0];
	printf("fd: %d %d %d\n", fb_fds[0], resp.framebuffers[0].width, resp.framebuffers[0].height);
	width = resp.framebuffers[0].width, height = resp.framebuffers[0].height;
	return retval;
}


void ScreenGrab::_init() {
	InitDRM();

}


int ScreenGrab::get_w() const {return width;}
int ScreenGrab::get_h() const {return height;}

static bool has_texture;

void ScreenGrab::update_texture(int id) const {
	godot::Godot::print("Will try update texture");
	if(has_texture)
		return;
	if(!InitDRM())
		return;
//	return;
	has_texture = true;
	
	glBindTexture(GL_TEXTURE_2D, id);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//		    glTexParameterf(texture->target, _GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	unsigned char data[4*4*4];
//	int width = 1920, height = 1080;
	int size = width * height * 4;

//	memset(data, 0x80, sizeof(data));
//	XImage *img = XGetImage(dpy, win, 0, 0, width, height, AllPlanes, ZPixmap);
//	XShmGetImage(dpy, win, img, 0, 0, AllPlanes);
	GLint const Swizzle[] = {GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA};
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, Swizzle[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, Swizzle[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, Swizzle[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, Swizzle[3]);
	GLuint memObject = 0;

    glCreateMemoryObjectsEXT(1, &memObject);

    GLint dedicated = GL_TRUE;
    glMemoryObjectParameterivEXT(memObject, GL_DEDICATED_MEMORY_OBJECT_EXT, &dedicated);
    glGetMemoryObjectParameterivEXT(memObject, GL_DEDICATED_MEMORY_OBJECT_EXT, &dedicated);
    glImportMemoryFdEXT(memObject, size, GL_HANDLE_TYPE_OPAQUE_FD_EXT, fd);
    glTexStorageMem2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, width , height, memObject, 0);
    glDeleteMemoryObjectsEXT(1, &memObject);
 //   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->data);

//	XDestroyImage(img);
}

void ScreenGrab::set_key_state(const godot::String keyName, int pressed) const{
	setKeyState(lookupKeycode(keyName.utf8().get_data()), pressed);
}



extern "C" void gprint(const char *str)
{
	godot::Godot::print(str);
}