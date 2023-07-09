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
		godot::register_method("get_window_title", &CompWindow::get_window_title);
		godot::register_method("register_popup", &CompWindow::register_popup);
		xcomp_init_display();
	}

	void _init() {}
	int register_window(godot::String atom,godot::String value, int index) { return xcomp_register_window( &data, atom.utf8().get_data(),value.utf8().get_data(), index);}
	int register_popup(int parent, int index) { return xcomp_register_popup( &data, parent, index);}

	int get_w() const { return data.width;}
	int get_h() const { return data.height;}
	int get_x() const { return data.x;}
	int get_y() const { return data.y;}
	void update_texture(int id) { xcomp_update_texture(&data, id);}
	int get_id() const {return data.w;}
	godot::String get_window_title() const {return (const char*)data.title;}
	void window_activate(unsigned int flags, int x, int y) { xsend_window_activate(&data, flags, x, y);}
	void window_update_mouse(unsigned int state, int flags,  int x, int y) { xsend_window_mouse(&data, state, flags, x, y);}
	void set_key_state(const godot::String keyName, int pressed, unsigned int state){ xsend_window_keyboard(&data, lookupKeycode(keyName.utf8().get_data()), pressed, state);}

private:
	compwindow_data data;
};

#endif // COMPWINDOW_H
