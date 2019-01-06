# Look for mesh instances and convert certain ones to nav mesh

extends MeshInstance

export(bool) var nav_mesh = true;
export(bool) var collision_trimesh = true;

#export(Array, String) var navmesh_names = [];

# Called when the node enters the scene tree for the first time.
func _ready():
	if (nav_mesh):
		NavigationMgr.add_nav_mesh_from_node(self);
		
	if (collision_trimesh):
		_create_trimesh_col();
	
#	var children = get_children();
#	for c in children:
#		print(c.get_name());
#		if (navmesh_names.has(c.get_name())):
#			NavigationMgr.add_nav_mesh_from_node(c);

	return

func _create_trimesh_col():
	var shape = mesh.create_trimesh_shape();

	var cshape = CollisionShape.new();
	cshape.set_shape(shape);
	
	var body = StaticBody.new();
	body.add_child(cshape);
	
	add_child(body);
