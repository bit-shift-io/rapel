 
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
#include "bwater.h"
#include "bterrain.h"
#include "bterrain_generator.h"
#include "butil.h"
#include "bmesh_gen_tool.h"
#include "core/bind/core_bind.h"
#include "core/ustring.h"
#include "scene/main/node.h"
#include "scene/main/timer.h"
#include "scene/2d/canvas_item.h"
#include "scene/3d/spatial.h"
#include "scene/3d/mesh_instance.h"
#include "scene/resources/texture.h"
#include "core/core_string_names.h"
#include <assert.h>

BWater *BWater::singleton=NULL;

void BWater::set_regenerate(bool regen) {
   _dirty();
}

bool BWater::get_regenerate() const {
    return false;
}

void BWater::set_size(float size)
{
    this->size = size;
    _dirty();
}

float BWater::get_size() const
{
    return size;
}

void BWater::set_resolution(int resolution) {
    this->resolution = resolution;
    _dirty();
}
 
int BWater::get_resolution() const {
    return resolution;
}

void BWater::set_surface_height(float height) {
    this->surface_height = height;
    _dirty();
}

float BWater::get_surface_height() const {
    return surface_height;
}

void BWater::set_depth(float depth) {
    this->depth = depth;
    _dirty();
}

float BWater::get_depth() const {
    return depth;
}

void BWater::_notification(int p_what)
{
    switch (p_what)
    {
        case NOTIFICATION_READY: {
            _dirty();			
        } break;

        case NOTIFICATION_ENTER_TREE: {
            inTree = true;
            _dirty();
        } break;
        
        case NOTIFICATION_EXIT_TREE: {
            inTree = false;
        } break;
        
        case NOTIFICATION_ENTER_WORLD: {
            _dirty();
        } break;
        
        case NOTIFICATION_EXIT_WORLD: {
            BUtil::get_singleton()->delete_children(this);
        } break;
			
        default: {}
    }
}

void BWater::_dirty()
{
    if (!inTree) {
        return;
    }
    
    if (skirt_material.is_null() || surface_material.is_null()) {
        print_line("Please specify a skirt_material and a surface_material for water");
    }
    
    if (dirty) {
        return;
    }
    
    dirty = true;
    call_deferred("_update");
}

void BWater::set_surface_material(const Ref<Material>& material) {
    if (surface_material.is_valid()) {
        surface_material->remove_change_receptor(this);
    }

    surface_material = material;

    if (surface_material.is_valid()) {
        surface_material->add_change_receptor(this);
        //surface_material->connect(CoreStringNames::get_singleton()->changed, this, "_material_changed");
    }

    _dirty();
}

Ref<Material> BWater::get_surface_material() const {
    return surface_material;
}

void BWater::set_skirt_material(const Ref<Material>& material) {
    if (skirt_material.is_valid()) {
        skirt_material->remove_change_receptor(this);
    }

    skirt_material = material;

    if (skirt_material.is_valid()) {
        skirt_material->add_change_receptor(this);
        //skirt_material->connect(CoreStringNames::get_singleton()->changed, this, "_material_changed");
    }

    _dirty();
}

Ref<Material> BWater::get_skirt_material() const {
    return skirt_material;
}

void BWater::set_chunks(int chunks) {
    this->chunks = chunks;
    _dirty();
}
    
int BWater::get_chunks() const {
    return chunks;
}

void BWater::_material_changed() {
    _dirty();
}

void BWater::_changed_callback(Object *p_changed, const char *p_prop) {
    _dirty();
}

void BWater::_update()
{
    if (!dirty) {
        return;
    }
    dirty = false;
    
    BUtil::get_singleton()->delete_children(this);
    
    Ref<Mesh> mesh = BUtil::get_singleton()->create_tessellated_quad(MAX(1, resolution / chunks));
    
    /* DO this if we add support for wave height
    // tweak bounds
    AABB aabb = mesh->get_aabb();
    aabb.size.y += (float)height;
    mesh->set_custom_aabb(aabb);
    */
    
    assert(mesh.is_valid());
    create_lod("lod1", 0, 999999, mesh);
}



// same as in BTerrain! if you fix something in here, you might need to fix in there!
Spatial* BWater::create_lod(const String& name, int near, int far, Ref<Mesh>& mesh)
{
    float chunk_step = size / chunks;
    Vector3 origin(-(size - chunk_step)/2.0, 0.0, -(size - chunk_step)/2.0); // minus one chunk  
    
    Spatial* lod = memnew( Spatial() );
    add_child(lod); // reparent
    lod->set_name(name); // leave this to client?
    
    for (int y = 0; y < chunks; ++y)
    {
        for (int x = 0; x < chunks; ++x)
        {
            MeshInstance* chunk = memnew( MeshInstance() );
            chunk->set_name("chunk");
            lod->add_child(chunk);
            chunk->set_mesh(mesh);
            
            // create transform
            Transform xform;
            xform = xform.scaled(Vector3(chunk_step, 1.0, chunk_step));
            xform.origin = Vector3(x * chunk_step, 0, y * chunk_step) + origin + Vector3(0, surface_height, 0);
            chunk->set_transform(xform);

            Ref<Material> mat = BTerrainGenerator::duplicate_material(surface_material, Transform());
            chunk->set_material_override(mat);            
            chunk->set_cast_shadows_setting(/*castShadows ? GeometryInstance::SHADOW_CASTING_SETTING_ON : */GeometryInstance::SHADOW_CASTING_SETTING_OFF);
            /*
            chunk->set_draw_range_begin(near);
            chunk->set_draw_range_end(far);  
             */
            
            // temporary while trying to work out some raycasting issues
            //chunk->create_trimesh_collision();
        }
    }
    
    
    // create skirt
    // need a single skirt around the whole perimiter as we are using transparency
    Transform xform;
    xform = xform.scaled(Vector3(size, 1.0, size));
    xform.origin = Vector3(0, surface_height, 0);
            
    Ref<Mesh> skirt_mesh = create_skirt(MAX(1, resolution), Transform());
                
    MeshInstance* chunk_skirt = memnew( MeshInstance() );
    chunk_skirt->set_name("chunk_skirt");
    lod->add_child(chunk_skirt);
    chunk_skirt->set_mesh(skirt_mesh);
    chunk_skirt->set_transform(xform);

    Ref<Material> mat = BTerrainGenerator::duplicate_material(skirt_material, Transform());
    chunk_skirt->set_material_override(mat); 
    
    return lod;
}


// Given an x and y location, convert that to world space position and add verts for the top of the skirt at terrain height
// and some bottom point specified by depth
void BWater::add_skirt_verts_for_point(BMeshGenTool& mesh_gen, int x, int y, int resolution, const Transform& uvXform) {
    float size = 1.0;
    Vector3 origin(-size/2.0, 0.0, -size/2.0);
    float resolution_step = size / resolution;
    float uv_step = 1.0 / resolution;

    mesh_gen.set_next_point(Vector3(x * resolution_step, 0.0f, y * resolution_step) + origin);
    mesh_gen.set_next_uv(Vector2(x * uv_step, y * uv_step));

    // add the bottom of the skirt
    mesh_gen.set_next_point(Vector3(x * resolution_step, -depth, y * resolution_step) + origin);
    mesh_gen.set_next_uv(Vector2(x * uv_step, y * uv_step));
}

// exactly the same as BTerrain::create_skirt, maybe setup this in its own class
// like chunked mesh generator?
// then a func ptr to change from terrain to water?
Ref<Mesh> BWater::create_skirt(int resolution, const Transform& uvXform) {       
    int pointsWidth = (resolution + 1);
    int pointCount = (resolution * 4) * 2;
    int quadCount = resolution * 4;
    int indexCount = quadCount * 2 * 3;
   
    BMeshGenTool mesh_gen;
    mesh_gen.resize_verts(pointCount);
    mesh_gen.resize_indicies(indexCount);
    
    // make all normals face directly upwards
    mesh_gen.populate_all_normals_and_tangents();
    
    /*
    //             < edge 3
    //              -------
    //             |       |
    //  \/ edge 4  |       | edge2 /\
    //             |_______|
    //              edge1 >
    */
    
    // edge 1
    int y = 0;
    for (int x = 1; x < pointsWidth; ++x)
    {
        add_skirt_verts_for_point(mesh_gen, x, y, resolution, uvXform);
    }
    
    // edge 2
    int x = pointsWidth - 1;
    for (int y = 1; y < pointsWidth; ++y)
    {
        add_skirt_verts_for_point(mesh_gen, x, y, resolution, uvXform);
    }
    
    // edge 3
    y = pointsWidth - 1;
    for (int x = (resolution - 1); x >= 0; --x)
    {
        add_skirt_verts_for_point(mesh_gen, x, y, resolution, uvXform);
    }
    
    // edge 4
    x = 0;
    for (int y = (resolution - 1); y >= 0; --y)
    {
        add_skirt_verts_for_point(mesh_gen, x, y, resolution, uvXform);
    }
    
    
    // generate indicies
    int qi = 0; // quad index
    for (int q = 0; q < quadCount - 1; ++q)
    {
        mesh_gen.set_next_triangle_indicies(qi, qi + 1, qi + 3);
        mesh_gen.set_next_triangle_indicies(qi, qi + 3, qi + 2);
        qi+=2;
    }
    
    // add the last quad which wraps around
    mesh_gen.set_next_triangle_indicies(qi, qi + 1, 1);
    mesh_gen.set_next_triangle_indicies(qi, 1, 0);

    return mesh_gen.generate_mesh();
}

void BWater::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_regenerate"),&BWater::set_regenerate);
    ClassDB::bind_method(D_METHOD("get_regenerate"),&BWater::get_regenerate);
     ADD_PROPERTY( PropertyInfo(Variant::BOOL,"terrain/regenerate"), "set_regenerate", "get_regenerate");

    ClassDB::bind_method(D_METHOD("_material_changed"),&BWater::_material_changed);
    ClassDB::bind_method(D_METHOD("_update"),&BWater::_update);

    ClassDB::bind_method(D_METHOD("set_size"),&BWater::set_size);
    ClassDB::bind_method(D_METHOD("get_size"),&BWater::get_size);
    ADD_PROPERTY( PropertyInfo(Variant::REAL,"water/size"), "set_size", "get_size");
    
    ClassDB::bind_method(D_METHOD("set_chunks"),&BWater::set_chunks);
    ClassDB::bind_method(D_METHOD("get_chunks"),&BWater::get_chunks);
    ADD_PROPERTY( PropertyInfo(Variant::INT,"water/chunks"), "set_chunks", "get_chunks");
    
    ClassDB::bind_method(D_METHOD("set_resolution"),&BWater::set_resolution);
    ClassDB::bind_method(D_METHOD("get_resolution"),&BWater::get_resolution);
    ADD_PROPERTY( PropertyInfo(Variant::INT,"water/resolution"), "set_resolution", "get_resolution");
    
    ClassDB::bind_method(D_METHOD("set_surface_height"),&BWater::set_surface_height);
    ClassDB::bind_method(D_METHOD("get_surface_height"),&BWater::get_surface_height);
    ADD_PROPERTY( PropertyInfo(Variant::REAL,"water/surface_height"), "set_surface_height", "get_surface_height");

    ClassDB::bind_method(D_METHOD("set_depth"),&BWater::set_depth);
    ClassDB::bind_method(D_METHOD("get_depth"),&BWater::get_depth);
    ADD_PROPERTY( PropertyInfo(Variant::REAL,"water/depth"), "set_depth", "get_depth");
    
    ClassDB::bind_method(D_METHOD("set_surface_material","material"),&BWater::set_surface_material);
    ClassDB::bind_method(D_METHOD("get_surface_material"),&BWater::get_surface_material);
    ADD_PROPERTY( PropertyInfo(Variant::OBJECT,"water/surface_material",PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_surface_material", "get_surface_material");
    
    ClassDB::bind_method(D_METHOD("set_skirt_material","material"),&BWater::set_skirt_material);
    ClassDB::bind_method(D_METHOD("get_skirt_material"),&BWater::get_skirt_material);
    ADD_PROPERTY( PropertyInfo(Variant::OBJECT,"water/skirt_material",PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_skirt_material", "get_skirt_material");
}

BWater::BWater() {
    singleton = this;  
    dirty = false;
    inTree = false;
    surface_height = 0.0f;
    depth = 50.0;
    size = 1000;
    resolution = 10;
    chunks = 5;

    /*
    PropertyEditor* pe = EditorNode::get_singleton()->get_property_editor();
    pe->connect("variant_changed", this, "_property_changed");*/
}

BWater::~BWater() {
    if (singleton == this) {
        singleton = NULL;
    }
}


