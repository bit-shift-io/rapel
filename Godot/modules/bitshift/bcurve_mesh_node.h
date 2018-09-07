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
#ifndef BCURVE_MESH_NODE_H
#define BCURVE_MESH_NODE_H

#include "scene/3d/spatial.h"
#include "core/vector.h"
#include <assert.h>

class BCurveMesh;
class BArcLineCurve;
class MeshInstance;
class StaticBody;

/**
	@author Fabian Mathews <supagu@gmail.com>
 
	Node that holds a curvemesh (and hence a curve)
*/

class BCurveMeshNode : public Spatial  {

    GDCLASS(BCurveMeshNode, Spatial);
    
public:
	
	enum MeshGenerationType {
		MGT_BOUNDS,
		MGT_MESH,
	};
	
protected:
	
	Ref<BArcLineCurve> local_curve; // local space form of the curve
	Ref<BArcLineCurve> world_curve; // world space form of the curve
	Ref<BCurveMesh> curve_mesh;
	
	MeshGenerationType mesh_gen_type;

    StaticBody* static_body;
	MeshInstance* mesh_inst;
    //bool gen_mesh_on_transform_changed;
	bool world_curve_dirty;
		
	static void _bind_methods();
	
	void _notification(int p_what);
	void _validate_property(PropertyInfo &property) const;
	
	void _update();
    void _local_curve_changed();
	
public:
	
	Ref<BArcLineCurve> get_world_curve();
	
	Ref<BArcLineCurve> get_local_curve();
	void set_local_curve(const Ref<BArcLineCurve>& p_curve);
	
	Ref<BCurveMesh> get_curve_mesh();
	void set_curve_mesh(const Ref<BCurveMesh>& p_curve_mesh);
        MeshInstance* get_mesh_instance();
	
	void set_mesh_generation_type(int p_mesh_gen_type);
	int get_mesh_generation_type() const;
	
	void generate_mesh();
    void generate_collision();
	
	void set_generate_mesh_on_transform_changed(bool gen_mesh);
	bool get_generate_mesh_on_transform_changed() const;
	
	Dictionary closest_distance_to_curve(const Vector3& p_world_pos, float p_distance_max = -1.f) const;
    Transform get_global_transform_from_distance_along_curve(float p_offset);

    Array get_collision_nodes();
	
    BCurveMeshNode();
    ~BCurveMeshNode();       
};

#endif // BCURVE_MESH_NODE_H
