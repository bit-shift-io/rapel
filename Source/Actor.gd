extends KinematicBody2D

signal state_changed(state);

export(bool) var debug = false;

var speed = 2 # pixel/second

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

onready var move_target = get_global_position();

func _ready():
	add_to_group("actors");
	
	get_tree().set_debug_navigation_hint(true);
	
	map_floor.update_dirty_quadrants();
	
	set_process_input(false)
	set_physics_process(true)
	
	character.set("playback/curr_animation", "Idle");
	character.play_from_time(0.0);
	
	set_state(State.Idle);
	
	# deparent the path from this actor and reparent to scene root so the follow 2D node can be used
	path.get_parent().remove_child(path);
	get_tree().get_root().call_deferred("add_child", path);
	
	
func _draw():
	if (!debug):
		return;
		
	draw_circle(move_target, 5, Color(255, 0, 0, 1));
	draw_line(move_target, get_global_position(), Color(255, 0, 0, 1));
	
	
func _physics_process(delta):
	if (state == State.Idle):
		return
		
	# reached end?
	if (!path_follow.has_loop() && path_follow.get_unit_offset() >= 1.0):
		clear_path();
		set_state(State.Idle);
		return;
		
	var current_pos = get_global_position();
	path_follow.set_offset(path_follow.get_offset() + speed);
	var new_pos = path_follow.get_global_position();
	
	if (debug):
		print("path offset:" + str(path_follow.get_offset()) + " len: " + str(path.get_curve().get_baked_length()));
	
	var delta_pos = new_pos - current_pos;
	
	move_target = new_pos;
	
	# boil movement down to a direction
	var dir = delta_pos.normalized();
	
	if (debug):
		print("delta_pos:" + str(delta_pos));
	
	# make character face the direction we are moving
	character.flipX = (delta_pos.x < 0);
	
	#delta_pos = Vector2(1, 0) * 10;
	
	# move the physics/actor
	#set_global_position(new_pos);
	
	var result = move_and_collide(dir * speed); # move_and_slide?
	
	if (debug):
		print("result:" + str(result));

	update();
		
func clear_path():
	path.set_curve(Curve2D.new())
	path.update();
			
func set_path_points(p_path_points, p_loop):
	var curve : Curve2D = Curve2D.new()
	for p in p_path_points:
		curve.add_point(p)
	
	path.set_curve(curve)
	path.update(); # redraw
	
	path_follow.set_loop(p_loop);
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
		
	emit_signal("state_changed", state);
	
func show_path(p_show):
	#path.draw = p_show;
	path.visible = p_show;
		