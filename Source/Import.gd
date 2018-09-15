#
# Copyright (C) 2017 bitshift
# http://bit-shift.io 
# 
# View LICENSE at:
# https://github.com/bit-shift-io/trains-and-things/blob/master/LICENSE.md
#

tool

extends EditorScenePostImport

# ensure the Materials "Location" is set to "Mesh"
# and that "Storage" is set to "Built-In"

var missing_material_name = "res://Missing_M.tres";
var material_dir = "res://Material/";

func post_import(scene):
	print("Importing:" + get_source_file());
	iterate_scene(scene);
	return scene; # scene contains the imported scene starting from the root node

func iterate_scene(node):
	if (node is MeshInstance):
		print("MeshInstance found:" + node.get_name());
		var mesh = node.get_mesh();
		var surf_count = mesh.get_surface_count();
		for i in range(0, surf_count):
			var mat = mesh.surface_get_material(i);
			if (mat == null):
				print("Material is null in mesh:" + str(i));
				var new_mat = load(missing_material_name);
				mesh.surface_set_material(i, new_mat);
			else:
				print("Material exists in mesh:" + str(i) + " - " + mat.get_name());
				
				var new_mat_path = get_source_folder() + "/" + mat.get_name() + ".tres";
				var new_mat = load(new_mat_path);
				if (new_mat == null):
					new_mat = load(material_dir + mat.get_name() + ".tres");
				else:
					print("Material found:" + new_mat_path);
				
				if (new_mat == null):
					print("Material is null:" + str(i));
					new_mat = load(missing_material_name);
						
				mesh.surface_set_material(i, new_mat);

			mat = mesh.surface_get_material(i);
			
			print("Material:" + mat.get_name());

	for c in node.get_children():
		iterate_scene(c);

	return;
