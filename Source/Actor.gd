extends KinematicBody2D

const CommandList = preload("CommandList.gd");

signal state_changed(state);
signal command_list_completed();
signal notify_set_targetted(actor, targetting);

export(bool) var debug = false;

var speed = 2 # pixel/second

var movement_tolerance = 2.0 # how close do we need to be to mouse pos before we stop moving

onready var character = $"Character"
onready var character2 = $"Viewport/Char3D"
onready var character_sprite = $"Viewport_Sprite"

onready var map_floor = $"../../Navigation2D/floor";
onready var navigation = $"../../Navigation2D";

onready var path = $"Path2D";
onready var path_follow = $"Path2D/PathFollow2D";

onready var weapon = BUtil.find_first_child_by_class_name(self, "Weapon");

var health = 20;

var target_of = {}

enum State {
	Idle,
	Walk,
	Attack,
	Flinch,
	Dying,
	Dead,
}

var state;

onready var command_list = $"CommandList";

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
	
	GameState.connect("change_phase", self, "_change_phase");
	_change_phase(GameState.get_phase()); # pause anims
	
	# deparent the path from this actor and reparent to scene root so the follow 2D node can be used
	path.get_parent().remove_child(path);
	get_tree().get_root().call_deferred("add_child", path);
	
	command_list.get_parent().remove_child(command_list);
	get_tree().get_root().call_deferred("add_child", command_list);
	
	# reparent weapon to L_Hand
	# https://github.com/sanja-sa/gddragonbones/issues/23
	#var l_hand = BUtil.find_child(character, "L_Hand");
	#weapon.get_parent().remove_child(weapon);
	#l_hand.call_deferred("add_child", weapon);
	
	
func _change_phase(p_phase):
	if (p_phase == GameState.Phase.Plan):
		#character.stop_all();
		character.set("playback/speed", 0.0);
	else:
		#character.play(true);
		character.set("playback/speed", 1.0); # TODO: restore speed of when pasued
			
		# start following the commandlist
		if (command_list.get_first_command()):
			execute_command(command_list.get_first_command());
		
func execute_command(c):
	if (c == null):
		# go to idle
		#if (is_alive() && state != State.Flinch):
		#	set_state(State.Idle)
		
		# TODO: notify controller, which will then pause the game if player controller
		emit_signal("command_list_completed");
		return;
		
	# if we are in a state that can't be interupted, call
	# execute_command again later
	if (state == State.Flinch):
		call_deferred("execute_command", c);
		return;
		
	if (c.command == CommandList.CommandType.Walk):
		path.set_curve(c.curve)
		#path.update(); # redraw
		
		path_follow.set_loop(c.loop);
		path_follow.set_offset(0)
		set_state(State.Walk)
	elif (c.command == CommandList.CommandType.Attack):
		character.flipX = (c.direction.x < 0);
		character_sprite.set_flip_h(c.direction.x < 0);
		set_state(State.Attack)
		return;
		
	return;
	
func execute_next_command():
	command_list.pop_front_command();
	execute_command(command_list.get_first_command());
	
func _draw():
	if (!debug):
		return;
		
	return;
	draw_circle(move_target, 5, Color(255, 0, 0, 1));
	draw_line(move_target, get_global_position(), Color(255, 0, 0, 1));
	
func _physics_process(delta):
	if (GameState.get_phase() == GameState.Phase.Plan):
		return;
		
	if (state == State.Idle):
		pass
	elif (state == State.Walk):
		_process_state_walk(delta)
	elif (state == State.Attack):
		_process_state_attack(delta)
	elif (state == State.Dying):
		_process_state_dying(delta)
	elif (state == State.Dead):
		pass
	elif (state == State.Flinch):
		_process_state_flinch(delta)
		
	return;

func _process_state_flinch(delta):
	# reached end of anim
	print("tell:" + str(character.tell()));
	print("get_progress:" + str(character.get_progress()));
	
	if (character.tell() >= 1.0):
		set_state(State.Idle);
		return;
		
	return;
	
func _process_state_dying(delta):
	# reached end of anim
	print("tell:" + str(character.tell()));
	print("get_progress:" + str(character.get_progress()));
	
	if (character.tell() >= 1.0):
		set_state(State.Dead);
		return;
		
	return;
	
func _process_state_attack(delta):
	# reached end of anim
	print("tell:" + str(character.tell()));
	print("get_progress:" + str(character.get_progress()));
	
	if (character.tell() >= 1.0):
		# do damage - ideally do at 50% way through anim?
		# TODO: raycast or get target f
		var c = command_list.get_first_command();
		if (c):
			var direction = c.direction;
			var weapon = get_weapon();
			var range_min = weapon.range_min;
			var range_max = weapon.range_max;
			var current_pos = get_global_position();
			var col_list = Util.raycast(current_pos + (direction * range_min), current_pos + (direction * range_max));
			for col in col_list:
				var a = BUtil.find_parent_by_class_name(col.collider, "Actor");
				if (a != self):
					a.damage(weapon);
					break; # how to handle hitting multiple enemies? a list of actors to hit? for now just hit the first
		else:
			print("Lost attack info!");
		
		set_state(State.Idle);
		execute_next_command();
		return;
		
	return;
	
func _process_state_walk(delta):
	# reached end?
	if (!path_follow.has_loop() && path_follow.get_unit_offset() >= 1.0):
		#clear_path();
		set_state(State.Idle);
		execute_next_command();
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
	character_sprite.set_flip_h(delta_pos.x < 0);
	
	#delta_pos = Vector2(1, 0) * 10;
	
	# move the physics/actor
	#set_global_position(new_pos);
	
	var result = move_and_collide(dir * speed); # move_and_slide?
	
	if (debug):
		print("result:" + str(result));

	#update();
		
func clear_path():
	path.set_curve(Curve2D.new())
	path.update();
			
func set_path_points(p_path_points, p_loop):
	var curve : Curve2D = Curve2D.new()
	for p in p_path_points:
		curve.add_point(p)
	
	path.set_curve(curve)
	#path.update(); # redraw
	
	path_follow.set_loop(p_loop);
	path_follow.set_offset(0)
	set_state(State.Walk)
	
func can_change_to_state(p_state):
	if (is_alive()):
		return true;
	else:
		if (p_state == State.Dying || p_state == State.Dead):
			return true;
			
	return false;
	
func set_state(p_state):
	state = p_state;
	
	if (state == State.Idle):
		play_anim("Idle", true);
		character.play_from_time(0.0);
	elif (state == State.Walk):
		play_anim("Run", true);
		character.play_from_time(0.0);
	elif (state == State.Attack):
		play_anim("Attack_Sword", false);
	elif (state == State.Dying):
		play_anim("Dying", false);
		command_list.clear(); # no more commands for you dear sir
	elif (state == State.Dead):
		character.stop_all(); # leave on last keyframe of dying
		set_collision_layer(0); # disable physics - probably better to not have the physics shape as the root, but follow the physics shape?
		set_collision_mask(0);
	elif (state == State.Flinch):
		play_anim("Flinch", false);
		
	emit_signal("state_changed", state);
	
func get_controller():
	return get_parent();
	
func show_path(p_show):
	#path.draw = p_show;
	path.visible = p_show;
	
func get_weapon():
	return weapon;
	
func get_command_list():
	return command_list;
	
func is_alive():
	return health > 0;
	
func damage(p_weapon):
	if (!is_alive()):
		return;
		
	health -= p_weapon.damage;

	if (health <= 0):
		set_state(State.Dying);
	else:
		set_state(State.Flinch);
		
	return;
	
# let this actor know we have the focus of the enemy
func set_targetted(p_actor, p_targetting):
	if (p_targetting):
		target_of[p_actor] = p_actor;
	else:
		target_of.erase(p_actor);
		
	emit_signal("notify_set_targetted", p_actor, p_targetting);
	
func is_targetted():
	return (target_of.size() > 0);
	
func play_anim(p_name, p_loop):
	character.set("playback/loop", -1 if p_loop else 1);
	character.set("playback/curr_animation", p_name);
	character.play_from_time(0.0);
		
	var anim_name = p_name + "_Armature";
	character2.get_node("AnimationPlayer").get_animation(anim_name).set_loop(p_loop);
	character2.get_node("AnimationPlayer").play(anim_name);
	
		