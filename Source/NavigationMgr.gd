#
# Global navigation manager
#
extends Navigation

func _ready():
	pass

func add_nav_mesh_from_node(mesh_inst):
	var nav_mesh = NavigationMesh.new();
	nav_mesh.create_from_mesh(mesh_inst.mesh);
	
	var nav_mesh_inst = NavigationMeshInstance.new();
	nav_mesh_inst.set_navigation_mesh(nav_mesh);
	
	add_child(nav_mesh_inst);