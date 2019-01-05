# Look for mesh instances and convert certain ones to nav mesh
# TODO: expose a list for artists to specify the names

extends Spatial

# Called when the node enters the scene tree for the first time.
func _ready():
	var mesh_inst = null;
	var children = get_children();
	for c in children:
		print(c.get_name());
		if (c.get_name() == "Floor"):
			mesh_inst = c;

	var nav_mesh = NavigationMesh.new();
	nav_mesh.create_from_mesh(mesh_inst.mesh);
	
	var nav_mesh_inst = NavigationMeshInstance.new();
	nav_mesh_inst.set_navigation_mesh(nav_mesh);
	
	add_child(nav_mesh_inst);
	
	pass # Replace with function body.
