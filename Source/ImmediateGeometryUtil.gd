extends ImmediateGeometry

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass


func draw_line(start, end, color, width):
	var dir = (end - start).normalized();
	var right = Vector3(0, 1, 0).cross(dir); #dir.cross(Vector3(0, 1, 0));
	var halfWidth = width * 0.5;
	
	#start.y += 0.2;
	#end.y += 0.2;
	
	set_color(color);
	add_vertex(start - right * halfWidth);
	add_vertex(start + right * halfWidth);
	add_vertex(end + right * halfWidth);
	
	add_vertex(start - right * halfWidth);
	add_vertex(end + right * halfWidth);
	add_vertex(end - right * halfWidth);
	
func draw_curve(p_curve, p_color = Color(255, 255, 255, 50)):
	for i in range(p_curve.get_point_count()):
		if (i > 0):
			draw_line(p_curve.get_point_position(i - 1), p_curve.get_point_position(i), p_color, 0.2);

#		draw_circle(p_curve.get_point_position(i), 5, Color(255, 255, 255, 50));
	return;