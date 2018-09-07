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
#include "bcurve_mesh.h"
#include "butil.h"
#include "bmesh_gen_tool.h"
#include "scene/resources/mesh_data_tool.h"
#include "scene/3d/mesh_instance.h"
#include "scene/3d/collision_shape.h"
#include "scene/3d/physics_body.h"
#include "scene/resources/ray_shape.h"
#include "scene/resources/box_shape.h"

Ref<BArcLineCurve> BCurveMesh::get_arc_line_curve() const {
	return arc_line_curve;
}

void BCurveMesh::set_arc_line_curve(const Ref<BArcLineCurve>& p_arc_line_curve) {
	arc_line_curve = p_arc_line_curve;
}

void BCurveMesh::set_spline(const Ref<BSpline>& p_spline) {
	spline = p_spline;
}

void BCurveMesh::populate_mesh_data_tool(Node* p_mesh_inst, MeshDataTool* p_mdt) {
    ERR_FAIL_COND(!p_mesh_inst);
    ERR_FAIL_COND(!p_mdt);
    mesh_instance = Object::cast_to<MeshInstance>(p_mesh_inst); //p_mesh_inst->cast_to<MeshInstance>();
    ERR_FAIL_COND(!mesh_instance);
    ERR_FAIL_COND(!mesh_instance->get_mesh().is_valid());
    p_mdt->create_from_surface(mesh_instance->get_mesh(), 0); // 0 is the surface index
}

void BCurveMesh::set_instance_template(Node* p_instance_template) {
	mesh_instance = NULL;
	instance_template = p_instance_template;
	ERR_FAIL_COND(!instance_template);
	
	// cache some stuff
    Node* middle = instance_template->find_node("Middle");
    Node* front_cap = instance_template->find_node("FrontCap");
    Node* back_cap = instance_template->find_node("BackCap");

    populate_mesh_data_tool(middle, mesh_data_middle);
    populate_mesh_data_tool(front_cap, mesh_data_front_cap);
    populate_mesh_data_tool(back_cap, mesh_data_back_cap);

    mesh_instance = Object::cast_to<MeshInstance>(middle); //middle->cast_to<MeshInstance>();
    /*
    ERR_FAIL_COND(!middle);
    mesh_instance = middle->cast_to<MeshInstance>();
	ERR_FAIL_COND(!mesh_instance);
	ERR_FAIL_COND(!mesh_instance->get_mesh().is_valid());
    mesh_data_middle->create_from_surface(mesh_instance->get_mesh(), 0); // 0 is the surface index
    */

    // compute bounding volume - assumes front cap and back cap are just flat surfaces
    for (int v = 0; v < mesh_data_middle->get_vertex_count(); ++v) {
        Vector3 pos = mesh_data_middle->get_vertex(v);
		if (v == 0) {
			bounds_max = pos;
			bounds_min = pos;			
		}
		
		if (pos.x > bounds_max.x)
			bounds_max.x = pos.x;
		
		if (pos.x < bounds_min.x)
			bounds_min.x = pos.x;
		
		
		if (pos.y > bounds_max.y)
			bounds_max.y = pos.y;
		
		if (pos.y < bounds_min.y)
			bounds_min.y = pos.y;
		
		if (pos.z > bounds_max.z)
			bounds_max.z = pos.z;
		
		if (pos.z < bounds_min.z)
			bounds_min.z = pos.z;
	}
}

float BCurveMesh::get_curve_length() {
	if (arc_line_curve.is_valid())
		return arc_line_curve->get_baked_length();
	else if (spline.is_valid())
		return spline->get_baked_length();
	
	return 0.f;
}


void BCurveMesh::generate_collision(Node* p_parent, const Vector3& grow_vec) {
    float path_length = get_curve_length();
    float z_size = bounds_max.z - bounds_min.z;
    int required_instances = Math::ceil(path_length / z_size);

    // compute how much stretching needs to occur to each instance to cover the final distance
    // and close the gap
    float stretch = (path_length / z_size) - int(path_length / z_size);// get remainder, fmod?
    stretch *= z_size;
    // at this stage stretch = the length we have to close... so spread this over the number of instances
    stretch = 1.f + stretch / required_instances;

    //float z_offset = -bounds_max.z;
    int index_offset = 0;

    Vector3 extents = ((bounds_max - bounds_min) / 2) + grow_vec;

    for (int inst = 0; inst < required_instances; ++inst) {
            // populate vertex data
            float z = inst * z_size * stretch;

            // should the BCurve cache this when it bakes the points?
            // make it  bake transforms? can we interpolate matricies easy
            Transform t = get_transform_from_distance_along_curve(z);

            Ref<BoxShape> shape;
            shape.instance();
            shape->set_extents(extents);

            CollisionShape* col_shape = memnew( CollisionShape() );
            col_shape->set_shape(shape);
            col_shape->set_transform(t);

            p_parent->add_child(col_shape);
    }
}


// TODO: make this take params?
// This casts a ray along the track to see if it collides with anything
Array BCurveMesh::intersect_shape(int p_result_max, const Array& p_exclude_array, uint32_t p_collision_mask) {
    Set<RID> p_exclude;
    for (int i = 0; i < p_exclude_array.size(); ++i) {
        p_exclude.insert(p_exclude_array[i]);
    }

    BTerrain* terrain = BTerrain::get_singleton();
    ERR_FAIL_COND_V(!terrain, Array());
    Ref<World> world = terrain->get_world();
    PhysicsDirectSpaceState* space_state = world->get_direct_space_state();
    ERR_FAIL_COND_V(!space_state, Array());

    float path_length = get_curve_length();
    float z_size = bounds_max.z - bounds_min.z;
    int required_instances = Math::ceil(path_length / z_size);

    // compute how much stretching needs to occur to each instance to cover the final distance
    // and close the gap
    float stretch = (path_length / z_size) - int(path_length / z_size);// get remainder, fmod?
    stretch *= z_size;
    // at this stage stretch = the length we have to close... so spread this over the number of instances
    stretch = 1.f + stretch / required_instances;

    //float z_offset = -bounds_max.z;
    int index_offset = 0;

    Vector3 extents = (bounds_max - bounds_min) / 2;
    BoxShape shape;
    shape.set_extents(extents);

    //RayShape ray;
    //ray.set_length(z_size * stretch);

    PhysicsShapeQueryParameters p_shape_query;
    p_shape_query.set_shape_rid(shape.get_rid());

    Array ret;

    for (int inst = 0; inst < required_instances; ++inst) {
        // populate vertex data
        float z = inst * z_size * stretch;

        // should the BCurve cache this when it bakes the points?
        // make it  bake transforms? can we interpolate matricies easy
        Transform t = get_transform_from_distance_along_curve(z);
/*

        Vector3 p_a = t.xform(Vector3(bounds_min.x, bounds_min.y, 0.f));
        Vector3 p_b = t.xform(Vector3(bounds_min.x, bounds_max.y, 0.f));
        Vector3 p_c = t.xform(Vector3(bounds_max.x, bounds_max.y, 0.f));
        Vector3 p_d = t.xform(Vector3(bounds_max.x, bounds_min.y, 0.f));
*/

        p_shape_query.set_transform(t);


        Vector<PhysicsDirectSpaceState::ShapeResult> sr;
        sr.resize(p_result_max);
        int rc = space_state->intersect_shape(p_shape_query.get_shape_rid(), p_shape_query.get_transform(), p_shape_query.get_margin(), sr.ptrw(), sr.size(), p_exclude, p_shape_query.get_collision_mask());

        for (int i = 0; i < rc; i++) {
            Dictionary d;
            d["rid"] = sr[i].rid;
            d["collider_id"] = sr[i].collider_id;
            d["collider"] = sr[i].collider;
            d["shape"] = sr[i].shape;
            ret.append(d);

            if (ret.size() >= p_result_max) {
                return ret;
            }
        }
    }

    return ret;
}
/*
int BCurveMesh::intersect_shape(ShapeResult *r_results, int p_result_max, const Set<RID> &p_exclude = Set<RID>(), uint32_t p_collision_mask = 0xFFFFFFFF) {
    PhysicsDirectSpaceState* space_state = get_world()->get_direct_space_state();
    ERR_FAIL_COND_V(!space_state, 0);

    float path_length = get_curve_length();
    float z_size = bounds_max.z - bounds_min.z;
    int required_instances = int(path_length / z_size);

    // compute how much stretching needs to occur to each instance to cover the final distance
    // and close the gap
    float stretch = (path_length / z_size) - int(path_length / z_size);// get remainder, fmod?
    stretch *= z_size;
    // at this stage stretch = the length we have to close... so spread this over the number of instances
    stretch = 1.f + stretch / required_instances;

    //float z_offset = -bounds_max.z;
    int index_offset = 0;


    RayShape ray;
    ray.set_length(z_size);

    PhysicsShapeQueryParameters params;
    params.set_shape(ray);

    for (int inst = 0; inst < required_instances; ++inst) {
        // populate vertex data
        float z = inst * z_size * stretch;

        // should the BCurve cache this when it bakes the points?
        // make it  bake transforms? can we interpolate matricies easy
        Transform t = get_transform_from_distance_along_curve(z);

//		if (move_to_ground) {
            //t.origin = terrain->move_vector_to_ground(t.origin);
//			t.origin.y -= bounds_min.y; // move up to make the bottom of the mesh sit on the ground
//		}

/*
        // bring from world space back to object space
        t = t * world_to_obj;
* /

        Vector3 p_a = t.xform(Vector3(bounds_min.x, bounds_min.y, 0.f));
        Vector3 p_b = t.xform(Vector3(bounds_min.x, bounds_max.y, 0.f));
        Vector3 p_c = t.xform(Vector3(bounds_max.x, bounds_max.y, 0.f));
        Vector3 p_d = t.xform(Vector3(bounds_max.x, bounds_min.y, 0.f));


        params.set_transform(t);

        ShapeResult *results;

        Vector<ShapeResult> sr;
        sr.resize(p_result_max);

        const float margin = 1.0;
        space_state->intersect_shape(params.get_shape_rid(), params.get_transform(), margin, sr, sr.size(), p_exclude, p_collision_mask);


        //virtual int intersect_shape(const RID &p_shape, const Transform &p_xform, float p_margin, ShapeResult *r_results, int p_result_max, const Set<RID> &p_exclude = Set<RID>(), uint32_t p_collision_mask = 0xFFFFFFFF) = 0;

    }

    /*
        // raycast down till we reach the terrain to see if there is any physics objects
        // blocking us
        Ref<World> world = terrain->get_world();
        PhysicsDirectSpaceState* space_state = world->get_direct_space_state();
        if (space_state) {
            Vector3 from = tm.origin + tm.get_basis().get_axis(1) * 1000.0f;
            Vector3 to = tm.origin;

            Set<RID> exclude_set;
            PhysicsDirectSpaceState::RayResult ray_result;
            bool result = space_state->intersect_ray(from, to, ray_result, exclude_set);
            if (result) {
                Vector3 out_normal = ray_result.normal;
                Vector3 binormal =  tm.get_basis().get_axis(0).cross(out_normal);
                Vector3 tangent = out_normal.cross(tm.get_basis().get_axis(2));
                tm.basis.set_axis(0, tangent);
                tm.basis.set_axis(1, out_normal);
                tm.basis.set_axis(2, binormal);

                tm.origin = ray_result.position;
            }
        }
    * /
}*/

Transform BCurveMesh::move_transform_to_ground(const Transform& t) {

    // TODO: optimise!
    // note: this is temporary, and should be not enforced to occur,
    // ie. trains will never to a raycast
    // only when we want it to deform to the terrain
    // maybe some adaptive subdivision to follow the terrain to also reduce jitter and bumps
    // only divide the spline when it deviates to far from the terrain
    BTerrain* terrain = BTerrain::get_singleton();
    if (!terrain) {
        return t;
    }

    Transform tm = t;
    //tm = global_transform * tm;

    const float p_push_along_normal_distance = 1.0f;

    tm = terrain->move_transform_to_ground(tm);

    // push along normal a small distance
    tm.origin += tm.get_basis().get_axis(1) * p_push_along_normal_distance;

    return tm;
}

Transform BCurveMesh::get_transform_from_distance_along_curve(float p_offset) {
	if (arc_line_curve.is_valid())
        return move_transform_to_ground(arc_line_curve->get_transform_from_distance_along_curve(p_offset));
	else if (spline.is_valid())
        return move_transform_to_ground(spline->get_transform_from_distance_along_curve(p_offset));
	
	return Transform();
}

void BCurveMesh::add_vertex(MeshDataTool* p_mdt, int v, float z_offset, float stretch, BMeshGenTool *p_mesh_gen) {
    Vector3 pos = p_mdt->get_vertex(v);

    //print_line("pos[" + String::num(v) + "]=" + String(pos));

    // deform vert to mesh
    pos.z += z_offset;
    pos.z *= stretch;

    // should the BCurve cache this when it bakes the points?
    // make it  bake transforms? can we interpolate matricies easy
    //
    // hrmm there is a problem here, we need to have this transform in world space
    // but then we want to move it to local space!?!
    Transform t = get_transform_from_distance_along_curve(-pos.z);
    //if (move_to_ground) {
    //	t.origin = terrain->move_vector_to_ground(t.origin);
    //	t.origin.y -= bounds_min.y; // move up to make the bottom of the mesh sit on the ground
    //}

    pos.z = 0.f;
    pos = t.xform(pos);

    p_mesh_gen->set_next_point(pos);

    Vector3 normal = p_mdt->get_vertex_normal(v);
    Vector3 normal_t = t.basis.xform(normal);
    normal_t.normalize();

    t.origin = Vector3(0, 0, 0);
    Plane tangent = p_mdt->get_vertex_tangent(v);
    Plane tangent_t = t.xform(tangent);

                //mesh_gen.set_next_normal(Vector3(0, 1, 0));
    p_mesh_gen->set_next_normal(normal_t);
    p_mesh_gen->set_next_tangent(tangent_t);
    p_mesh_gen->set_next_colour(p_mdt->get_vertex_color(v));
}

int BCurveMesh::add_indicies(MeshDataTool* p_mdt, int index_offset, BMeshGenTool *p_mesh_gen) {

     for (int f = 0; f < p_mdt->get_face_count(); ++f) {
         int a = p_mdt->get_face_vertex(f, 0) + index_offset;
         int b = p_mdt->get_face_vertex(f, 1) + index_offset;
         int c = p_mdt->get_face_vertex(f, 2) + index_offset;
         p_mesh_gen->set_next_triangle_indicies(a, b, c);
     }

     return index_offset + p_mdt->get_vertex_count();
 }

Ref<ArrayMesh> BCurveMesh::generate_mesh(const Transform& p_obj_to_world) {
    ERR_FAIL_COND_V(!(spline.is_valid() || arc_line_curve.is_valid()), Ref<Mesh>());
    ERR_FAIL_COND_V(!instance_template, Ref<Mesh>());
    ERR_FAIL_COND_V(!mesh_instance, Ref<Mesh>());
    ERR_FAIL_COND_V(!mesh_instance->get_mesh().is_valid(), Ref<Mesh>());
    ERR_FAIL_COND_V(!(mesh_instance->get_mesh()->get_surface_count() > 0), Ref<Mesh>());
/*	
	// convert curve to global space
	arc_line_curve->apply_transform_to_points(p_obj_to_world);
*/	
	//BTerrain* terrain = BTerrain::get_singleton();
	//if (move_to_ground) {
	//	assert(terrain);
	//}
        
    int surface_count = mesh_instance->get_mesh()->get_surface_count();
    assert(surface_count == 1);
    Ref<Material> material = mesh_instance->get_surface_material(0);
    if (material.is_null())
        material = mesh_instance->get_mesh()->surface_get_material(0);
	
    float path_length = get_curve_length();
    if (path_length <= 0.f)
        return NULL;

    float z_size = bounds_max.z - bounds_min.z;
    int required_instances = Math::ceil(path_length / z_size);

    // compute how much stretching needs to occur to each instance to cover the final distance
    // and close the gap
    float stretch = (path_length / z_size) - int(path_length / z_size);// get remainder, fmod?
    stretch *= z_size;
    // at this stage stretch = the length we have to close... so spread this over the number of instances
    stretch = 1.f + stretch / required_instances;

    float z_offset = -bounds_max.z;
    int index_offset = 0;

    BMeshGenTool mesh_gen;
    mesh_gen.resize_verts((required_instances * mesh_data_middle->get_vertex_count()) + mesh_data_front_cap->get_vertex_count() + mesh_data_back_cap->get_vertex_count());
    mesh_gen.resize_indicies(((required_instances * mesh_data_middle->get_face_count()) + mesh_data_front_cap->get_face_count() + mesh_data_back_cap->get_face_count())  * 3);

    // add back cap
    for (int v = 0; v < mesh_data_back_cap->get_vertex_count(); ++v) {
        add_vertex(mesh_data_back_cap, v, z_offset, 1.f, &mesh_gen);
    }
    index_offset = add_indicies(mesh_data_back_cap, index_offset, &mesh_gen);

    // add middle instances
	for (int inst = 0; inst < required_instances; ++inst) {
		// populate vertex data
        for (int v = 0; v < mesh_data_middle->get_vertex_count(); ++v) {
            add_vertex(mesh_data_middle, v, z_offset, stretch, &mesh_gen);
		}
        index_offset = add_indicies(mesh_data_middle, index_offset, &mesh_gen);
		
		z_offset -= z_size;
	}

    // add front cap
    for (int v = 0; v < mesh_data_front_cap->get_vertex_count(); ++v) {
        add_vertex(mesh_data_front_cap, v, z_offset, 1.f, &mesh_gen);
    }
    index_offset = add_indicies(mesh_data_front_cap, index_offset, &mesh_gen);

/*	
	// convert curve back to local space
	arc_line_curve->apply_transform_to_points(p_obj_to_world.inverse());
*/	
	generated_mesh = mesh_gen.generate_mesh();
	
	if (material.is_valid())
		generated_mesh->surface_set_material(0, material);
	
	return generated_mesh;
}

Ref<ArrayMesh> BCurveMesh::generate_mesh_from_bounds(const Transform& p_obj_to_world) {
	ERR_FAIL_COND_V(!(spline.is_valid() || arc_line_curve.is_valid()), Ref<Mesh>());
	ERR_FAIL_COND_V(!instance_template, Ref<Mesh>());
	ERR_FAIL_COND_V(!mesh_instance, Ref<Mesh>());
	ERR_FAIL_COND_V(!mesh_instance->get_mesh().is_valid(), Ref<Mesh>());
	ERR_FAIL_COND_V(!(mesh_instance->get_mesh()->get_surface_count() > 0), Ref<Mesh>());
/*	
	Transform world_to_obj = p_obj_to_world.affine_inverse();
	
	// convert curve to global space
	arc_line_curve->apply_transform_to_points(p_obj_to_world);
*/
/*
	BTerrain* terrain = BTerrain::get_singleton();
	if (move_to_ground) {
		assert(terrain);
	}
*/	
    Ref<Material> material = mesh_instance->get_surface_material(0);
    if (material.is_null())
        material = mesh_instance->get_mesh()->surface_get_material(0);
	
    float path_length = get_curve_length();
    if (path_length <= 0)
        return NULL;

    float z_size = bounds_max.z - bounds_min.z;
    int required_instances = Math::ceil(path_length / z_size);
    required_instances = MAX(2, required_instances); // need at least two, a start and end point!

    // compute how much stretching needs to occur to each instance to cover the final distance
    // and close the gap
    float stretch = (path_length / z_size) - int(path_length / z_size);// get remainder, fmod?
    stretch *= z_size;
    // at this stage stretch = the length we have to close... so spread this over the number of instances
    stretch = 1.f + stretch / required_instances;

    //float z_offset = -bounds_max.z;
    int index_offset = 0;

    BMeshGenTool mesh_gen;
    mesh_gen.resize_verts(required_instances * 4);
    mesh_gen.resize_indicies((required_instances - 1) * 6 * 3);


    for (int inst = 0; inst < required_instances; ++inst) {
            // populate vertex data
            float z = inst * z_size * stretch;

            // should the BCurve cache this when it bakes the points?
            // make it  bake transforms? can we interpolate matricies easy
            Transform t = get_transform_from_distance_along_curve(z);

//		if (move_to_ground) {
                    //t.origin = terrain->move_vector_to_ground(t.origin);
//			t.origin.y -= bounds_min.y; // move up to make the bottom of the mesh sit on the ground
//		}	
 
/*		
            // bring from world space back to object space
            t = t * world_to_obj;
*/
            Vector3 p_a = t.xform(Vector3(bounds_min.x, bounds_min.y, 0.f));
            Vector3 p_b = t.xform(Vector3(bounds_min.x, bounds_max.y, 0.f));
            Vector3 p_c = t.xform(Vector3(bounds_max.x, bounds_max.y, 0.f));
            Vector3 p_d = t.xform(Vector3(bounds_max.x, bounds_min.y, 0.f));

            mesh_gen.set_next_point(p_a);
            mesh_gen.set_next_point(p_b);
            mesh_gen.set_next_point(p_c);
            mesh_gen.set_next_point(p_d);
            //mesh_gen.set_next_normal(mesh_data->get_vertex_normal(v)); // TODO: transform this normal/tangent somehow
            //mesh_gen.set_next_tangent(mesh_data->get_vertex_tangent(v));
            //mesh_gen.set_next_colour(mesh_data->get_vertex_color(v));

            if (inst > 0) {

                    // side
                    mesh_gen.set_next_triangle_indicies(index_offset + 1, index_offset + 0, index_offset + 4);
                    mesh_gen.set_next_triangle_indicies(index_offset + 5, index_offset + 1, index_offset + 4);

                    // top
                    mesh_gen.set_next_triangle_indicies(index_offset + 5, index_offset + 2, index_offset + 1);
                    mesh_gen.set_next_triangle_indicies(index_offset + 2, index_offset + 5, index_offset + 6);

                    // side
                    mesh_gen.set_next_triangle_indicies(index_offset + 6, index_offset + 3, index_offset + 2);
                    mesh_gen.set_next_triangle_indicies(index_offset + 3, index_offset + 6, index_offset + 7);

                    // cant see the bottom, so leave it off

                    index_offset += 4;

                    /*
                    mesh_gen.set_next_triangle_indicies(index_offset + 2, index_offset + 1, index_offset + 0);
                    mesh_gen.set_next_triangle_indicies(index_offset + 2, index_offset + 3, index_offset + 1);
                    index_offset += 2;*/

            }
    }
/*	
    // convert curve back to local space
    arc_line_curve->apply_transform_to_points(p_obj_to_world.inverse());
*/	
    generated_mesh = mesh_gen.generate_mesh();

    if (material.is_valid())
            generated_mesh->surface_set_material(0, material);

    return generated_mesh;
}

void BCurveMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_spline", "spline"),&BCurveMesh::set_spline);
	ClassDB::bind_method(D_METHOD("set_arc_line_curve", "arc_line_curve"),&BCurveMesh::set_arc_line_curve);
	ClassDB::bind_method(D_METHOD("set_instance_template", "instance_template"),&BCurveMesh::set_instance_template);
    ClassDB::bind_method(D_METHOD("generate_mesh", "obj_to_world_transform"),&BCurveMesh::generate_mesh);
    ClassDB::bind_method(D_METHOD("generate_mesh_from_bounds", "obj_to_world_transform"),&BCurveMesh::generate_mesh_from_bounds);
    ClassDB::bind_method(D_METHOD("generate_collision", "obj_to_world_transform"),&BCurveMesh::generate_collision);
    ClassDB::bind_method(D_METHOD("get_transform_from_distance_along_curve", "offset"),&BCurveMesh::get_transform_from_distance_along_curve);
    ClassDB::bind_method(D_METHOD("intersect_shape"), &BCurveMesh::intersect_shape, DEFVAL(Array()), DEFVAL(0x7FFFFFFF));
}

BCurveMesh::BCurveMesh() {
	instance_template = NULL;
	mesh_instance = NULL;
    mesh_data_middle = memnew(MeshDataTool);
    mesh_data_front_cap = memnew(MeshDataTool);
    mesh_data_back_cap = memnew(MeshDataTool);
	move_to_ground = true;
}

BCurveMesh::~BCurveMesh() {
    memdelete(mesh_data_middle);
    memdelete(mesh_data_front_cap);
    memdelete(mesh_data_back_cap);
}
