#
# Copyright (C) 2017 bitshift
# http://bit-shift.io 
# 
# View LICENSE at:
# https://github.com/bit-shift-io/trains-and-things/blob/master/LICENSE.md
#

extends Camera

export(float) var height_from_terrain = 50.0; # dont allow terrain any closer to terrain than this

# zoom settings
var zoom_prev = 0.0; 
export(float) var zoom = 0.0;
var zoom_step = 200.0;

# keyboard pan settings
var pan_keyboard_speed = 20.0;
var pan_position_prev = Vector3();
export(Vector3) var pan_position = Vector3();

# mouse grab pan settings
var pan_mouse_grab_active = false;
var pan_mouse_grab_raycast_world_position = Vector3();

var follow_node = WeakRef.new();

onready var tween = get_node("Tween");
onready var pan_animation_player = get_node("PanAnim"); 
onready var input_actions = get_node("InputActions");

var controller;

signal follow_node_changed();

func apply_to_controller(p_controller):
	controller = p_controller;
	
	if (!controller.is_network_master()):
		return;
		
	controller.add_camera(self);
	return;
	
func set_follow_node(node):
	if (node == null):
		follow_node = WeakRef.new();
		emit_signal("follow_node_changed");
		return;
		
	follow_node = weakref(node);
	emit_signal("follow_node_changed");
	return;
	
func get_follow_node():
	return follow_node.get_ref();

# Helper functions
func move_towards(p_current, target, maxdelta):
    if abs(target - p_current) <= maxdelta:
        return target
    return p_current + sign(target - p_current) * maxdelta

func _ready():
	set_process_input(true); # this is required as it just tells us when some external class disables input on us
	#set_process_unhandled_input(true);
	set_process(true);
	
	#Log.debug("start origin: " + String(transform.origin));

	setup_keyboard_pan();
	input_actions.add_actions(["move_forward", "move_backward", "move_left", "move_right"], input_actions.HM_UNHANDLED);
	#input_actions.add_actions(["zoom_in", "zoom_out", "pan"], input_actions.HM_HANDLED);
	var action = input_actions.add_action("pan", input_actions.HM_HANDLED);
	action.connect("is_pressed", self, "start_mouse_grab_pan");
	action.connect("is_released", self, "end_mouse_grab_pan");
	
	action = input_actions.add_action("zoom_in", input_actions.HM_HANDLED);
	action.connect("is_pressed", self, "zoom_in");
	
	action = input_actions.add_action("zoom_out", input_actions.HM_HANDLED);
	action.connect("is_pressed", self, "zoom_out");

func zoom_in():
	animate_zoom(1.0);
	
func zoom_out():
	animate_zoom(-1.0);
	
#func _input(event):
#	if (get_follow_node() == null):
#		return;
#
#	if (event is InputEventKey):
#		set_follow_node(null);
#
#	# any mouse press except zoom (4 & 5)
#	#if (event is InputEventMouseButton && event.get_button_index() != 4 && event.get_button_index() != 5):
#	#	set_follow_node(null);

func _process(delta):
	if (!is_current()):
		return;
		
	# should this just be a is_active ?
	if (Console.is_console_open()):
		return;
		
	process_mouse_zoom(delta);
	
	process_follow_node();
	process_keyboard_pan(delta);
	process_mouse_grab_pan(delta);
		
	terrain_col_check();
	#Log.debug("origin: " + String(transform.origin));
	
	#input_actions.clear();
	
func clear_follow_node():
	if (follow_node.get_ref() != null):
		follow_node = WeakRef.new();
		emit_signal("follow_node_changed");
	
func start_mouse_grab_pan():
	if (controller.mouse_over_ui):
		return;
		
	if (!BUtil.get_terrain()):
		return;
	
	# get the grab point by raycasting the terrain
	# this then becomes the grab point we always want under the mouse
	var screen_mouse_pos = get_viewport().get_mouse_position()
	var world_from = project_ray_origin(screen_mouse_pos)
	var world_to = world_from + project_ray_normal(screen_mouse_pos) * 10000
		
	var col = BUtil.get_terrain().raycast(world_from, world_to);
	if (!col.empty()):
		pan_mouse_grab_active = true;
		pan_mouse_grab_raycast_world_position = col["position"];
		clear_follow_node(); # I want to break free!
	
func end_mouse_grab_pan():
	pan_mouse_grab_active = false;
	
func process_mouse_grab_pan(delta):
	if (!pan_mouse_grab_active):
		return;
		
	# what this method does is grabs the world position the user clicked on
	# and then casts that down to a ground plane at 0 height
	# we also do the same with the mouse position on screen
	# then we have the two points projected onto the ground plane and and compute the movement
	# in the X/Z plane and just apply that movement to the camera
	
	#var screen_target_pos = unproject_position(pan_mouse_grab_raycast_world_position);
	var screen_mouse_pos = get_viewport().get_mouse_position();
	var world_from = project_ray_origin(screen_mouse_pos)
	
	var ground_plane = Plane(Vector3(0, 1, 0), 0);
	var mouse_ground_intersect_point = ground_plane.intersects_ray(world_from, project_ray_normal(screen_mouse_pos));
	
	var grab_dir = pan_mouse_grab_raycast_world_position - world_from;
	var grab_position_ground_intersect_point = ground_plane.intersects_ray(world_from, grab_dir);
	
	var camera_movement = grab_position_ground_intersect_point - mouse_ground_intersect_point;
	transform.origin += camera_movement;
	

func process_mouse_zoom(delta):
	if (controller.mouse_over_ui):
		return;
		
	var zoom_delta = zoom - zoom_prev;	
	#Log.debug("zoom_delta: " + String(zoom_delta));
	
	# zoom in zooms into the mouse position
	if (zoom_delta > 0.0):
		var screen_mouse_pos = get_viewport().get_mouse_position();
		var world_mouse_dir = project_ray_normal(screen_mouse_pos);
		transform.origin += world_mouse_dir * zoom_delta * zoom_step * delta;
		
	# else, if we are zooming out, we zoom from the centre of the camera
	elif (zoom_delta < 0.0):
		transform.origin -= transform.basis[2] * zoom_delta * zoom_step * delta;

	zoom_prev = zoom;

func setup_keyboard_pan():
	# this set the keyframes to the current position of the camera
	pan_animation_player.stop();
	var anim = Util.get_first_animation(pan_animation_player);
	
	anim.track_set_key_value(0,1,transform.origin);
	anim.track_set_key_value(0,1,transform.origin);	
	pan_position = transform.origin;
	pan_position_prev = transform.origin;
	
# TODO: this should occur in the input handler
func process_keyboard_pan(delta):
	if (!is_processing_input()):
		return;
		
	# apply the pan motion
	var pan_position_delta = pan_position - pan_position_prev;	
	pan_position_prev = pan_position;
	if (pan_position_delta.length_squared() > 0):
		transform.origin += pan_position_delta;
	
	#Log.debug("pos: " + String(translation) + " orig: " + String(transform.origin));
	
	# keyboard pan
	var dir = Vector3()
	if (input_actions.is_down("move_forward")):
		dir += -transform.basis[2];
	if (input_actions.is_down("move_backward")):
		dir += transform.basis[2];
	if (input_actions.is_down("move_left")):
		dir += -transform.basis[0];
	if (input_actions.is_down("move_right")):
		dir += transform.basis[0];

	# no movement occured, so bail out
	if (dir.length_squared() <= 0.0):
		return;
		
	clear_follow_node(); # I want to break free!
		
	dir.y = 0;
	dir = dir.normalized();
	dir *= pan_keyboard_speed * delta;
	
	pan_animation_player.stop();
	var anim = Util.get_first_animation(pan_animation_player);
	var pan_position_end_value = anim.track_get_key_value(0, 1);
	
	anim.track_set_key_value(0,0,pan_position); # current position
	anim.track_set_key_value(0,1,pan_position_end_value + dir);
	pan_animation_player.play(pan_animation_player.find_animation(anim));
	
# pan to node, or zoom to node
func pan_to_node(node):
	var position = node.get_global_transform().origin;
	if (node.has_method("get_follow_node_position")):
		position = node.get_follow_node_position();
		
	var offset = compute_camera_movement_to_look_at(position);
	pan_to_position(pan_position + offset);
	return;
	
# pan to node, or zoom to node
func pan_to_position(pos):
	pan_animation_player.stop();
	var anim = Util.get_first_animation(pan_animation_player);
	anim.track_set_key_value(0,0,pan_position); # current position
	anim.track_set_key_value(0,1,pos);
	pan_animation_player.play(pan_animation_player.find_animation(anim));
	return;
	
func animate_zoom(z):
	zoom_prev = zoom;
	
	# stop the animation player, get the value at the end of the track
	# and then modify the value from there
	# set the start of the track to the current value
	var anim_player = get_node("ZoomAnim");	
	anim_player.stop();
	
	var anim_name = anim_player.get_animation_list()[0];
	var anim = anim_player.get_animation(anim_name);
	var zoom_end_value = anim.track_get_key_value(0, 1);	
	#zoom_end_value = clamp(zoom_end_value, zoom_min, zoom_max);
	
	anim.track_set_key_value(0,0,zoom);
	anim.track_set_key_value(0,1,zoom_end_value + z);
	anim_player.play(anim_name);
		
func terrain_col_check():
	if (BUtil.get_terrain()):
		var col = BUtil.get_terrain().raycast_down(transform.origin);
		if (!col.empty()):
			var pos = col["position"];
			pos.y += height_from_terrain;
			if (pos.y > transform.origin.y):
				transform.origin.y = pos.y;
				
	return;
		
# return true if we are following a node
func process_follow_node():
	var node = follow_node.get_ref();
	if (node == null):
		return false;

	var position = node.get_global_transform().origin;
	if (node.has_method("get_follow_node_position")):
		position = node.get_follow_node_position();

	var offset = compute_camera_movement_to_look_at(position);
	var target = transform.origin + offset;
	transform.origin += offset;
	return true;

# create a plane at the position the user wants us to look at
# then, make a ground plane out of this
# then project the camera's normal through this plane
# then we compute the x, z movement and apply it to the camera
#
# TODO: move to util class?
func compute_camera_movement_to_look_at(position):
	var centrePos = get_viewport().get_size() * 0.5 #.get_rect().size * 0.5
	var from = project_ray_origin(centrePos)
	
	var normal = Util.up;
	var plane = Plane(normal, normal.dot(position));
	var intersectPt = plane.intersects_ray(from, project_ray_normal(centrePos));
	
	# ops, we went through the plane we are raycasting into, so cast backwards
	if (intersectPt == null):
		intersectPt = plane.intersects_ray(from, -project_ray_normal(centrePos));
		
	var offset = position - intersectPt;
	return offset;
	
