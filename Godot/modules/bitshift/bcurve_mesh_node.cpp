/*************************************************************************/
/*                    This file is part of:                              */
/*                    BITSHIFT GODOT PLUGIN                              */
/*                    http://bit-shift.io                                */
/*************************************************************************/
/* Copyright (c) 2017   Fabian Mathews.                                  */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "bcurve_mesh_node.h"
#include "bcurve_mesh.h"
#include "butil.h"
#include "scene/resources/mesh.h"
#include "scene/3d/mesh_instance.h"
#include "scene/3d/physics_body.h"
#include "core/core_string_names.h"
#include <assert.h>
	
Ref<BArcLineCurve> BCurveMeshNode::get_local_curve() {
	return local_curve;
}

void BCurveMeshNode::set_local_curve(const Ref<BArcLineCurve>& p_curve) {
	world_curve_dirty = true;
	
	if (!p_curve.is_valid()) {
		local_curve->clear_points();
		return;
	}
		
	local_curve->copy(p_curve);
}

Ref<BCurveMesh> BCurveMeshNode::get_curve_mesh() {
	return curve_mesh;
}

void BCurveMeshNode::set_curve_mesh(const Ref<BCurveMesh>& p_curve_mesh) {
	curve_mesh = p_curve_mesh; 
}

MeshInstance* BCurveMeshNode::get_mesh_instance() {
    return mesh_inst;
}

// get the curve in world space
Ref<BArcLineCurve> BCurveMeshNode::BCurveMeshNode::get_world_curve() {
	if (world_curve_dirty)
		_update();
	
	return world_curve;
}

void BCurveMeshNode::_update() {
	if (!world_curve_dirty)
		return;

	world_curve->copy(local_curve);
	world_curve->apply_transform_to_points(get_global_transform());	
	world_curve_dirty = false;
}

void BCurveMeshNode::_notification(int p_what) {
	
	if (p_what == NOTIFICATION_TRANSFORM_CHANGED) {			
		world_curve_dirty = true;
		
        if (is_visible_in_tree()) {
            /*
			if (gen_mesh_on_transform_changed) {
				generate_mesh();
            }*/

			if (get_script_instance())
				get_script_instance()->call("_transform_changed");
		}
	}
	
	/*
	if (p_what == NOTIFICATION_LOCAL_TRANSFORM_CHANGED && is_visible_in_tree()) {	
		if (get_curve().is_valid()) {			
			if (gen_mesh_on_transform_changed) {
				generate_mesh();
			}
		}
		
		if (get_script_instance())
			get_script_instance()->call("_local_transform_changed");
	}*/
}

void BCurveMeshNode::set_mesh_generation_type(int p_mesh_gen_type) {
	mesh_gen_type = (MeshGenerationType)p_mesh_gen_type;
}

int BCurveMeshNode::get_mesh_generation_type() const {
	return mesh_gen_type;
}

void BCurveMeshNode::generate_mesh() {
	if (world_curve_dirty)
		_update();
	
	// convert the mesh from world space to local space
	Ref<Mesh> mesh;
	if (mesh_gen_type == MGT_MESH)
		mesh = curve_mesh->generate_mesh(get_global_transform());
	else if (mesh_gen_type == MGT_BOUNDS)
		mesh = curve_mesh->generate_mesh_from_bounds(get_global_transform());
	
    if (mesh.is_null())
        return;

	// the mesh that is generated is in world space
	// because we need to raycast into the terrain in world space
	// 
	mesh_inst->set_transform(get_global_transform().affine_inverse());
	mesh_inst->set_mesh(mesh);

	if (get_script_instance())
		get_script_instance()->call("_mesh_generated", mesh_inst);
}

void BCurveMeshNode::generate_collision() {
    // generate collision
    BUtil::get_singleton()->delete_children(static_body);
    curve_mesh->generate_collision(static_body, Vector3());
    static_body->set_transform(get_global_transform().affine_inverse());
}

Array BCurveMeshNode::get_collision_nodes() {
    Array arr;
    arr.append(static_body);
    /*
    int cc = static_body->get_child_count();
    arr.resize(cc);
    for (int i = 0; i < cc; i++)
        arr[i] = static_body->get_child(i);
*/
    return arr;
}

/*
void BCurveMeshNode::set_generate_mesh_on_transform_changed(bool gen_mesh) {
	gen_mesh_on_transform_changed = gen_mesh;
}

bool BCurveMeshNode::get_generate_mesh_on_transform_changed() const {
	return gen_mesh_on_transform_changed;
}*/

Dictionary BCurveMeshNode::closest_distance_to_curve(const Vector3& p_world_pos, float p_distance_max) const {
	Transform t = get_global_transform();
	Vector3 local_pos = t.xform_inv(p_world_pos); // convert from world space to local space
	Dictionary d = curve_mesh->get_arc_line_curve()->closest_distance_to(local_pos, p_distance_max);
	d["point_on_curve"] = t.xform(Vector3(d["point_on_curve"])); // convert local to world to world space
	return d;
}

Transform BCurveMeshNode::get_global_transform_from_distance_along_curve(float p_offset) {
	Transform t = get_global_transform();
    Transform t_r = curve_mesh->get_transform_from_distance_along_curve(p_offset);
	t_r = t * t_r;
	return t_r;
}

void BCurveMeshNode::_validate_property(PropertyInfo &property) const {
	if (property.name == "curve" || property.name == "curve_mesh") {
		property.usage = 0;
		return;
	}
}

void BCurveMeshNode::_local_curve_changed() {
    world_curve_dirty = true;
}

void BCurveMeshNode::_bind_methods() {
	
    ClassDB::bind_method(D_METHOD("_local_curve_changed"), &BCurveMeshNode::_local_curve_changed);

	ClassDB::bind_method(D_METHOD("get_world_curve"), &BCurveMeshNode::get_world_curve);
	
	ClassDB::bind_method(D_METHOD("get_local_curve"), &BCurveMeshNode::get_local_curve);
	ClassDB::bind_method(D_METHOD("set_local_curve"), &BCurveMeshNode::set_local_curve);
	
	ADD_PROPERTYNZ(PropertyInfo(Variant::OBJECT, "local_curve", PROPERTY_HINT_NONE, "BArcLineCurve", PROPERTY_USAGE_NOEDITOR), "set_local_curve", "get_local_curve");
	
	ClassDB::bind_method(D_METHOD("get_curve_mesh"), &BCurveMeshNode::get_curve_mesh);
	ClassDB::bind_method(D_METHOD("set_curve_mesh"), &BCurveMeshNode::set_curve_mesh);
        
    ClassDB::bind_method(D_METHOD("get_mesh_instance"), &BCurveMeshNode::get_mesh_instance);
	
	ADD_PROPERTYNZ(PropertyInfo(Variant::OBJECT, "curve_mesh", PROPERTY_HINT_NONE, "BCurveMesh", PROPERTY_USAGE_NOEDITOR), "set_curve_mesh", "get_curve_mesh");
	
	ClassDB::bind_method(D_METHOD("get_mesh_generation_type"), &BCurveMeshNode::get_mesh_generation_type);
	ClassDB::bind_method(D_METHOD("set_mesh_generation_type"), &BCurveMeshNode::set_mesh_generation_type);
	
	BIND_CONSTANT(MGT_BOUNDS);
	BIND_CONSTANT(MGT_MESH);
	
	ClassDB::bind_method(D_METHOD("generate_mesh"), &BCurveMeshNode::generate_mesh);
    ClassDB::bind_method(D_METHOD("generate_collision"), &BCurveMeshNode::generate_collision);
	
    //ClassDB::bind_method(D_METHOD("get_generate_mesh_on_transform_changed"), &BCurveMeshNode::get_generate_mesh_on_transform_changed);
    //ClassDB::bind_method(D_METHOD("set_generate_mesh_on_transform_changed"), &BCurveMeshNode::set_generate_mesh_on_transform_changed);
	
	ClassDB::bind_method(D_METHOD("closest_distance_to_curve"), &BCurveMeshNode::closest_distance_to_curve);
	ClassDB::bind_method(D_METHOD("get_global_transform_from_distance_along_curve"), &BCurveMeshNode::get_global_transform_from_distance_along_curve);

    ClassDB::bind_method(D_METHOD("get_collision_nodes"), &BCurveMeshNode::get_collision_nodes);
}

BCurveMeshNode::BCurveMeshNode() {
	local_curve = Ref<BArcLineCurve>(memnew(BArcLineCurve)); 
	local_curve->local = true; // for debugging!
    local_curve->connect(CoreStringNames::get_singleton()->changed, this, "_local_curve_changed");
	
	world_curve = Ref<BArcLineCurve>(memnew(BArcLineCurve)); 
	curve_mesh = Ref<BCurveMesh>(memnew(BCurveMesh)); 
	curve_mesh->set_arc_line_curve(world_curve);
	mesh_gen_type = MGT_MESH;
	world_curve_dirty = true;
	
	mesh_inst = memnew(MeshInstance); // should we inherit from mesh inst?
	add_child(mesh_inst);

    static_body = memnew(StaticBody);
    add_child(static_body);
	
	set_notify_local_transform(true);
	set_notify_transform(true);
    //set_generate_mesh_on_transform_changed(true);
}

BCurveMeshNode::~BCurveMeshNode() {
}
