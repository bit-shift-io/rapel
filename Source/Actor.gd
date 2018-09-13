extends KinematicBody2D

var is_moving = false
var move_to_pos = Vector2()
var speed = 5 # pixel/second

var movement_tolerance = 2.0 # how close do we need to be to mouse pos before we stop moving

onready var character = $"Character"

onready var map_floor = $"../../Navigation2D/floor";
onready var navigation = $"../../Navigation2D";

onready var path = $"Path2D";
onready var path_follow = $"Path2D/PathFollow2D";

enum State {
	Idle,
	Walk
}

var state;

func _ready():
	get_tree().set_debug_navigation_hint(true);
	
	map_floor.update_dirty_quadrants();
	
	set_process_input(false)
	set_physics_process(true)
	
	character.set("playback/curr_animation", "Idle");
	character.play_from_time(0.0);
	
	set_state(State.Idle);
	
	# deparent the path from this actor and reparent to scene root so the follow 2D node can be used
	path.get_parent().remove_child(path);
	#path.set_global_position(Vector2(0, 0));
	get_tree().get_root().call_deferred("add_child", path);
	
	
func _physics_process(delta):
	if (state == State.Idle):
		return
		
	var current_pos = get_global_position();
	path_follow.set_offset(path_follow.get_offset() + speed);
	var new_pos = path_follow.get_global_position();
	
	print("path offset:" + str(path_follow.get_offset()) + " len: " + str(path.get_curve().get_baked_length()));
	
	var delta_pos = new_pos - current_pos;
	print("delta_pos:" + str(delta_pos));
	
	# make character face the direction we are moving
	character.flipX = (delta_pos.x < 0);
	
	#delta_pos = Vector2(1, 0) * 10;
	
	# move the physics/actor
	#set_global_position(new_pos);
	
	var result = move_and_collide(delta_pos); # move_and_slide?
	print("result:" + str(result));
#
#	if is_moving:
#		var direction = move_to_pos - get_global_position()
#
#		character.flipX = (direction.x < 0);
#		#character.set("playback/curr_animation", "Run");
#		#character.play_from_time(0.0);
#
#		if abs(direction.length()) < movement_tolerance:
#			#we're at the position we want to be
#			is_moving = false
#
#			character.set("playback/curr_animation", "Idle");
#			character.play_from_time(0.0);
#			return # stop here, so we don't move any further
#
#		direction = direction.normalized()
#		move_and_slide(direction * speed)
		
			
func set_path_points(p_path_points):
	var curve : Curve2D = Curve2D.new()
	for p in p_path_points:
		curve.add_point(p)
	
	path.set_curve(curve)
	path.update(); # redraw
	
	path_follow.set_offset(0)
	set_state(State.Walk)
	
func set_state(p_state):
	state = p_state;
	
	if (state == State.Idle):
		character.set("playback/curr_animation", "Idle");
		character.play_from_time(0.0);
	elif (state == State.Walk):
		character.set("playback/curr_animation", "Run");
		character.play_from_time(0.0);
		