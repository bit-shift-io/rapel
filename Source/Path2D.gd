extends Path2D
#
#export(bool) var draw = true;
#
#func _draw():
#	if (draw == false):
#		return;
#
#	# draw the path
#	var curve = get_curve();
#	for i in range(0, curve.get_point_count()):
#		draw_circle(curve.get_point_position(i), 5, Color(255, 0, 0, 1));
#		if (i > 0):
#			draw_line(curve.get_point_position(i - 1), curve.get_point_position(i), Color(255, 0, 0, 1));
