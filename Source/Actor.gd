extends GDDragonBones

# Declare member variables here. Examples:
# var a = 2
# var b = "text"

# Called when the node enters the scene tree for the first time.
func _ready():
	
	# https://github.com/sanja-sa/gddragonbones
	set("playback/curr_animation", "Run");
	play_from_time(0.0);
	
	#var skel = self;
	#set_speed(1.0)
	#fade_in("Run", -1, -1, 2, "", GDDragonBones.FadeOut_All)
	#play_from_time(0.0);
	
	#fade_in("Run", 0.5, 1, 0, )
	
	# start play	
	#skeleton.play()

	pass # Replace with function body.

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
