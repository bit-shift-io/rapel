 
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
#include "bterrain.h"
#include "bterrain_generator.h"
#include "butil.h"
#include "bmaterial.h"
#include "bconfig.h"
#include "bmesh_gen_tool.h"
#include "bimage_exr.h"
#include "core/bind/core_bind.h"
#include "core/ustring.h"
#include "scene/main/node.h"
#include "scene/main/timer.h"
#include "scene/2d/canvas_item.h"
#include "scene/3d/spatial.h"
#include "scene/3d/mesh_instance.h"
#include "scene/3d/physics_body.h"
#include "scene/resources/texture.h"
#include "core_string_names.h"
#include <assert.h>

BTerrain *BTerrain::singleton=NULL;


void BTerrain::set_regenerate(bool regen) {
   _dirty(F_TERRAIN_MATERIAL | F_HEIGHT_MAP | F_HEIGHT);
}

bool BTerrain::get_regenerate() const {
    return false;
}

void BTerrain::set_height_map_path(const String& path) {
    if (height_map_path == path)
        return;

    height_map_path = path;
    _dirty(F_HEIGHT_MAP);
}

String BTerrain::get_height_map_path() const {
    return height_map_path;
}

void BTerrain::set_normal_map_path(const String& path) {
    if (normal_map_path == path)
        return;

    normal_map_path = path;
    _dirty(F_NORMAL_MAP);
}

String BTerrain::get_normal_map_path() const {
    return normal_map_path;
}

void BTerrain::_changed_callback(Object *p_changed, const char *p_prop) {
	// material changed, so update the mesh
    //_dirty();
    int nothing = 0;
    ++nothing;
}

void BTerrain::_terrain_material_changed() {
    _dirty(F_TERRAIN_MATERIAL);
}

void BTerrain::set_terrain_material(const Ref<Material>& p_material) {
    if (terrain_material == p_material)
        return;

	if (terrain_material.is_valid()) {
		terrain_material->remove_change_receptor(this);
	}
	
    terrain_material = p_material;
    _dirty(F_TERRAIN_MATERIAL);
	
	if (terrain_material.is_valid()) {
		terrain_material->add_change_receptor(this);
        terrain_material->connect(CoreStringNames::get_singleton()->changed, this, "_terrain_material_changed");
	}
}

Ref<Material> BTerrain::get_terrain_material() const {
    return terrain_material;
}

void BTerrain::set_skirt_material(const Ref<Material>& p_material) {
    if (p_material == skirt_material)
        return;

    skirt_material = p_material;
    _dirty(F_SKIRT_MATERIAL);
}

Ref<Material> BTerrain::get_skirt_material() const {
    return skirt_material;
}

void BTerrain::_notification(int p_what)
{
    switch (p_what)
    {
        case NOTIFICATION_READY: {
            first_set_flags = -1; // any changes after a ready means we should apply them!
            dirty_flags = F_SIZE; // flags to determine what we need to generate on ready - need to generate mesh
            _update();
        } break;

        case NOTIFICATION_ENTER_TREE: {
            //inTree = true;
            //_dirty();
        } break;
        
        case NOTIFICATION_EXIT_TREE: {
            //inTree = false;
        } break;
        
        case NOTIFICATION_ENTER_WORLD: {
            //_dirty();
        } break;
        
        case NOTIFICATION_EXIT_WORLD: {
            //BUtil::get_singleton()->delete_children(this);
        } break;
			
        default: {}
    }
	
    //Spatial::_notification(p_what);
}

void BTerrain::_dirty(int flags) {
    bool first_changed = !(first_set_flags & flags);
    first_set_flags |= flags;

    // do not call _update on first set of values
    // this is handled by the NOTIFICATION_READY
    if (first_changed)
        return;

    dirty_flags |= flags;
    call_deferred("_update");
}

void BTerrain::_update() {

    if (in_update)
        return;

    in_update = true;

    bool load_height_map_ok =_load_height_map();
    if (!load_height_map_ok || !height_map.get_height_map_tool().is_valid()) {
        BUtil::get_singleton()->log_editor_message("ERROR: Setup a proper heightmap to see terrain. Failed to load height map:" + height_map_path);
        in_update = false;
        return;
    }

    bool load_normal_map_ok = _load_normal_map();
    if (!load_normal_map_ok) {
        BUtil::get_singleton()->log_editor_message("ERROR: Failed to load terrin normal map:" + normal_map_path);
    }

    /*
    if ((dirty_flags & F_HEIGHT) != 0 || (dirty_flags & F_HEIGHT_MAP) != 0 || !load_normal_map_ok) {
        _generate_normal_map();
        load_normal_map_ok = _load_normal_map();
    }*/

    if ((dirty_flags & F_HEIGHT) != 0 || (dirty_flags & F_CHUNKS) != 0 || (dirty_flags & F_SIZE) != 0 || (dirty_flags & F_RESOLUTION) != 0) {
        for (int i = 0; i < lodNodes.size(); ++i) {
            remove_child(lodNodes[i]);
            memdelete(lodNodes[i]);
        }
        lodNodes.clear();

        aabb = AABB();
        collisionNodes.clear();

        //

        // compute resolution

        float max_quads_per_metre = BConfig::get_singleton()->get_setting("bitshift/terrain/max_quads_per_metre", 1.f);
        float min_quads_per_metre = BConfig::get_singleton()->get_setting("bitshift/terrain/min_quads_per_metre", 0.1f);

        float collision_quads_per_metre = max_quads_per_metre * size;
        float terrain_quality = BConfig::get_singleton()->get_setting("video_options/terrain_quality", 1.f); // percentage

        // we want fast loading in the editor, not so careing about quality of the terrain
        if (Engine::get_singleton()->is_editor_hint()) {
            terrain_quality = BConfig::get_singleton()->get_setting("bitshift/terrain/editor_terrain_quality", 0.f);
        }

        float mesh_quads_per_metre = min_quads_per_metre + (max_quads_per_metre - min_quads_per_metre) * terrain_quality;
        float quads_per_chunk = 10000;

        Spatial* lod = memnew(Spatial());
        add_child(lod); // reparent
        lod->set_name("Terrain Lod1"); // leave this to client?
        lodNodes.push_back(lod);

        aabb = terrain_generator->compute_aabb(size, height_map.get_height());

        BTerrainGenerator::MeshInputData mid(lod, &normal_map, &height_map, size);
        mid.terrain_material = terrain_material;
        mid.skirt_material = skirt_material;
        mid.skirt_bottom = skirt_bottom;
        mid.quads_per_chunk = quads_per_chunk;

        // TODO:
        // can we thread this? does chunking the collision into smaller chunks result on faster processing of the collision shape?
        /*
        // collision:
        mid.quads_per_metre = collision_quads_per_metre;
        StaticBody *collision = tgen.create_collision(mid);
        lod->add_child(collision);
        collisionNodes.push_back(collision);
        */

        mid.quads_per_metre = mesh_quads_per_metre;
        mid.far = 999999;
        mid.near = 0;

        // TODO: when this starts handling LOD's we need to pass in a set of lod data
        // so it only needs be called once and then the threading does all the lods in one hit
        //
        // also we can put each chunk into a thread
        // for this reason it might be better to have not quads_per_chunk but a chunk size, so there are always the same amount of chunks regardless of terrain quality
        //

        int lod_levels = 1;// TODO: implement some more lods
        for (int i = 0; i < lod_levels; ++i) {
            terrain_generator->create_lod(mid);
            //aabb = mod.aabb;
            //mid.quads_per_metre /= 2;
            //mid.quads_per_chunk /= 2;
        }
    }

    dirty_flags = 0;

    _change_notify();
    emit_signal(CoreStringNames::get_singleton()->changed);

    in_update = false;
}

bool BTerrain::_load_height_map() {
    uint64_t us = OS::get_singleton()->get_ticks_usec();

    height_map.load(height_map_path);
    print_line("BTerrain::_load_height_map took (seconds): " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));
    return true;
}

bool BTerrain::_load_normal_map() {
    uint64_t us = OS::get_singleton()->get_ticks_usec();

    Error error = normal_map.load(normal_map_path);
    if (error != OK) {
        return false;
    }

    _set_normal_map_on_terrain_material();

    print_line("BTerrain::_load_normal_map took (seconds): " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));
    return true;
}

void BTerrain::_set_normal_map_on_terrain_material() {
    if (!terrain_material.is_valid())
        return;

    BSpatialMaterial* spat_mat = Object::cast_to<BSpatialMaterial>(*terrain_material);
    if (spat_mat) {
        spat_mat->set_texture(BSpatialMaterial::TEXTURE_NORMAL, normal_map.get_normal_map());
    }
    else {
        BUtil::get_singleton()->log_editor_message("ERROR: Terrain expects a BSpatialMaterial of some sort, ideally BTerrainMaterial");
    }
}

Array BTerrain::get_collision_nodes() {
    Array results;
    for (int i = 0; i < collisionNodes.size(); ++i) {
        results.append(collisionNodes[i]);
    }

    return results;
}

Transform BTerrain::move_transform_to_ground(const Transform& p_transform) const {
    Transform t_r = p_transform;

    // bring world space position into local space
    Transform xform = get_global_transform();
    Vector3 localPos = xform.xform_inv(t_r.origin);
	
    float halfSize = float(size) / 2.0;
    
    float px = (localPos.x + halfSize) / float(size); // * (heightmap.width_m1 / float(size));
    float py = (localPos.z + halfSize) / float(size); // * (heightmap.height_m1 / float(size));
    Vector2 texel(px, py);

    if (!height_map.is_texel_valid(texel)) //!heightmap.is_bilinear_texel_valid(px, py))
        return p_transform;
	
    //assert(heightmap.height == normal_map.height);
    //assert(heightmap.width == normal_map.width);
    Vector3 out_normal = normal_map.is_valid() ? normal_map.get_normal_from_texel(texel) : Vector3(0, 1, 0); //decompress_normal(normal_map.get_texel_as_vec3(px, py));
    
    // dodgey normal maps! - shouldnt be possible, but you never know!
    // if a dodgey normal gets used, the game can crash!
    if (Math::is_nan(out_normal.x) || Math::is_nan(out_normal.y) || Math::is_nan(out_normal.z)) {
        return p_transform;
    }

    //Vector3 krita_col = out_normal * 255.f;
    //print_line("pixel: (" + itos(px) + ", " + itos(py) + ")");
    //print_line("krita col: (" + itos(krita_col.x) + ", " + itos(krita_col.y) + ", "+ itos(krita_col.z) + ")");
    
    float height = height_map.get_height_from_texel(texel);
    //Vector3 out_pos = heightmap.get_texel_as_vec3_bilinear(px, py);
        
    localPos.y = height; //out_pos.x * height * height_normalise;
    t_r.origin = xform.xform(localPos);
    
    Vector3 binormal =  p_transform.get_basis().get_axis(0).cross(out_normal);
    Vector3 tangent = out_normal.cross(p_transform.get_basis().get_axis(2));
    t_r.basis.set_axis(0, tangent);
    t_r.basis.set_axis(1, out_normal);
    t_r.basis.set_axis(2, binormal);
    
    return t_r;
}

Vector3 BTerrain::move_vector_to_ground(const Vector3& p_world_pos) const {
	// bring world space position into local space
    Transform xform = get_global_transform();
    Vector3 localPos = xform.xform_inv(p_world_pos);
	
    float halfSize = float(size) / 2.0;
    
    float px = (localPos.x + halfSize) / float(size);
    float py = (localPos.z + halfSize) / float(size);
    Vector2 texel(px, py);

    if (!height_map.is_texel_valid(texel)) //if (!heightmap.is_pixel_valid(px, py))
        return p_world_pos;
	
    float height = height_map.get_height_from_texel(texel);
    //Vector3 c = heightmap.get_texel_as_vec3_bilinear(px, py);
    //Color c = image.get_pixel(px, py);   
    
    localPos.y = height; //c.x * height * height_normalise;
    
    Vector3 worldPos = xform.xform(localPos);
	return worldPos;
}

Vector2 BTerrain::world_to_texture_space(const Transform& xform, const Vector3& position) const {
    // bring world space position into local space
    Vector3 localPos = xform.xform_inv(position);

    float halfSize = float(size) / 2.0;

    float px = (localPos.x + halfSize) / size;
    float py = (localPos.z + halfSize) / size;

    return Vector2(px, py);
}

Vector2 BTerrain::world_to_texture_space(const Vector3& position) const {
    return world_to_texture_space(get_global_transform(), position);
}

Dictionary BTerrain::raycast_down(const Vector3& position) const {
    // bring world space position into local space
    Transform xform = get_global_transform();
    Vector3 localPos = xform.xform_inv(position);
    
    float halfSize = float(size) / 2.0;
    
    float px = (localPos.x + halfSize) / float(size);
    float py = (localPos.z + halfSize) / float(size);
    Vector2 texel(px, py);

	Dictionary d;
    if (!height_map.is_texel_valid(texel)) //if (heightmap.image.is_null() || !heightmap.is_pixel_valid(px, py))
		return d;
	
    float height = height_map.get_height_from_texel(texel);
    //Color c = image.get_pixel(px, py);   
    
    localPos.y = height; //c.x * height * height_normalise;
    
    Vector3 worldPos = xform.xform(localPos);
    
    d["position"] = worldPos;
    d["normal"] = normal_map.is_valid() ? normal_map.get_normal_from_texel(texel) : Vector3(0, 1, 0); //get_terrain_normal_at_pixel(px, py); // straight up for now.... TODO: normalmap lookup
    d["collider"]=this;        
    return d;
}

Dictionary BTerrain::raycast(const Vector3& from, const Vector3& to) {
    Dictionary d;
    
    if (!normal_map.is_valid())
        return d;

    Transform xform = get_global_transform();
    AABB laabb = xform.xform(aabb);

    // here i cast the ray through the AABB for the terrain
    // and reduce the length of the ray to start at the collision point
    // and exit when it gets to the other side of the AABB collision point
    // to make it more accurate!
    Vector3 clipFrom;
    Vector3 clipTo;
    
    if (!laabb.intersects_segment(from, to, &clipFrom)) {
        // no collision
        return d;
    }
    
    if (!laabb.intersects_segment(to, from, &clipTo)) {
        assert(false); // what????
    }
    
    assert(height_map.get_height_map_tool().is_square());
    float w_div_size = 1/*height_map.get_height_map_tool().width_m1*/ / size;
    float halfSize = size / 2.0;
    
    Vector3 fromLocal = xform.xform_inv(clipFrom);
    Vector3 toLocal = xform.xform_inv(clipTo);
    
    // convert from local space to texel space
    fromLocal = Vector3((fromLocal.x + halfSize) * w_div_size, fromLocal.y, (fromLocal.z + halfSize) * w_div_size);
    toLocal = Vector3((toLocal.x + halfSize) * w_div_size, toLocal.y, (toLocal.z + halfSize) * w_div_size);
    
    
    // OPTIMISATIONS:
    // wait for the distance sign to flip?
    // then do some sort of binary search
    const int step_count = 1000.0;
    Vector2 texelStep = (Vector2(toLocal.x, toLocal.z) - Vector2(fromLocal.x, fromLocal.z)) / (float)step_count;
    float heightStep = (toLocal.y - fromLocal.y) / (float)step_count;
    
    //float closestDist = 10000.0;
    bool collision = false;
    //Vector3 localPos = fromLocal;
    
    float prevDist = 1.0;
    
    Vector2 texelPos = Vector2(fromLocal.x, fromLocal.z); // the current ray position ray in texel space
    float texelHeight = fromLocal.y; // the height at the current ray

    for (int s = 0; s < step_count; ++s) {
        if (height_map.is_texel_valid(texelPos)) { //.is_pixel_valid(localPos.x, localPos.z)) {
            //Vector3 c =
            float height = height_map.get_height_from_texel(texelPos); //.get_height_from_pixel(localPos.x, localPos.z);
            //Color c = image.get_pixel(localPos.x, localPos.z);
            //float pixelHeight = c.x * height * height_normalise;
            float dist = texelHeight - height; //pixelHeight;

            // wait for a change in sign
            if ((prevDist > 0.0 && dist < 0.0) || (prevDist < 0.0 && dist > 0.0)) {
                // close enough! - collision
                //print_line("we have a collision!");
                collision = true;
                break;
            }

            // or some tiny distance
            if (ABS(dist) < 0.01) {
                // close enough! - collision
                collision = true;
                break;
            }

            prevDist = dist;
        }

        //localPos += step;
        texelPos += texelStep;
        texelHeight += heightStep;
    }
     
    if (collision) {
        Vector3 worldNormal = normal_map.is_valid() ? normal_map.get_normal_from_texel(texelPos) : Vector3(0, 1, 0); //localPos.x, localPos.z); //get_terrain_normal_at_pixel(localPos.x, localPos.z);
                
        // convert from texel space to local space
        Vector3 localPos = Vector3((texelPos.x / w_div_size) - halfSize, texelHeight, (texelPos.y / w_div_size) - halfSize); //Vector3((localPos.x / w_div_size) - halfSize, localPos.y, (localPos.z / w_div_size) - halfSize);

        // local to world
        Vector3 worldPos = xform.xform(localPos);

        //print_line("position: " + worldPos);

        d["position"]=worldPos;
        d["normal"]=worldNormal;
        d["collider"]=this;   
    }
    else {
        print_line("no collision with terrain!");
    }
    return d;
}

void BTerrain::set_size(float p_size) {
    if (p_size == size)
        return;

    size = p_size;
    _dirty(F_SIZE);
}

float BTerrain::get_size() const {
    return size;
}

void BTerrain::set_height(float p_height) {
    if (p_height == height_map.get_height())
        return;

    height_map.set_height(p_height);
    _dirty(F_HEIGHT);
}

float BTerrain::get_height() const {
    return height_map.get_height();
}

void BTerrain::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_regenerate"),&BTerrain::set_regenerate);
    ClassDB::bind_method(D_METHOD("get_regenerate"),&BTerrain::get_regenerate);
     ADD_PROPERTYNZ( PropertyInfo(Variant::BOOL,"terrain/regenerate"), "set_regenerate", "get_regenerate");

    ClassDB::bind_method(D_METHOD("_terrain_material_changed"),&BTerrain::_terrain_material_changed);
    ClassDB::bind_method(D_METHOD("_update"),&BTerrain::_update);
    
    ClassDB::bind_method(D_METHOD("raycast_down"),&BTerrain::raycast_down);
    ClassDB::bind_method(D_METHOD("raycast"),&BTerrain::raycast);

    ClassDB::bind_method(D_METHOD("set_size"),&BTerrain::set_size);
    ClassDB::bind_method(D_METHOD("get_size"),&BTerrain::get_size);
    ADD_PROPERTYNZ( PropertyInfo(Variant::REAL,"terrain/size"), "set_size", "get_size");

    ClassDB::bind_method(D_METHOD("set_height"),&BTerrain::set_height);
    ClassDB::bind_method(D_METHOD("get_height"),&BTerrain::get_height);
    ADD_PROPERTYNZ( PropertyInfo(Variant::REAL,"terrain/height"), "set_height", "get_height");

    ClassDB::bind_method(D_METHOD("set_height_map_path"),&BTerrain::set_height_map_path);
    ClassDB::bind_method(D_METHOD("get_height_map_path"),&BTerrain::get_height_map_path);
    ADD_PROPERTYNZ( PropertyInfo(Variant::STRING,"terrain/height_map_path", PROPERTY_HINT_FILE, "*.tif,*.png"), "set_height_map_path", "get_height_map_path");

    ClassDB::bind_method(D_METHOD("set_normal_map_path"),&BTerrain::set_normal_map_path);
    ClassDB::bind_method(D_METHOD("get_normal_map_path"),&BTerrain::get_normal_map_path);
    ADD_PROPERTYNZ( PropertyInfo(Variant::STRING,"terrain/normal_map_path", PROPERTY_HINT_FILE, "*.png"), "set_normal_map_path", "get_normal_map_path");

    ClassDB::bind_method(D_METHOD("set_terrain_material","material"),&BTerrain::set_terrain_material);
    ClassDB::bind_method(D_METHOD("get_terrain_material"),&BTerrain::get_terrain_material);
    ADD_PROPERTYNZ( PropertyInfo(Variant::OBJECT,"terrain/terrain_material",PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_terrain_material", "get_terrain_material");
    
    ClassDB::bind_method(D_METHOD("set_skirt_material","material"),&BTerrain::set_skirt_material);
    ClassDB::bind_method(D_METHOD("get_skirt_material"),&BTerrain::get_skirt_material);
    ADD_PROPERTYNZ( PropertyInfo(Variant::OBJECT,"terrain/skirt_material",PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_skirt_material", "get_skirt_material");
	
	ClassDB::bind_method(D_METHOD("move_transform_to_ground"), &BTerrain::move_transform_to_ground);
	ClassDB::bind_method(D_METHOD("move_vector_to_ground"), &BTerrain::move_vector_to_ground);

    ClassDB::bind_method(D_METHOD("get_collision_nodes"), &BTerrain::get_collision_nodes);

    ADD_SIGNAL(MethodInfo(CoreStringNames::get_singleton()->changed));
}

BTerrain::BTerrain() {
    singleton = this;   

    terrain_generator = memnew(BTerrainGenerator);
    size = 1000;
    //height = 0;
    skirt_bottom = -20.0;
    resolution = 100;
    chunks = 5;
    castShadows = false;
    generateCollision = false;
    generateLOD = false;
    //inTree = false;
    dirty_flags = 0;
    first_set_flags = 0;
    in_update = false;
}

BTerrain::~BTerrain() {
    memdelete(terrain_generator);
    terrain_generator = NULL;

    set_terrain_material(Ref<Material>());
	
    if (singleton == this) {
        singleton = NULL;
    }
}


