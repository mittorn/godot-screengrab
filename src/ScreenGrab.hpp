#ifndef SIMPLE_H
#define SIMPLE_H

#include <Godot.hpp>

#include <Input.hpp>
#include <Reference.hpp>
#include <Sprite.hpp>
#include <Control.hpp>

class ScreenGrab : public godot::Reference {
	GODOT_CLASS(ScreenGrab, godot::Reference)

public:
	static void _register_methods();

	void _init();

	int get_w() const;
	int get_h() const;
	void update_texture(int) const;
};

#endif // SIMPLE_H
