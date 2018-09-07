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
#ifndef BCURVE_MESH_H
#define BCURVE_MESH_H

#include "bspline.h"
#include "barc_line_curve.h"
#include <assert.h>

class MeshInstance;
class MeshDataTool;
class StaticBody;

/**
	@author Fabian Mathews <supagu@gmail.com>
*/

class BCurveMesh : public Resource {

    GDCLASS(BCurveMesh, Resource);
    
	Node* instance_template;
	Ref<BSpline> spline;
	Ref<BArcLineCurve> arc_line_curve;
	
	MeshInstance* mesh_instance;
    MeshDataTool* mesh_data_front_cap;
    MeshDataTool* mesh_data_middle;
    MeshDataTool* mesh_data_back_cap;
	
	Ref<ArrayMesh> generated_mesh;	
	
	Vector3 bounds_max;
	Vector3 bounds_min;
	
	bool move_to_ground;
	
	float get_curve_length();


    void populate_mesh_data_tool(Node* p_mesh_inst, MeshDataTool* mdt);
    void add_vertex(MeshDataTool* p_mdt, int v, float z_offset, float stretch, BMeshGenTool *p_mesh_gen);
    int add_indicies(MeshDataTool* p_mdt, int index_offset, BMeshGenTool *p_mesh_gen);
	
    Transform move_transform_to_ground(const Transform& t);

protected:

	static void _bind_methods();
	
public:

    Transform get_transform_from_distance_along_curve(float p_offset);
	
	Ref<BArcLineCurve> get_arc_line_curve() const;
	void set_arc_line_curve(const Ref<BArcLineCurve>& p_arc_line_curve);
	void set_spline(const Ref<BSpline>& p_spline);
	void set_instance_template(Node* p_instance_template);
	
	Ref<ArrayMesh> generate_mesh(const Transform& p_obj_to_world);
	
	// far cheaper than generate_mesh as it just returns 
	// uses a cube shape to stretch along the spline
	Ref<ArrayMesh> generate_mesh_from_bounds(const Transform& p_obj_to_world);

    // Get the set of physics objects this mesh collides with along its route
    //int intersect_shape(ShapeResult *r_results, int p_result_max, const Set<RID> &p_exclude = Set<RID>(), uint32_t p_collision_mask = 0xFFFFFFFF);

    Array intersect_shape(int p_result_max, const Array& p_exclude_array, uint32_t p_collision_mask = 0xFFFFFFFF);

    void generate_collision(Node* p_parent, const Vector3& grow_vec);

    BCurveMesh();
    ~BCurveMesh();       
};

#endif // BCURVE_MESH_H
