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
#ifndef BTERRAIN_GENERATOR_H
#define BTERRAIN_GENERATOR_H

#include "scene/3d/spatial.h"
#include "core/self_list.h"
#include "core/func_ref.h"
#include "itch_io.h"
#include "bnormal_map.h"
#include "bheight_map.h"

class CanvasItemMaterial;
class Spatial;
class Mesh;
class Node;
class BMeshGenTool;
class StaticBody;

/**
	@author Fabian Mathews <supagu@gmail.com>
*/

class BTerrainGenerator {

public:

    enum Flags {
        F_NO_SHADOWS         = 1 << 0,

        F_DEFAULT = 0,
    };

    struct MeshInputData {

        Flags flags;

        Node* parent;

        Ref<Material> terrain_material;
        Ref<Material> skirt_material;

        BNormalMap *normal_map;
        BHeightMap *height_map;

        float size; // size in metres
        float quads_per_metre;
        float quads_per_chunk;

        float skirt_bottom;

        // lod
        int near;
        int far;

        MeshInputData() {
            parent = NULL;
            normal_map = NULL;
            height_map = NULL;
            size = 100.f;
            flags = F_DEFAULT;
            skirt_bottom = -20.f;
        }

        MeshInputData(Node* p_parent, BNormalMap *p_normal_map, BHeightMap* p_height_map, float p_size) {
            parent = p_parent;
            normal_map = p_normal_map;
            height_map = p_height_map;
            size = p_size;
            flags = F_DEFAULT;
            skirt_bottom = -20.f;
        }
    };

    struct MeshOutputData {
        AABB aabb;
    };

    Thread *thread;

    BTerrainGenerator();
    ~BTerrainGenerator();

    AABB compute_aabb(float size, float height);

    void create_lod(MeshInputData& meshInputData);

    StaticBody *create_collision(MeshInputData& meshInputData);

    Ref<Mesh> create_terrain(int resolution, int x, int y, const Transform& uvXform);
    Ref<Mesh> create_skirt(int resolution, const Transform& uvXform);

    static Ref<Material> duplicate_material(Ref<Material>& material, const Transform& uvTransform);

    void add_skirt_verts_for_point(BMeshGenTool& mesh_gen, int x, int y, int resolution, const Transform& uvXform);

    void _set_normal_map_on_terrain_material();

protected:

    static void _create_lod_thread_function(void *self);

    void _create_lod();

    MeshInputData mesh_input_data;

};

#endif // BTERRAIN_GENERATOR_H
