#
# Copyright (C) 2017 bitshift
# http://bit-shift.io 
# 
# View LICENSE at:
# https://github.com/bit-shift-io/trains-and-things/blob/master/LICENSE.md
#

extends Node

var up = Vector3(0, 1, 0);

func _init():
	randomize()
	
	
func _ready():
	Console.register_command("apply_theme", {
		method = funcref(self, "apply_theme"),
		owner = self,
		description = "Reload and apply res://Theme/default.theme",
		args = "",
		num_args = 0
	})
	return
	
func apply_theme():
	BTheme.apply();
	return;
	
func find_parent_by_class_inheritance(node, className):
	var scriptClassName = className + ".gd";
	var p = node;
	while (p.get_name() != "root"):
		Log.debug(typeof(p));
		if (p.is_class(className) || p.is_class(scriptClassName)):
			return p;
			
		p = p.get_parent();
		
	return null;

# use this if you want to cast through multiple objects
func raycast(from, to):
	var world = get_tree().get_root().get_world_2d();
	if !world: # || !ClassDB.is_parent_class(world.get_class(), "Spatial"):
		return []; # in the menu or something
		
	var space_state = world.get_direct_space_state();
	if (!space_state):
		return []; # probably paused
	
	var results = []
	var exclusions = []
	var validCol = true
	while (validCol):
		var col = space_state.intersect_ray(from, to, exclusions, 0xFFFFFFFF, true, true);
		if (!col.empty()):
			results.append(col)
			exclusions.append(col["rid"])
		validCol = !col.empty()
		
	# I need normals and collision points, so this new method:
	# does not work!
	
#	var ray_dir = (to - from).normalized();
#	var ray_length = (to - from).length();
#	var ray = RayShape.new();
#	ray.set_length(ray_length);
#
#	var transform = Transform();
#	transform.origin = from;
#	var up = Vector3(0, 1, 0);
#	var right = up.cross(ray_dir);
#	var up2 = right.cross(ray_dir);
#	transform.basis[0] = right;
#	transform.basis[1] = up2;
#	transform.basis[2] = ray_dir;
#
#	var params = PhysicsShapeQueryParameters.new();
#	params.set_shape(ray);
#	params.set_transform(transform);
#	var results2 = space_state.intersect_shape(params, 10);

	#var t_ray_dir = transform.basis[2];
	#var t_from = transform.origin;
	#var t_to = t_from + t_ray_dir * ray_length;
	#assert(t_ray_dir == ray_dir);
	#assert(t_to == to);
	#assert(t_from == from);
	
#	if (results.size()):
#		var nothing = 0;
#		nothing += 1;
#
#	if (results2.size()):
#		var nothing = 0;
#		nothing += 1;
		
	return results
	
# use this if we are only interested in the first thing we hit
func raycastFirst(from, to, exclusions = []):
	var space_state = get_tree().get_root().get_world().get_direct_space_state()
	return space_state.intersect_ray(from, to, exclusions)
	
# get children of a tree item as an array
func getTreeItemChildren(treeItem):
	var children = []
	if (treeItem):
		var child = treeItem.get_children()
		while child != null:
			children.append(child)
			child = child.get_next()

	return children;
	
# setup the tree defaults
func init_tree_no_titles(tree, column_count):
	tree.set_hide_root(true);
	tree.create_item();
	
	tree.set_columns(column_count);
#	routeTree.set_column_title(0, "Route")
#	routeTree.set_column_titles_visible(true)
	tree.set_select_mode(tree.SELECT_ROW);
	
func init_tree_with_titles(tree, column_names):
	tree.set_hide_root(true);
	tree.create_item();
	
	tree.set_columns(column_names.size());
	
	for i in range(0, column_names.size()):
		tree.set_column_title(i, column_names[i]);
		
	tree.set_column_titles_visible(true);
	tree.set_select_mode(tree.SELECT_ROW);
	
# often we need to add or remove children on a tree item
# and we don't want them all removed and resetup as this clears any user selection
# so this helps resize a tree item children
func resize_tree_item_children(tree, treeItem, count):
	Log.assert(treeItem, "Invalid treeItem");
	
	# remove extras
	while (getTreeItemChildren(treeItem).size() > count):
		treeItem.remove_child(treeItem.get_children());
		
	# add missings
	while (getTreeItemChildren(treeItem).size() < count):
		tree.create_item(treeItem);
		
func find_selected_tree_item(tree, tree_item, column):
	for c in getTreeItemChildren(tree_item):
		if (c.is_selected(0)):
			return c;
			
	return null;
	
func arrayToString(array, token = ","):
	var r = ""
	for i in range(0, array.size()):
		if (i != 0):
			r += token
			
		r += array[i]
				
	return r

# print instance id of a node and its children
func printTree(node, var indent = 0):
	if (indent == 0):
		Log.debug("------------")
	
	var pad = ""
	for i in range(0, indent):
		pad += "    "
		
	Log.debug("%s %s - ID: %d" % [pad, node.get_name(), node.get_instance_ID()])
	for c in node.get_children():
		printTree(c, indent + 1)
		
	if (indent == 0):
		Log.debug("------------")
		
func getMasterPlayerController():
	var controllersNode = get_tree().get_root().get_node("World/Globals/Controllers")
	if (!controllersNode):
		return null
		
	var controllers = controllersNode.get_children()
	for c in controllers:
		if (c.is_network_master()):
			return c
			
	return null
	
# pack a curve for network transmission
func packCurve(curve):
    return curve.pack();
	
# unpack a curve from network
func unpackCurve(packedCurve, radius):
	var curve = newBArcLineCurve(radius);
	curve.unpack(packedCurve);
	#Log.assert(curve.radius == radius, "unpackCurve radius mismatch");
	return curve;
	
# helper method to create curves
func newBArcLineCurve(radius):
	var curve = BArcLineCurve.new();
	curve.radius = radius;
	return curve;

# convert currency to a string
func format_currency(amount : float):
	var strAmt = String(abs(amount));
	var commaStrAmt = ""
	var commaCount = ceil(strAmt.length() / 3.0)
	var endTriplet
	for i in range(0, commaCount):
		var le = 3
		var from = strAmt.length() - (3 * (i + 1))
		if (from < 0):
			le += from
			from = 0
			
		endTriplet = strAmt.substr(from, le)
		if (i != 0):
			commaStrAmt = endTriplet + "," + commaStrAmt
		else:
			commaStrAmt = endTriplet
		
	
	if (amount < 0.0):
		return "$(%s)" % commaStrAmt
		
	return "$%s" % commaStrAmt
	
# a method to reduce the code taken to setup timers!
func createTimer(owner, timeoutMethod, frequency, start, oneshot = false):
	var timer = Timer.new()
	timer.set_one_shot(oneshot)
	timer.set_wait_time(frequency)
	timer.connect("timeout", owner, timeoutMethod)
	owner.add_child(timer)
	if (start):
		timer.start()
	return timer
	
func get_audio_bus_index(name):
	for b in AudioServer.get_bus_count():
		if (AudioServer.get_bus_name(b).to_lower() == name):
			return b;
			
	return -1;
	
# gives mod a chance to modify attachment list before applying
#func mod_modify_attachment_list(controller, attachment_resource_list):
#	var mods = PluginManager.get_mods();
#	for mod in mods:
#			 attachment_resource_list = mod.modify_attachment_list(controller, attachment_resource_list);
#
#	return attachment_resource_list;

# given a list of resource paths, load them and attach them to the controller
#func load_attachment_list(controller, attachment_resource_list):
#	for a in attachment_resource_list:
#		Log.debug("Loading attachment: " + a);
#		var res = load_resource(controller, a);
#		Log.assert(res, "Failed to load: " + a);
#		if (!res):
#			return;
#
#		var inst = res.instance();
#		if (inst.has_method("apply_to_controller")):
#			inst.apply_to_controller(controller);
#		else:
#			controller.call_deferred("add_child", inst);
#
#		Log.debug("Attachment loaded: " + a + ", time taken:");
			
			
# gives mods a chance to modify resources before being loaded
#func load_resource(owner, resource_path):
#	var mods = PluginManager.get_mods();
#	for mod in mods:
#		resource_path = mod.modify_load_resource_path(owner, resource_path);
#
#	var loaded_res = ResourceLoader.load(resource_path);
#
#	for mod in mods:
#		resource_path = mod.modify_load_resource(owner, loaded_res);
#
#	return loaded_res;
	
# given a circular array, return an array with every element but shifted to start with 
# start_element
func get_circular_array_starting_with(array, start_element):
	var i = 0;
	for x in range(0, array.size()):
		if (array[i] == start_element):
			break;
			
		i += 1;
	
	var result = [];
	for x in range(i, array.size()):
		result.append(array[x]);
		
	for x in range(0, i):
		result.append(array[x]);
		
	Log.assert(result[0] == start_element, "get_circular_array_starting_with error");
	Log.assert(result.size() == array.size(), "get_circular_array_starting_with error");
	return result;
	
	
# same as above, but circles backwards over the array
func get_circular_array_reverse_starting_with(array, start_element):
	var arr = get_circular_array_starting_with(array, start_element);
	arr = reverse_array(arr);
	var back = arr.pop_back();
	Log.assert(back == start_element, "get_circular_array_reverse_starting_with error");
	arr.push_front(start_element);
	return arr;
	
func reverse_array(array):
	var result = [];
	for i in range(0, array.size()):
		result.push_front(array[i]);
		
	return result;
	
func set_shortcut_key(button, scancode):
	var hotkey = InputEventKey.new();
	hotkey.scancode = scancode;
	
	var shortcut = ShortCut.new();
	shortcut.set_shortcut(hotkey);
	
	# and then on BaseButton
	button.set_shortcut(shortcut);
	return;
	
# to populate option button (drop down box) with array
func populate_option_button(button, array):
	button.clear();
	for item in array:
		button.add_item(item);
	return;	
	
func json_parse_file(path):
	var file = File.new();
	file.open(path, file.READ);
	var content = file.get_as_text();
	file.close();
	return parse_json(content);
	
func get_first_animation(animation_player):
	var anim_name = animation_player.get_animation_list()[0];
	return animation_player.get_animation(anim_name);
	
# remove this node, and all its children who are in the group "group_name"
func remove_from_group_and_all_children(node, group_name):
	if (node.is_in_group(group_name)):
		node.remove_from_group(group_name);
	
	# ensure children do not save themselves
	var node_list = BUtil.find_children_by_class_name(node, "Node");
	for node in node_list:
		if (node.is_in_group(group_name)):
			node.remove_from_group(group_name);
			
	return;
			
# temporary: https://github.com/godotengine/godot/issues/15524#issuecomment-358815250
func set_click_mask_from_normal_alpha(texture_button):
	return BUtil.set_click_mask_from_normal_alpha(texture_button);

