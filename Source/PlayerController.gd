extends Node2D

onready var actor = $"Actor";

func _ready():
	add_to_group("controllers");
	
	actor.show_path(true); # only for players
	
	set_process_input(true)
	
func _input(event):
	if event is InputEventMouseButton and event.pressed:
		var actor_pos = actor.get_global_position();
		var move_to_pos = get_global_mouse_position()
		
		var path_points = actor.navigation.get_simple_path(actor_pos, move_to_pos);
		
		# if no path (path finding is broken atm) just go in a straight line for now
		if (len(path_points) <= 0):
			path_points = [actor_pos, move_to_pos];
			
		print(path_points)
		actor.set_path_points(path_points, false);