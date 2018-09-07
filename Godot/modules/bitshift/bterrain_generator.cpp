 
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
#include "bterrain_generator.h"
#include "butil.h"
#include "bmaterial.h"
#include "bmesh_gen_tool.h"
#include "core/bind/core_bind.h"
#include "core/ustring.h"
#include "scene/main/node.h"
#include "scene/main/timer.h"
#include "scene/2d/canvas_item.h"
#include "scene/3d/spatial.h"
#include "scene/3d/mesh_instance.h"
#include "scene/3d/physics_body.h"
#include "scene/3d/collision_shape.h"
#include "scene/resources/texture.h"
#include "core_string_names.h"
#include <assert.h>

AABB BTerrainGenerator::compute_aabb(float size, float height) {
    float half_size = size / 2;
    AABB aabb;
    aabb.expand_to(Vector3(half_size, height, half_size));
    aabb.expand_to(Vector3(-half_size, 0.f, -half_size));
    aabb.grow_by(1.0f);
    return aabb;
}

void BTerrainGenerator::_create_lod_thread_function(void *self) {
    BTerrainGenerator* tgen = (BTerrainGenerator*)self;
    tgen->_create_lod();
}

// same as in BWater! if you fix something in here, you might need to fix in there!
void BTerrainGenerator::create_lod(MeshInputData& meshInputData) {
    mesh_input_data = meshInputData;
    thread = Thread::create(_create_lod_thread_function, this);
}

void BTerrainGenerator::_create_lod() {
    uint64_t us = OS::get_singleton()->get_ticks_usec();

    int total_quads = Math::pow(mesh_input_data.size * mesh_input_data.quads_per_metre, 2);
    int total_chunks = total_quads / mesh_input_data.quads_per_chunk;
    int chunks = MAX(1, Math::sqrt((float)total_chunks)); // convert back to a single dimension eg. from m2 to metres

    int resolution = mesh_input_data.size * mesh_input_data.quads_per_metre;

    float chunk_step = mesh_input_data.size / chunks;
    float uv_step = 1.0 / chunks;
    Vector3 origin(-(mesh_input_data.size - chunk_step)/2.0, 0.0, -(mesh_input_data.size - chunk_step)/2.0); // minus one chunk
    
    Node* parent = mesh_input_data.parent;
    
    //AABB aabb;

    for (int y = 0; y < chunks; ++y)
    {
        for (int x = 0; x < chunks; ++x)
        {
            Transform uvXform;
            uvXform = uvXform.scaled(Vector3(uv_step, uv_step, 1.0));
            uvXform.origin = Vector3(x * uv_step, y * uv_step, 0.0);   // position 
            
            Ref<Mesh> mesh = create_terrain(MAX(1, resolution / chunks), x, y, uvXform);

            MeshInstance* chunk = memnew( MeshInstance() );
            chunk->set_name("chunk");
            if (parent) {
                parent->add_child(chunk);
            }
            chunk->set_mesh(mesh);
            
            // create transform
            Transform xform;
            xform = xform.scaled(Vector3(chunk_step, 1.0, chunk_step));
            xform.origin = Vector3(x * chunk_step, 0, y * chunk_step) + origin;            
            chunk->set_transform(xform);
            
            
            Ref<Material> mat = duplicate_material(mesh_input_data.terrain_material, uvXform);
            chunk->set_material_override(mat); 
            
            chunk->set_cast_shadows_setting(!!(mesh_input_data.flags & F_NO_SHADOWS) ? GeometryInstance::SHADOW_CASTING_SETTING_OFF : GeometryInstance::SHADOW_CASTING_SETTING_ON);
            /*
            chunk->set_draw_range_begin(near);
            chunk->set_draw_range_end(far);  
             */
            
            AABB chunk_aabb = chunk->get_aabb();
            // check for AABB validity
            #ifdef DEBUG_ENABLED
                ERR_FAIL_COND(chunk_aabb.position.x > 1e15 || chunk_aabb.position.x < -1e15);
                ERR_FAIL_COND(chunk_aabb.position.y > 1e15 || chunk_aabb.position.y < -1e15);
                ERR_FAIL_COND(chunk_aabb.position.z > 1e15 || chunk_aabb.position.z < -1e15);
                ERR_FAIL_COND(chunk_aabb.size.x > 1e15 || chunk_aabb.size.x < 0.0);
                ERR_FAIL_COND(chunk_aabb.size.y > 1e15 || chunk_aabb.size.y < 0.0);
                ERR_FAIL_COND(chunk_aabb.size.z > 1e15 || chunk_aabb.size.z < 0.0);
                ERR_FAIL_COND(Math::is_nan(chunk_aabb.size.x));
                ERR_FAIL_COND(Math::is_nan(chunk_aabb.size.y));
                ERR_FAIL_COND(Math::is_nan(chunk_aabb.size.z));

            #endif

            // temporary while trying to work out some raycasting issues
            //Node* collision = chunk->create_trimesh_collision_node();
            //chunk->add_child(collision);
            //collisionNodes.push_back(collision);

            //if (x == 0 && y == 0)
            //    aabb = xform.xform(chunk_aabb);
            //else
            //    aabb = aabb.merge(xform.xform(chunk_aabb));
            
            // create skirt
            if (x == 0 || x == (chunks - 1) || y == 0 || y == (chunks - 1)) {
                Ref<Mesh> skirt_mesh = create_skirt(MAX(1, resolution / chunks), uvXform);
                
                MeshInstance* chunk_skirt = memnew( MeshInstance() );
                chunk_skirt->set_name("chunk_skirt");
                if (parent) {
                    parent->add_child(chunk_skirt);
                }
                chunk_skirt->set_mesh(skirt_mesh);
                chunk_skirt->set_transform(xform);
                
                Ref<Material> mat = duplicate_material(mesh_input_data.skirt_material, Transform());
                chunk_skirt->set_material_override(mat); 
            }
        }
    }

    // add some padding to stop cases where the terrain skims right along the aabb, so collision isnt registering
    //aabb.grow_by(1.0f);
    
    print_line("BTerrainGenerator::create_lod took (seconds): " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));

    //MeshOutputData mod;
    //mod.aabb = aabb;
    //return mod;
}

StaticBody *BTerrainGenerator::create_collision(MeshInputData& meshInputData) {
    uint64_t us = OS::get_singleton()->get_ticks_usec();
    mesh_input_data = meshInputData;

    // in future it might be best to optimise this by implementing this ourself instead:
    /*
    Ref<Shape> Mesh::create_trimesh_shape() const {

        PoolVector<Face3> faces = get_faces();
        if (faces.size() == 0)
            return Ref<Shape>();

        PoolVector<Vector3> face_points;
        face_points.resize(faces.size() * 3);

        for (int i = 0; i < face_points.size(); i++) {

            Face3 f = faces.get(i / 3);
            face_points.set(i, f.vertex[i % 3]);
        }

        Ref<ConcavePolygonShape> shape = memnew(ConcavePolygonShape);
        shape->set_faces(face_points);
        return shape;
    }
    */

    // for now, just generate terrain as a mesh and let godot do the slow? conversion

    Transform uvXform;
    Ref<Mesh> mesh = create_terrain(mesh_input_data.size * mesh_input_data.quads_per_metre, 0, 0, uvXform);
    Ref<Shape> shape = mesh->create_trimesh_shape();

    StaticBody *static_body = memnew(StaticBody);
    CollisionShape *cshape = memnew(CollisionShape);
    cshape->set_shape(shape);
    static_body->add_child(cshape);

    print_line("BTerrainGenerator::create_collision took (seconds): " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));
    return static_body;
/*
    // temporary while trying to work out some raycasting issues
    Node* collision = chunk->create_trimesh_collision_node();
    chunk->add_child(collision);
    collisionNodes.push_back(collision);
*/
}

Ref<Material> BTerrainGenerator::duplicate_material(Ref<Material>& material, const Transform& uvTransform) {
    // setup the material
    // ready to be used on the terrain
    ERR_FAIL_COND_V(!material.is_valid(), Ref<Material>());

    Ref<ShaderMaterial> mat = SAFE_CAST<ShaderMaterial *>(material.ptr());
    if (mat.is_valid()) {
        mat = material->duplicate();
        mat->set_shader_param("uv_transform", uvTransform);
        //mat->set_shader_param("height_range", height);
        //assert(heightmap.is_valid());
        //assert(heightmap.is_square()); // expect a square heightmap
        //mat->set_shader_param("heightmap_pixel_step", Vector3(-1.0 / heightmap.width, 0, 1.0 / heightmap.height));
        return mat; 
    }
    
    Ref<BSpatialMaterial> bmat = SAFE_CAST<BSpatialMaterial *>(material.ptr());
    if (bmat.is_valid()) {
        bmat = material->duplicate();
        Vector3 scale = uvTransform.get_basis().get_scale();
        bmat->set_uv1_scale(Vector3(scale.x, scale.y, 1.f));
        bmat->set_uv1_offset(Vector3(uvTransform.origin.x, uvTransform.origin.y, 1.f));
        return bmat;
    }

    Ref<SpatialMaterial> spmat = material->duplicate();
    Vector3 scale = uvTransform.get_basis().get_scale();
    spmat->set_uv1_scale(Vector3(scale.x, scale.y, 1.f));
    spmat->set_uv1_offset(Vector3(uvTransform.origin.x, uvTransform.origin.y, 1.f));
    return spmat;
}

// Given an x and y location, convert that to world space position and add verts for the top of the skirt at terrain height
// and some bottom point specified by skirt_bottom
void BTerrainGenerator::add_skirt_verts_for_point(BMeshGenTool& mesh_gen, int x, int y, int resolution, const Transform& uvXform) {
    float size = 1.0;
    Vector3 origin(-size/2.0, 0.0, -size/2.0);
    float resolution_step = size / resolution;
    float uv_step = 1.0 / resolution;
    
    Vector3 xformedUV = uvXform.xform(Vector3(x * uv_step, y * uv_step, 0.0)); 
    float height = mesh_input_data.height_map->get_height_from_texel(Vector2(xformedUV.x, xformedUV.y));
    //Vector3 c = heightmap.get_pixel_as_vec3(xformedUV.x * heightmap.width_m1, xformedUV.y * heightmap.height_m1);
    //float pixelHeight = c.x * height * height_normalise;

    mesh_gen.set_next_point(Vector3(x * resolution_step, height, y * resolution_step) + origin);
    mesh_gen.set_next_uv(Vector2(x * uv_step, y * uv_step));

    // add the bottom of the skirt
    mesh_gen.set_next_point(Vector3(x * resolution_step, mesh_input_data.skirt_bottom, y * resolution_step) + origin);
    mesh_gen.set_next_uv(Vector2(x * uv_step, y * uv_step));
}

Ref<Mesh> BTerrainGenerator::create_skirt(int resolution, const Transform& uvXform) {
    assert(mesh_input_data.height_map->get_height_map_tool().is_valid());

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

Ref<Mesh> BTerrainGenerator::create_terrain(int resolution, int x, int y, const Transform& uvXform) {

    // TODO: check for flatness - then we can just put in a quad, as a lot of underwater should be flat right?
    assert(mesh_input_data.height_map->get_height_map_tool().is_valid());
    assert(mesh_input_data.height_map->get_height_map_tool().is_square());
    
    float size = 1.0;
    Vector3 origin(-size/2.0, 0.0, -size/2.0);
    float resolution_step = size / resolution;
    float uv_step = 1.0 / resolution;
    
    int pointsWidth = (resolution + 1);
    int pointCount = (resolution + 1) * (resolution + 1);
    int triCount = resolution * resolution * 2;
    int indexCount = triCount * 3;
    
    BMeshGenTool mesh_gen;
    mesh_gen.resize_verts(pointCount);
    mesh_gen.resize_indicies(indexCount);
    
    // make all normals face directly upwards
    mesh_gen.populate_all_normals_and_tangents();
        
    // generate verts
    for (int y = 0; y < pointsWidth; ++y)
    {
        for (int x = 0; x < pointsWidth; ++x)
        {
            mesh_gen.set_next_uv(Vector2(x * uv_step, y * uv_step));
            
            Vector3 xformedUV = uvXform.xform(Vector3(x * uv_step, y * uv_step, 0.0));
            float height = mesh_input_data.height_map->get_height_from_texel(Vector2(xformedUV.x, xformedUV.y));
            //Vector3 c = heightmap.get_pixel_as_vec3(xformedUV.x * heightmap.width_m1, xformedUV.y * heightmap.height_m1);
            //float pixelHeight = c.x * height * height_normalise;
            mesh_gen.set_next_point(Vector3(x * resolution_step, height, y * resolution_step) + origin);
        }
    }
    
    // generate indices
    int q = 0; // what quad are we generating?
    int qi = 0; // quad index
    for (int i = 0; i < indexCount; i += 6)
    {
        // reference: https://github.com/Zylann/godot_terrain_plugin/blob/master/addons/zylann.terrain/terrain.gd
        int p00 = qi;
        int p01 = qi + 1;
        int p11 = qi + pointsWidth + 1;
        int p10 = qi + pointsWidth;
        
        // quad adaption
        bool reverse_quad = ABS(mesh_gen.get_point(p00).y - mesh_gen.get_point(p11).y) > ABS(mesh_gen.get_point(p10).y - mesh_gen.get_point(p01).y);
        
        // 00---01
	//  |  /|
	//  | / |
	//  |/  |
        // 10---11
        if (reverse_quad) {
            mesh_gen.set_next_triangle_indicies(p00, p01, p10);
            mesh_gen.set_next_triangle_indicies(p01, p11, p10);
        }
        // 00---01
	//  |\  |
	//  | \ |
	//  |  \|
        // 10---11
        else {
            mesh_gen.set_next_triangle_indicies(p00, p01, p11);
            mesh_gen.set_next_triangle_indicies(p00, p11, p10);
        }
        
        ++qi;
        ++q;
        
        // skip the extra vert at the end of the row as we jump back to the start
        if (q != 0 && (q % resolution) == 0) {
            ++qi;
        }
    }
                
    return mesh_gen.generate_mesh();
}

BTerrainGenerator::BTerrainGenerator() {

}

BTerrainGenerator::~BTerrainGenerator() {

}


