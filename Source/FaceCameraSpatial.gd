# Make this spatial act like a sprite, in that it is always oriented towards the camera
# at the moment this is locked around the Y (up) axis
extends Spatial

export(bool) var enabled = true;

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	if (!enabled):
		return;
		
	var camera = get_viewport().get_camera();
	var cxform = camera.get_camera_transform();
	
	var parent_xform = get_parent().get_global_transform();
	var xform = get_transform();
	
	var world_to_local = parent_xform.inverse();
	var local_camera_xform = world_to_local * cxform;
		
	var fwd : Vector3 = local_camera_xform.basis.z; #Vector3(0, 0, 1); 
	var up : Vector3 = Vector3(0, 1, 0);
	var right : Vector3 = up.cross(fwd).normalized();
	up = fwd.cross(right).normalized();
	
	xform.basis.x = right;
	xform.basis.y = up;
	xform.basis.z = fwd;
	
	set_transform(xform);
	pass
