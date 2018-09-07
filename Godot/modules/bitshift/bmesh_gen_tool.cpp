 
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
#include "bmesh_gen_tool.h"
#include "scene/resources/mesh.h"
#include <assert.h>

void BMeshGenTool::resize_verts(int vertCount) {
    ERR_FAIL_COND(vertCount <= 0);

    points.resize(vertCount);
    normals.resize(vertCount);
    tangents.resize(vertCount*4);
    uvs.resize(vertCount);
	colours.resize(vertCount);
    
    pointsw = points.write();
    normalsw = normals.write();
    tangentsw = tangents.write();
    uvsw = uvs.write();
	coloursw = colours.write();
    
    point_index = 0;
    uv_index = 0;
	normal_index = 0;
	tangent_index = 0;
	colour_index = 0;
}

void BMeshGenTool::resize_indicies(int indexCount) {
    indices.resize(indexCount);
    indicesw = indices.write();
    indices_index = 0;
}

Ref<ArrayMesh> BMeshGenTool::generate_mesh() {
    
    Array arr;
    arr.resize(VS::ARRAY_MAX);
    arr[VS::ARRAY_VERTEX] = points;
	arr[VS::ARRAY_INDEX] = indices;
	
    if (uv_index > 0) {
        assert(uv_index == uvs.size());
		arr[VS::ARRAY_TEX_UV] = uvs;
    }
    
    if (point_index > 0) {
        assert(point_index == points.size());
    }
	
	if (normal_index > 0) {
        assert(normal_index == normals.size());
		arr[VS::ARRAY_NORMAL] = normals;
    }
	
	if (tangent_index > 0) {
        assert(tangent_index == tangents.size());
		arr[VS::ARRAY_TANGENT] = tangents;
    }
	
	if (colour_index > 0) {
        assert(colour_index == colours.size());
		arr[VS::ARRAY_COLOR] = colours;
    }
    
    if (indices_index > 0) {
        int size = indices.size();
        assert(indices_index == size);
    }
            
    Ref<ArrayMesh> mesh = memnew( ArrayMesh() );
    if (points.size() && indices.size()) {
        mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arr);

        // ensure aabb is good!
        //aabb.grow_by(0.01f);
        //mesh->surface_set_custom_aabb(0, aabb);
    }

    return mesh;
}

void BMeshGenTool::set_next_point(const Vector3& point) {
    assert(point_index < points.size());
    //if (point_index == 0)
    //    aabb.position = point;
    //else
    //    aabb.expand_to(point);

    pointsw[point_index] = point;
    ++point_index;
}

void BMeshGenTool::set_next_uv(const Vector2& uv) {
    assert(uv_index < uvs.size());
    uvsw[uv_index] = uv;
    ++uv_index;
}
    
void BMeshGenTool::set_point(int index, const Vector3& point) {
    assert(index >= 0 && index < points.size());
    pointsw[index] = point;
}

void BMeshGenTool::set_uv(int index, const Vector2& uv) {
    assert(index >= 0 && index < uvs.size());
    uvsw[index] = uv;
}

void BMeshGenTool::set_next_normal(const Vector3& normal) {
	assert(normal_index < normals.size());
    normalsw[normal_index] = normal;
    ++normal_index;
}

void BMeshGenTool::set_next_tangent(const Plane& tangent) {
	assert(tangent_index < tangents.size());
    tangentsw[tangent_index + 0] = tangent.normal.x;
	tangentsw[tangent_index + 1] = tangent.normal.y;
	tangentsw[tangent_index + 2] = tangent.normal.z;
	tangentsw[tangent_index + 3] = tangent.d;
    tangent_index += 4;
}

void BMeshGenTool::set_next_colour(const Color& colour) {
	assert(colour_index < colours.size());
    coloursw[colour_index] = colour;
    ++colour_index;
}

void BMeshGenTool::set_next_triangle_indicies(int a, int b, int c) {
    assert((indices_index + 2) < indices.size());
    assert(a >= 0 && a < points.size());
    assert(b >= 0 && b < points.size());
    assert(c >= 0 && c < points.size());
    
    indicesw[indices_index] = a;
    indicesw[indices_index + 1] = b;
    indicesw[indices_index + 2] = c;  
    
    indices_index += 3;
}

void BMeshGenTool::set_triangle_indicies(int tri_index, int a, int b, int c) {
    int idx = tri_index * 3;
    assert(idx >= 0 && (idx + 2) < indices.size());
    assert(a >= 0 && a < points.size());
    assert(b >= 0 && b < points.size());
    assert(c >= 0 && c < points.size());
    
    indicesw[idx] = a;
    indicesw[idx + 1] = b;
    indicesw[idx + 2] = c;    
}

void BMeshGenTool::populate_all_normals_and_tangents(const Vector3& normal, const Vector3& tangent, const Vector3& binormal) {    
    Vector3 bn = normal.cross(tangent);
    float d = binormal.dot(bn);
    
    for (int i = 0; i < points.size(); ++i) {
        normalsw[i] = normal;

        // taken from SurfaceTool::commit case Mesh::ARRAY_FORMAT_TANGENT
        // this generates the tangent plane
        float ti = i * 4;
        tangentsw[ti+0]=tangent.x;
        tangentsw[ti+1]=tangent.y;
        tangentsw[ti+2]=tangent.z;
        tangentsw[ti+3]=d<0 ? -1 : 1;
    }
	
	normal_index = points.size();
	tangent_index = points.size() * 4;
}

Vector3 BMeshGenTool::get_point(int index) {
    assert(index >= 0 && index < points.size());
    return pointsw[index];
}

BMeshGenTool::BMeshGenTool() {

}

BMeshGenTool::~BMeshGenTool() {

}


