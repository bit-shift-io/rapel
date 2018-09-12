extends KinematicBody2D

var is_moving = false
var move_to_pos = Vector2()
var speed = 200 # pixel/second

var movement_tolerance = 2.0 # how close do we need to be to mouse pos before we stop moving

onready var character = $"Character"

onready var map_floor = $"../Navigation2D/floor";
onready var navigation = $"../Navigation2D";

var path = []


func _ready():
	get_tree().set_debug_navigation_hint(true);
	
	map_floor.update_dirty_quadrants();
	
	set_process_input(true)
	set_physics_process(true)
	
	character.set("playback/curr_animation", "Idle");
	character.play_from_time(0.0);
	
func _physics_process(delta):
	if is_moving:
		var direction = move_to_pos - get_global_position()

		if abs(direction.length()) < movement_tolerance:
			#we're at the position we want to be
			is_moving = false
			
			character.set("playback/curr_animation", "Idle");
			character.play_from_time(0.0);
			return # stop here, so we don't move any further
				
		direction = direction.normalized()
		move_and_slide(direction * speed)
#        if is_colliding():
#            # do all your collision stuff here
#            pass

func _input(event):
#	if event is InputEventMouseMotion:
#		print("char is at: " + str(get_global_position()))
#		print("global mouse pos is at: " + str(event.get_global_position()));
#		print("local mouse pos is at: " + str(event.get_global_position()));
#		print("canvas global mouse pos is at: " + str(get_global_mouse_position()));
		
	if event is InputEventMouseButton and event.pressed:
		move_to_pos = get_global_mouse_position() #event.get_global_position()
		event
		is_moving = true
		
		var direction = move_to_pos - get_global_position()
		character.flipX = (direction.x < 0);
		character.set("playback/curr_animation", "Run");
		character.play_from_time(0.0);
		
		path = navigation.get_simple_path(get_global_position(), move_to_pos);
		print(path)
		
func _draw():
	if not(path==null):
		for i in path:
			draw_circle(i-get_global_position(),3,Color(255,0,0))
		