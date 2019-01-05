extends Navigation

# Declare member variables here. Examples:
# var a = 2
# var b = "text"

# Called when the node enters the scene tree for the first time.
func _ready():
	var startPoint = get_node("StartPoint");
	var endPoint = get_node("EndPoint");
	var navigation = self;
	
	var path = navigation.get_simple_path(startPoint.get_translation(), endPoint.get_translation());
	print(path);
	
	var ig = get_node("ImmediateGeometry");
	ig.clear();
	ig.begin(Mesh.PRIMITIVE_LINE_STRIP);
	for p in path:
		ig.add_vertex(Vector3(p.x, 0.1, p.z));
	
	ig.end();
	pass # Replace with function body.

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
