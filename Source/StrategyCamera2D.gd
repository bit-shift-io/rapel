extends Camera2D

var first_distance =0
var events={}
var percision = 10
var current_zoom
var maximum_zoomin = Vector2(0.5,0.5)
var minimum_zoomout = Vector2(3,3)
var center

func _ready():
    set_process_unhandled_input(true)
    pass

func is_zooming():
    return events.size()>1

func dist():
    var first_event =null
    var result 
    for event in events:
        if first_event!=null:
            result = events[event].pos.distance_to(first_event.pos)
            break
        first_event = events[event]
    return result

func center():
    var first_event =null
    var result 
    for event in events:
        if first_event!=null:
            result = (map_pos(events[event].pos) + map_pos(first_event))/2
            break
        first_event = events[event].pos
    return result

func map_pos(pos):
    var mtx = get_viewport().get_canvas_transform()
    var mt = mtx.affine_inverse()
    var p = mt.xform(pos)
    return p 
	
func _unhandled_input(event):
	
	# for PC
	if event is InputEventMouseButton and event.is_pressed():
		# zoom in
		if event.button_index == BUTTON_WHEEL_DOWN:
			var zoom_pos = get_global_mouse_position()
			var z = Vector2(1.5,1.5)*get_zoom()
			set_zoom(z)
		# zoom out
		if event.button_index == BUTTON_WHEEL_UP:
			var zoom_pos = get_global_mouse_position()
			var z = Vector2(1.0/1.5,1.0/1.5)*get_zoom()
			set_zoom(z)
			
	# touch
	if event is InputEventScreenTouch and event.is_pressed():
		events[event.index]=event
		
		if events.size()>1:
			current_zoom=get_zoom()
			first_distance = dist()
			center = center()
	elif event is InputEventScreenTouch and not event.is_pressed():
		events.erase(event.index)
	elif event is InputEventScreenDrag:
		events[event.index] = event
		
		if events.size()>1:
			var second_distance = dist()
			if abs(first_distance-second_distance)>percision:
				var new_zoom =Vector2(first_distance/second_distance,first_distance/second_distance)
				var z = new_zoom*current_zoom
				if z<minimum_zoomout and z>maximum_zoomin:
					set_zoom(z)
				elif zoom>minimum_zoomout:
					set_zoom(minimum_zoomout)
				elif z<maximum_zoomin:
					set_zoom(maximum_zoomin)
				set_global_position(center)
		elif events.size()==1:
			set_global_position(get_global_position()-event.relative*get_zoom()*2)