# A list of commands an actor can receive
# to be executed one after the other
extends Spatial

enum CommandType {
	Attack,
	Walk,
	Run
}

export(bool) var draw = true;

var list = []

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.
	
func clear():
	list.clear()
	
# get position of the actor once they have finished executing the commands
func get_end_position():
	var c = list.back();
	if (c.command == CommandType.Walk):
		var num_points = c.curve.get_point_count();
		return c.curve.get_point_position(num_points - 1);
	elif (c.command == CommandType.Attack):
		return c.position;
		
	return null;
	
func walk_along_path(p_path_points, p_loop):
	var curve : Curve3D = Curve3D.new()
	for p in p_path_points:
		curve.add_point(p)
		
	list.append({
		"command": CommandType.Walk,
		"curve": curve,
		"loop": p_loop
	});
	return;
	
func attack(p_position, p_direction):
	list.append({
		"command": CommandType.Attack,
		"position": p_position,
		"direction": p_direction
	});
	return;

func _draw():
	if (draw == false):
		return;

	#set_global_position(Vector3(0, 0, 0));
	for c in list:
		if (c.command == CommandType.Walk):
			draw_curve(c.curve);
		elif (c.command == CommandType.Attack):
			draw_attack(c.position, c.direction);
			
func draw_curve(p_curve):
#	for i in range(p_curve.get_point_count()):
#		if (i > 0):
#			draw_line(p_curve.get_point_position(i - 1), p_curve.get_point_position(i), Color(255, 255, 255, 50), 2.0);
#
#		draw_circle(p_curve.get_point_position(i), 5, Color(255, 255, 255, 50));
	return;
		
			
func draw_attack(p_position, p_direction):
#	draw_line(p_position, p_position + p_direction, Color(255, 0, 0, 1));
	return;
	
func get_first_command():
	if (list.size() <= 0):
		return null;
		
	return list.front();
	
func pop_front_command():
	list.pop_front();
	#update();
