extends Spatial

onready var actor = $"Actor";
onready var camera = $"Target";

onready var play_pause_button = $"CanvasLayer/PlayPauseButton";

var stop_tex = load("res://stop.png");
var play_tex = load("res://play.png");

var mouse_over_ui = false; # from Trains & Things

func _ready():
	Log.debug("Loading PlayerController prefabs");
	add_to_group("player_controllers");
	add_to_group("controllers");
	
	camera.controller = self;
	# modifiers or attachments for the player controller
	# commented out attachments are WIP
#	var attachments = [
#		"res://Prefab/PlayerController/StrategyCamera.tscn"
#		];
#
#	attachments = Util.mod_modify_attachment_list(self, attachments);
#	Util.load_attachment_list(self, attachments);			
#	Log.debug("Finished PlayerController prefabs");
	
	actor.show_path(true); # only for players
	
	set_process_input(true)
	
	GameState.connect("change_phase", self, "_change_phase");
	_change_phase(GameState.get_phase()); # pause anims
	
	actor.connect("command_list_completed", self, "_command_list_completed");
	actor.connect("notify_set_targetted", self, "_notify_set_targetted");
	
	# for now, reparent controller to the actor
	#self.get_parent().remove_child(self);
	#actor.call_deferred("add_child", self);
	
	camera.get_parent().remove_child(camera);
	actor.call_deferred("add_child", camera);
	
func _notify_set_targetted(p_actor, p_targetting):
	# when targetted, pause
	# when untargetted, unpause?
	if (actor.is_targetted()):
		GameState.set_phase(GameState.Phase.Plan);
	else:
		GameState.set_phase(GameState.Phase.Execute);
	
func _command_list_completed():
	if (actor.is_targetted()):
		GameState.set_phase(GameState.Phase.Plan);
	return;
	
func _change_phase(p_phase):
	if (p_phase == GameState.Phase.Plan):
		play_pause_button.set_normal_texture(stop_tex);
	else:
		play_pause_button.set_normal_texture(play_tex);
		
func _process(delta):
	return;
		
func _unhandled_input(event):
	if event is InputEventMouseButton and event.pressed:
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
					#command_list.update();

					if (GameState.get_phase() == GameState.Phase.Execute):
						actor.execute_command(command_list.get_first_command());
					return;

		# no enemy, so must be a move command...
		# ground collision should be the last thing we collide with
		var ground_col = col_list.back();
		if (!ground_col):
			return; # user clicked in nothingness
			
		var ground_pos = ground_col.position;

		var actor_pos = actor.get_global_position();
		var move_to_pos = ground_pos;

		var path_points = NavigationMgr.get_simple_path(actor_pos, move_to_pos);

		# if no path (path finding is broken atm) just go in a straight line for now
		#if (len(path_points) <= 0):
		#	path_points = [actor_pos, move_to_pos];

		print(path_points)

		var command_list = actor.get_command_list();
		command_list.clear();
		command_list.walk_along_path(path_points, false);
		#command_list.update();

		if (GameState.get_phase() == GameState.Phase.Execute):
			actor.execute_command(command_list.get_first_command());
	return;
		
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
		
# mouse picking by raycasting into the screen	
# this will cast through objects and so retusn an array, sorted by distance from camera
func mouse_pick():
	var mousePos = get_viewport().get_mouse_position()
	var from = camera.project_ray_origin(mousePos)
	var to = from + camera.project_ray_normal(mousePos) * 10000
	
	var colList = Util.raycast(from, to);
		
	if (colList.size()):
		pass;
	#if (colList.size()):
		#var xform = Transform();
		#xform.origin = colList[0]["position"];
		#get_node("Cursor").set_global_transform(xform);
		#Log.debug("Hit phss object: " + String(xform.origin));
	
	
#	if (BUtil.get_terrain()):
#		var col = BUtil.get_terrain().raycast(from, to);
#		if (!col.empty()):
#			colList.append(col);
#			var xform = Transform();
#			xform.origin = col["position"];
#
#			xform = BUtil.get_terrain().move_transform_to_ground(xform);
#			get_cursor().set_global_transform(xform);
#			#Log.debug("Hit terrain: " + String(xform.origin));
	
	return colList;
	
func is_enemy(p_other_actor):
	if (p_other_actor != actor):
		return true;
	
	return false;

func _on_PlayPauseButton_pressed():	
	if (GameState.get_phase() == GameState.Phase.Execute):
		GameState.set_phase(GameState.Phase.Plan);
	else:
		GameState.set_phase(GameState.Phase.Execute);
