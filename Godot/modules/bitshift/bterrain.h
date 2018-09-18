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
#ifndef BTERRAIN_H
#define BTERRAIN_H

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
class BTerrainGenerator;

/**
	@author Fabian Mathews <supagu@gmail.com>
*/

class BTerrain : public Spatial {

    friend class BWater;
	friend class BClutterMap;
    
    GDCLASS(BTerrain,Spatial)

    static BTerrain *singleton;

    BTerrainGenerator *terrain_generator;

    BNormalMap normal_map;
    BHeightMap height_map;

    String height_map_path;
    String normal_map_path;

    Ref<Material> terrain_material;
    Ref<Material> skirt_material;

    float size;

    /*
    float height;

    // in a tif file, we have a int16, which has a max value of 32767 (see geotiff_to_height.py),
    // so we "normalise" values between 0 -> 32767 to give us the maximum resolution
    // this value brings that back to 0 -> 1 range so height is correct
    float height_normalise;*/

    float skirt_bottom;
    int resolution;
    int chunks;
    bool castShadows;
    bool generateCollision;
    bool generateLOD;

    enum Flags {
        F_SIZE         = 1 << 0,
        F_HEIGHT_MAP   = 1 << 1,
        F_CHUNKS       = 1 << 2,
        F_RESOLUTION   = 1 << 3,
        F_HEIGHT       = 1 << 4,
        F_SKIRT_MATERIAL  = 1 << 5,
        F_TERRAIN_MATERIAL  = 1 << 6,
        F_NORMAL_MAP   = 1 << 7,
    };
    int dirty_flags;
    int first_set_flags;

    bool in_update;
    //bool inTree;

    AABB aabb;

    Map<int, Ref<Mesh> > resolutionMeshMap;

    Vector<Node*> lodNodes;
    Vector<Node*> collisionNodes;

    struct TerrainLod {
        int far;
        int near;
    };

    Spatial* create_lod(const String& name, int near, int far/*, Ref<Mesh>& mesh*/);

    void _dirty(int flags);
    void _update();

    void set_size(float size);
    float get_size() const;

    void set_height(float height);

    bool _load_normal_map();
    bool _load_height_map();
    void _set_normal_map_on_terrain_material();
        
protected:

    static void _bind_methods();
    void _notification( int p_what);
	virtual void _changed_callback(Object *p_changed, const char *p_prop);
    void _terrain_material_changed();

public:

    BTerrain();
    ~BTerrain();      

    void set_regenerate(bool regen);
    bool get_regenerate() const;

    void set_height_map_path(const String& path);
    String get_height_map_path() const;

    void set_normal_map_path(const String& path);
    String get_normal_map_path() const;

    void set_terrain_material(const Ref<Material>& material);
    Ref<Material> get_terrain_material() const;

    void set_skirt_material(const Ref<Material>& material);
    Ref<Material> get_skirt_material() const;

	Transform move_transform_to_ground(const Transform& p_transform) const;
	Vector3 move_vector_to_ground(const Vector3& p_world_pos) const;
	
    Dictionary raycast_down(const Vector3& position) const;
    Dictionary raycast(const Vector3& from, const Vector3& to);

    Vector2 world_to_texture_space(const Vector3& position) const;
    Vector2 world_to_texture_space(const Transform& xform, const Vector3& position) const;

    //float get_normal_map_slope_angle() const;
    float get_height() const;

    Array get_collision_nodes();

    static BTerrain *get_singleton() { return singleton; }
};

#endif // BTERRAIN_H
