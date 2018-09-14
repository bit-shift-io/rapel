extends Node2D

onready var actor = $"Actor";
onready var camera = $"Camera2D";

onready var play_pause_button = $"CanvasLayer/PlayPauseButton";

var stop_tex = load("res://stop.png");
var play_tex = load("res://play.png");

var ignore_next_mouse_button_event = false;

func _ready():
	add_to_group("controllers");
	
	actor.show_path(true); # only for players
	
	set_process_input(true)
	
	GameState.connect("change_phase", self, "_change_phase");
	_change_phase(GameState.get_phase()); # pause anims
	
	actor.connect("command_list_completed", self, "_command_list_completed");
	
	# for now, reparent controller to the actor
	#self.get_parent().remove_child(self);
	#actor.call_deferred("add_child", self);
	
	camera.get_parent().remove_child(camera);
	actor.call_deferred("add_child", camera);
	
	
func _command_list_completed():
	GameState.set_phase(GameState.Phase.Plan);
	
func _change_phase(p_phase):
	if (p_phase == GameState.Phase.Plan):
		play_pause_button.set_texture(stop_tex);
	else:
		play_pause_button.set_texture(play_tex);
		
func _input(event):
	if event is InputEventMouseButton and event.pressed:
		if (ignore_next_mouse_button_event):
			ignore_next_mouse_button_event = false;
			return;
			
		# first check if we are attacking an enemy
		var col_list = mouse_pick();
		for i in range(col_list.size()):
			var a = BUtil.find_parent_by_class_name(col_list[i].collider, "Actor");
			if (a):
				print("collision with:" + a.get_path());
				if (is_enemy(a)):
					var actor_pos = actor.get_global_position();
					var enemy_pos = a.get_global_position();
					var dir = (enemy_pos - actor_pos).normalized();
					
					var command_list = actor.get_command_list();
					command_list.clear();
					command_list.attack(actor_pos, dir);
					command_list.update();
					return;
					
		# no enemy, so must be a move command...
			
		var actor_pos = actor.get_global_position();
		var move_to_pos = get_global_mouse_position()
		
		var path_points = actor.navigation.get_simple_path(actor_pos, move_to_pos);
		
		# if no path (path finding is broken atm) just go in a straight line for now
		if (len(path_points) <= 0):
			path_points = [actor_pos, move_to_pos];
			
		print(path_points)
		
		var command_list = actor.get_command_list();
		command_list.clear();
		command_list.walk_along_path(path_points, false);
		command_list.update();
		
		#actor.set_path_points(path_points, false);
		
#	if event is InputEventMouseMotion:
#		var col_list = mouse_pick();
#		var cursor_pos = get_global_mouse_position();
#		for i in range(col_list.size()):
#			var a = BUtil.find_parent_by_class_name(col_list[i].collider, "Actor");
#			if (a):
#				print("collision with:" + a.get_path());
#			#	if (is_enemy(a, actor)):
#			#		a.show_select_indicator();
#
#
#
#		return;
		
func mouse_pick():
	#var mousePos = get_viewport().get_mouse_position()
	var mousePos = get_global_mouse_position();
	var from = mousePos #camera.project_ray_origin(mousePos)
	var to = mousePos + Vector2(1.0, 0.0) #from + camera.project_ray_normal(mousePos) * 10000
	var colList = Util.raycast(from, to);
	return colList;
	
func is_enemy(p_other_actor):
	if (p_other_actor != actor):
		return true;
	
	return false;

func _on_PlayPauseButton_pressed():
	ignore_next_mouse_button_event = true;
	
	if (GameState.get_phase() == GameState.Phase.Execute):
		GameState.set_phase(GameState.Phase.Plan);
	else:
		GameState.set_phase(GameState.Phase.Execute);
