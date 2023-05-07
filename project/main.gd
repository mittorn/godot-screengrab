extends Node

const ScreenGrab = preload("res://gdnative/screengrab.gdns")

onready var screengrab_instance = ScreenGrab.new()
var tex
var t = 0

func _on_Button_pressed():
	tex = ExternalTexture.new()
	print('External Texture:',tex.get_external_texture_id())
	screengrab_instance.update_texture(tex.get_external_texture_id())
	$TextureRect.texture = tex

func _input(event):
	
	if event is InputEventMouseMotion:
		print(event.position.x)
		#OS.execute('xdotool',['mousemove', '%d' % event.position.x, '%d' % event.position.y])
	if event is InputEventMouseButton:
		print(event.pressed)
		var state
		if event.pressed:
			state = 'mousedown'
		else:
			state = 'mouseup'
		#OS.execute('xdotool',[state, '1'])

func _process(delta):
	t += delta
	
	if(tex and t > 0.05):
		screengrab_instance.update_texture(tex.get_external_texture_id())
		t = 0
