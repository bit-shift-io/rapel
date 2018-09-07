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
#include "bsurface_tool.h"

uint32_t BSurfaceTool::VectorHasher::hash(const Vector3 &p_vector) {

    uint32_t h = hash_djb2_buffer((const uint8_t *)&p_vector, sizeof(real_t) * 3);
    //print_line("vertex: (" + rtos(p_vector.x) + "," + rtos(p_vector.y) + "," + rtos(p_vector.z) + ") -> hash: " + itos(h));
    return h;
}

// mofidied version of SurfaceTool::generate_normals to force all normals to be generated and smoothed
void BSurfaceTool::generate_smooth_normals() {

	ERR_FAIL_COND(primitive != Mesh::PRIMITIVE_TRIANGLES);

	bool was_indexed = index_array.size();

	deindex();

    index_array.clear(); // so reindexing works

    HashMap<Vector3, Vector3, VectorHasher> vertex_hash;

    //int count = 0;

    List<Vertex>::Element *B = vertex_array.front();

    // generate a normal for each face and add it to the cached vector
    for (List<Vertex>::Element *E = B; E;) {

        List<Vertex>::Element *v[3];
		v[0] = E;
		v[1] = v[0]->next();
		ERR_FAIL_COND(!v[1]);
		v[2] = v[1]->next();
		ERR_FAIL_COND(!v[2]);
		E = v[2]->next();

		Vector3 normal = Plane(v[0]->get().vertex, v[1]->get().vertex, v[2]->get().vertex).normal;

        for (int i = 0; i < 3; i++) {
            Vector3 *lv = vertex_hash.getptr(v[i]->get().vertex);
            if (!lv) {
                vertex_hash.set(v[i]->get().vertex, normal);
            } else {
                (*lv) += normal;
            }
        }
	}

    //print_line("smoothing result:");
    int i = 0;
    // apply smoothed normals from normal cache
    for (List<Vertex>::Element *E = B; E;) {
        Vector3 *lv = vertex_hash.getptr(E->get().vertex);
        if (lv) {
            Vector3 new_norm = lv->normalized();
                //print_line(itos(i) + ": vertex: (" + rtos(E->get().vertex.x) + "," + rtos(E->get().vertex.y) + "," + rtos(E->get().vertex.z) + ")");
                //print_line(itos(i) + ": normal from: (" + rtos(E->get().normal.x) + "," + rtos(E->get().normal.y) + "," + rtos(E->get().normal.z) + ") -> (" + rtos(new_norm.x) + "," + rtos(new_norm.y) + "," + rtos(new_norm.z) + ")");
            E->get().normal = new_norm;
        } else {
            // oh dear?!
            ERR_FAIL_COND(true);
        }
        E = E->next();
        ++i;
    }

    //print_line("--------");

	format |= Mesh::ARRAY_FORMAT_NORMAL;

	if (was_indexed) {
		index();
	}
}

void BSurfaceTool::_bind_methods() {

	ClassDB::bind_method(D_METHOD("generate_smooth_normals"), &BSurfaceTool::generate_smooth_normals);

    ClassDB::bind_method(D_METHOD("create_from:Mesh", "existing:Mesh", "surface"), &GSurfaceTool::create_from);
    ClassDB::bind_method(D_METHOD("append_from:Mesh", "existing:Mesh", "surface", "transform:Transform"), &GSurfaceTool::append_from);
}

BSurfaceTool::BSurfaceTool() {

}
