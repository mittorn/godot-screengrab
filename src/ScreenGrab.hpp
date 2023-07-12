#ifndef SIMPLE_H
#define SIMPLE_H

#include <Godot.hpp>
#include <Reference.hpp>

class ScreenGrab : public godot::Reference {
	GODOT_CLASS(ScreenGrab, godot::Reference)

public:
	static void _register_methods();

	void _init();

	int get_w() const;
	int get_h() const;
	void update_texture(int) const;
	void set_key_state(const godot::String keyName, int pressed) const;

};

#endif // SIMPLE_H
