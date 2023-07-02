#ifndef COMPWINDOW_H
#define COMPWINDOW_H

#include <Godot.hpp>
#include <Reference.hpp>
#include "xcompgrab.h"
#include "gdvk_util.h"

class CompWindow : public godot::Reference {
	GODOT_CLASS(CompWindow, godot::Reference)

public:
	static void _register_methods()
	{
		godot::register_method("update_texture", &CompWindow::update_texture);
		godot::register_method("get_width", &CompWindow::get_w);
		godot::register_method("get_height", &CompWindow::get_h);
		godot::register_method("get_x", &CompWindow::get_x);
		godot::register_method("get_y", &CompWindow::get_y);
		godot::register_method("get_id", &CompWindow::get_id);
		godot::register_method("register_window", &CompWindow::register_window);
		godot::register_method("window_activate", &CompWindow::window_activate);
		godot::register_method("window_update_mouse", &CompWindow::window_update_mouse);
		godot::register_method("set_key_state", &CompWindow::set_key_state);
		xcomp_init_display();
	}

	void _init() {}
	int register_window(godot::String cname, int index) { return xcomp_register_window( &data, cname.utf8().get_data(), index);}

	int get_w() const { return data.width;}
	int get_h() const { return data.height;}
	int get_x() const { return data.x;}
	int get_y() const { return data.y;}
	void update_texture(int id) { xcomp_update_texture(&data, id);}
	int get_id() const {return data.w;}
	void window_activate(unsigned int flags, int x, int y) { xsend_window_activate(data.w, flags, x, y);}
	void window_update_mouse(unsigned int buttons, int flags,  int x, int y) { xsend_window_mouse(data.w, buttons, flags, x, y);}
	void set_key_state(const godot::String keyName, int pressed){ xsend_window_keyboard(data.w, lookupKeycode(keyName.utf8().get_data()), pressed);}

private:
	compwindow_data data;
};

#endif // COMPWINDOW_H
