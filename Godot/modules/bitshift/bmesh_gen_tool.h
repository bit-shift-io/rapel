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
#ifndef BMESH_GEN_TOOL_H
#define BMESH_GEN_TOOL_H

#include "core/reference.h"
#include "core/dvector.h"
#include "core/math/vector3.h"
//#include "core/math/math_2d.h"

class ArrayMesh;

/**
	@author Fabian Mathews <supagu@gmail.com>
*/

class BMeshGenTool  {
    
    PoolVector<Vector3> points;
    PoolVector<Vector3>::Write pointsw;
    int point_index;
    
	PoolVector<Color> colours;
    PoolVector<Color>::Write coloursw;
	int colour_index;
	
    PoolVector<Vector3> normals;
    PoolVector<Vector3>::Write normalsw;
	int normal_index;
    
    PoolVector<Vector2> uvs;
    PoolVector<Vector2>::Write uvsw;
    int uv_index;
    
    PoolVector<int> indices;
    PoolVector<int>::Write indicesw;
    int indices_index;
    
    PoolVector<float> tangents;
    PoolVector<float>::Write tangentsw;
	int tangent_index;

    //AABB aabb;
    
protected:

    // advanced, less safety checks here
    // make protected to avoid them being used for now
    void set_triangle_indicies(int tri_index, int a, int b, int c);
    void set_point(int index, const Vector3& point);
    void set_uv(int index, const Vector2& uv);
        
public:

    void resize_verts(int vertCount);
    void resize_indicies(int indexCount);
    
    void populate_all_normals_and_tangents(const Vector3& normal = Vector3(0, 1, 0), const Vector3& tangent = Vector3(1, 0, 0), const Vector3& binormal = Vector3(0, 0, -1));
   
    void set_next_triangle_indicies(int a, int b, int c);
    void set_next_point(const Vector3& point);
    void set_next_uv(const Vector2& uv);
	void set_next_normal(const Vector3& normal);
	void set_next_tangent(const Plane& tangent);
	void set_next_colour(const Color& colour);
    
    Vector3 get_point(int index);

    Ref<ArrayMesh> generate_mesh();
    
    BMeshGenTool();
    ~BMeshGenTool();      
        
};

#endif // BMESH_GEN_TOOL_H
