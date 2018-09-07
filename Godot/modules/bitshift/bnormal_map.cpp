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
#include "bnormal_map.h"
#include "bterrain.h"
#include "butil.h"
#include "bimage_exr.h"
#include "editor/editor_file_system.h"
#include "core/os/os.h"
#include <assert.h>

void BNormalMap::set_normal_map(const Ref<Texture>& cm) {
    normal_map.set_texture(cm);
}

Ref<Texture> BNormalMap::get_normal_map() const {
    return normal_map.texture;
}

/*
Error BNormalMap::save(const String& resource_path) {
    if (resource_path.ends_with(".exr")) {
        BImageEXR image_exr;
        image_exr.save_image(normal_map.image, resource_path, Image::FORMAT_RGBF);
    }

    Error err = normal_map.image->save_png(resource_path);



    EditorFileSystem::get_singleton()->reimport_files(reimports); // TODO: HERE we need this code gameside
    //EditorFileSystem efs;
    //efs.reimport_files(reimports);

    return err;
}
*/

Error BNormalMap::load(const String& path) {

    Ref<StreamTexture> n = ResourceLoader::load(path);
    if (n.is_null())
        return FAILED;

    /*
    Ref<StreamTexture> n;
    n.instance();
    Error error = n->load(path);
    if (error != OK)
        return error;


    /*
    Ref<ImageTexture> n;
    n.instance();

    Ref<Image> i;
    i.instance();
    Error error = i->load(path);
    if (error != OK)
        return error;

    n->create_from_image(i);
*/

    set_normal_map(n);
    return OK; //error;
}

Error BNormalMap::generate_and_save_from_height_map(const String& path, const BHeightMap& height_map, float blur_size, float blur_sigma) {

    uint64_t us = OS::get_singleton()->get_ticks_usec();

    Error error = generate_from_height_map(height_map);
    if (error != OK)
        return error;

    print_line("BNormalMap generate_from_height_map took (seconds): " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));

    // guassian blur
    if (blur_size > 0.f && blur_sigma > 0.f) {
        uint64_t us = OS::get_singleton()->get_ticks_usec();
        normal_map.gaussian_blur(blur_size, blur_sigma);
        print_line("BNormalMap gaussian_blur took (seconds): " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));
    }

    {
        Ref<ImageTexture> normal_texture;
        normal_texture.instance();
        normal_texture->create_from_image(normal_map.image);

        ResourceSaver::save(path, normal_texture);

     #ifdef TOOLS_ENABLED
        // this is so we can load straight away in the editor
        if (EditorFileSystem::get_singleton()) {
            // cause the image to be "imported"
            Vector<String> reimports;
            reimports.push_back(path);
            EditorFileSystem::get_singleton()->reimport_files(reimports);
        }
      #endif

        //memdelete(normal_texture);
    }
    return OK;
}

Error BNormalMap::generate_from_height_map(const BHeightMap& height_map) {
    Error error = OK;
    //assert(height_map.get_height_map_tool().image->get_format() == Image::FORMAT_RF);


/*
    // get the path to the resource
    String height_map_path = p_height_map->get_path();
    print_line(height_map_path);

    // load the exr ourself to avoid compression/banding issues
    // we will read it as a float texture, just the red channel
    Ref<Image> height_image;
    if (height_map_path.ends_with(".exr")) {
        height_image.instance();

        BImageEXR image_exr;
        error = image_exr.load_image(height_image, height_map_path, Image::FORMAT_RF);
    }
    else {
        // not ideal, we are using a png or something hopeless to generate a normalmap!!!
        height_image = p_height_map->get_data();
        BUtil::get_singleton()->log_editor_message("Please use heightmaps that are of .exr extension, else you will have nasty banding on your heightmaps!");
    }*/

    // setup a texture io tool for easy modifcation
    //BTextureIOTool height_map;
    //height_map.set_image(height_image);

    // create a normal map of the appropriate dimensions
    Ref<Image> normal_image;
    normal_image.instance();
    normal_image->create(height_map.get_height_map_tool().width, height_map.get_height_map_tool().height, false, Image::FORMAT_RGB8); // TODO: do we need a float verion?
    normal_image->fill(Color(0.5f, 0.5f, 1.0f));

    normal_map.set_image(normal_image);

    //Ref<StreamTexture> normal_texture;
    //Ref<ImageTexture> normal_texture;
    //normal_texture.instance();

    //normal_texture->create_from_image(normal_image);
    //set_normal_map(normal_texture);

    // Weighted step list provides a blur/filtering/softening mechanism to iron out wrinkles
    Vector<WeightedStep> weightedStepList;
    weightedStepList.push_back(WeightedStep(1, 1.f));
    /*
    for (int i = 1; i <= 2; ++i)
    {
        float weight = (40.f - float(i)) / 40.f;
        weightedStepList.push_back(WeightedStep(i, weight));
    }*/

    _sobel_generate_from_height_map(height_map, weightedStepList);
    //_fabian_generate_from_height_map(height_map, weightedStepList);

    //delete normal_texture;

    return error;
}

/*

// https://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map

A common method is using a Sobel filter for a weighted/smooth derivative in each direction.

Start by sampling a 3x3 area of heights around each texel (here, [4] is the pixel we want the normal for).

[6][7][8]
[3][4][5]
[0][1][2]

Then,

//float s[9] contains above samples
vec3 n;
n.x = scale * -(s[2]-s[0]+2*(s[5]-s[3])+s[8]-s[6]);
n.y = scale * -(s[6]-s[0]+2*(s[7]-s[1])+s[8]-s[2]);
n.z = 1.0;
n = normalize(n);

Where scale can be adjusted to match the heightmap real world depth relative to its size.
*/

void BNormalMap::_sobel_generate_from_height_map(const BHeightMap& height_map, Vector<WeightedStep>& weightedStepList) {
    for (uint32_t y = 0; y < height_map.get_height_map_tool().height; ++y) {
        for (uint32_t x = 0; x < height_map.get_height_map_tool().width; ++x) {
            Vector3 normal;
            for (uint32_t w = 0; w < weightedStepList.size(); ++w) {
                WeightedStep& ws = weightedStepList.write[w];
                Vector3 step_normal = _sobel_get_normal_from_height_map(height_map, x, y, ws.step);
                normal += step_normal * ws.weight;
            }

            normal.normalize();
            normal = (normal + Vector3(1.f, 1.f, 1.f)) * 0.5f; // encode normal for 0-1 range of texture
            normal_map.image->set_pixel(x, y, BUtil::to_colour(normal));
        }
    }
}

Vector3 BNormalMap::_sobel_get_normal_from_height_map(const BHeightMap& height_map, uint32_t x, uint32_t y, int step) {
    float width = height_map.get_height_map_tool().width_m1;
    float height = height_map.get_height_map_tool().height_m1;

    float s0 = height_map.get_height_from_pixel(CLAMP(x - step, 0, width), CLAMP(y + step, 0, height));
    float s1 = height_map.get_height_from_pixel(x    , CLAMP(y + step, 0, height));
    float s2 = height_map.get_height_from_pixel(CLAMP(x + step, 0, width), CLAMP(y + step, 0, height));
    float s3 = height_map.get_height_from_pixel(CLAMP(x - step, 0, width), y    );
    //float s4 = height_map.get_height_from_pixel(x, y);
    float s5 = height_map.get_height_from_pixel(CLAMP(x + step, 0, width), y    );
    float s6 = height_map.get_height_from_pixel(CLAMP(x - step, 0, width), CLAMP(y - step, 0, height));
    float s7 = height_map.get_height_from_pixel(x    , CLAMP(y - step, 0, height));
    float s8 = height_map.get_height_from_pixel(CLAMP(x + step, 0, width), CLAMP(y - step, 0, height));

    Vector3 n;
    n.x = -((s2 - s0) + (2.0 * (s5 - s3)) + (s8 - s6)); // * 0.5f + 0.5f;
    n.y = -((s6 - s0) + (2.0 * (s7 - s1)) + (s8 - s2)); // * 0.5f + 0.5f;
    n.z = 1.f;
    n.normalize();

    return n;
}

void BNormalMap::_fabian_generate_from_height_map(const BHeightMap& height_map, Vector<WeightedStep>& weightedStepList) {
    for (uint32_t y = 0; y < height_map.get_height_map_tool().height; ++y) {
        for (uint32_t x = 0; x < height_map.get_height_map_tool().width; ++x) {
            Vector3 normal;
            for (uint32_t w = 0; w < weightedStepList.size(); ++w) {
                WeightedStep& ws = weightedStepList.write[w];
                Vector3 step_normal = _fabian_get_normal_from_height_map(height_map, x, y, ws.step);
                normal += step_normal * ws.weight;
            }

            normal.normalize();
            normal = (normal + Vector3(1.f, 1.f, 1.f)) * 0.5f; // encode normal for 0-1 range of texture
            normal_map.image->set_pixel(x, y, BUtil::to_colour(normal));
        }
    }
}

Vector3 BNormalMap::_fabian_get_normal_from_height_map(const BHeightMap& height_map, uint32_t x, uint32_t y, int step) {
    float width = height_map.get_height_map_tool().width_m1;
    float height = height_map.get_height_map_tool().height_m1;
/*
    print_line("height at (" + String::num(x) + "," + String::num(y) + ")=" + String::num(height_map.get_height_from_pixel(x, y)));
    if (x == 2048 & y == 2048) {
        int nothing = 0;
        ++nothing;
    }*/

    float up = height_map.get_height_from_pixel(x, CLAMP(y - step, 0, width));
    float down = height_map.get_height_from_pixel(x, CLAMP(y + step, 0, width));
    float left = height_map.get_height_from_pixel(CLAMP(x - step, 0, width), y);
    float right = height_map.get_height_from_pixel(CLAMP(x + step, 0, width), y);

    Vector3 n = Vector3((left - right), (down - up), 1.0);
    n.normalize();
    return n;
}
/*
Vector3 BTerrain::calculate_normal_at_pixel(float x, float y) {
    float center=get_terrain_height_at_pixel(x,y);
    return Vector3(0, center, 0);

    // smoothed normal, redundant

    float up=(y>0)?get_terrain_height_at_pixel(x,y-1):center;
    float down=(y<(height-1))?get_terrain_height_at_pixel(x,y+1):center;
    float left=(x>0)?get_terrain_height_at_pixel(x-1,y):center;
    float right=(x<(width-1))?get_terrain_height_at_pixel(x+1,y):center;

    Vector3 normal = Vector3((left - right) * height, 100.0, (up - down) * height).normalized();
    return normal;
}


Vector3 BNormalMap::_calculate_normal_at_pixel(float x, float y) {
    float center=get_terrain_height_at_pixel(x,y);
    return Vector3(0, center, 0);

    // smoothed normal, redundant

    float up=(y>0)?get_terrain_height_at_pixel(x,y-1):center;
    float down=(y<(height-1))?get_terrain_height_at_pixel(x,y+1):center;
    float left=(x>0)?get_terrain_height_at_pixel(x-1,y):center;
    float right=(x<(width-1))?get_terrain_height_at_pixel(x+1,y):center;

    Vector3 normal = Vector3((left - right) * height, 100.0, (up - down) * height).normalized();
    return normal;
}


Vector3 BNormalMap::_calculate_normal(float u, float v) {
    int w = heightmap.width;
    float x = u * w;
    float y = v * w;
    return get_terrain_height_at_pixel(x,y); //calculate_normal_at_pixel(x, y);
}

void BNormalMap::_generate_normalmap()
{

    //
    bool blur_normalmap = false;

    Image heightmap_image = heightmap->get_data();

    int width = heightmap->get_width();
    int height = heightmap->get_height();
    Image normalmap_image(width,height,0, Image::FORMAT_RGB);
    Image normalmap_blur_image(width,height,0, Image::FORMAT_RGB);
    for (int y=0;y<height;y++) {
        for (int x=0;x<width;x++) {

            Vector3 normal = calculate_normal_at_pixel(x, y);
            /*
            float center=heightmap_image.get_pixel(x,y).gray();
            float up=(y>0)?heightmap_image.get_pixel(x,y-1).gray():center;
            float down=(y<(height-1))?heightmap_image.get_pixel(x,y+1).gray():center;
            float left=(x>0)?heightmap_image.get_pixel(x-1,y).gray():center;
            float right=(x<(width-1))?heightmap_image.get_pixel(x+1,y).gray():center;

            Vector3 normal = Vector3((left - right) * height, 2.0, (up - down) * height).normalized();* /
            Color result((normal.x+1.0)*0.5, (normal.y+1.0)*0.5, (normal.z+1.0)*0.5);
            normalmap_image.put_pixel( x, y, result );
        }
    }

    // TO SLOW!
    if (blur_normalmap) {
        // blur the normal map to diffuse the banding
        int blur_dist = 5;
        const int blur_size = 7;
        float blur_matrix[blur_size][blur_size] = {
            {0.00000067, 	0.00002292, 	0.00019117, 	0.00038771, 	0.00019117, 	0.00002292, 	0.00000067},
            {0.00002292, 	0.00078634, 	0.00655965, 	0.01330373, 	0.00655965, 	0.00078633, 	0.00002292},
            {0.00019117, 	0.00655965, 	0.05472157, 	0.11098164, 	0.05472157, 	0.00655965, 	0.00019117},
            {0.00038771, 	0.01330373, 	0.11098164, 	0.22508352, 	0.11098164, 	0.01330373, 	0.00038771},
            {0.00019117, 	0.00655965, 	0.05472157, 	0.11098164, 	0.05472157, 	0.00655965, 	0.00019117},
            {0.00002292, 	0.00078633, 	0.00655965, 	0.01330373, 	0.00655965, 	0.00078633, 	0.00002292},
            {0.00000067, 	0.00002292, 	0.00019117, 	0.00038771, 	0.00019117, 	0.00002292, 	0.00000067} };

        for (int y=0;y<height;y++) {
            for (int x=0;x<width;x++) {

                Vector3 blurred_normal;
                for (int by=0;by<blur_size;by++) {
                    for (int bx=0;bx<blur_size;bx++) {
                        Vector3 pix = col_to_vec(normalmap_image.get_pixel(x + (bx * blur_dist), y + (by * blur_dist)));
                        blurred_normal += pix * blur_matrix[bx][by];
                    }
                }

                normalmap_blur_image.put_pixel( x, y, vec_to_col(blurred_normal));
            }
        }
    }

    normalmap->create(heightmap->get_width(), heightmap->get_height(),Image::FORMAT_RGB);
    normalmap->set_data(blur_normalmap ? normalmap_blur_image : normalmap_image);

}*/

Vector3 BNormalMap::get_normal_from_texel(Vector2 texel_coord) const {
    return normal_map.decompress_normal_xy(normal_map.get_texel_as_vec3_bilinear(texel_coord));
}

/*
Vector3 BNormalMap::get_normal_from_pixel(int x, int y) const {
    return normal_map.decompress_normal(normal_map.get_pixel_as_vec3(x, y));
}*/

bool BNormalMap::is_valid() const {
    return normal_map.is_valid();
}

void BNormalMap::_bind_methods() {
    /*
    ClassDB::bind_method(D_METHOD("set_normal_map","texture"),&BNormalMap::set_normal_map);
    ClassDB::bind_method(D_METHOD("get_normal_map"),&BNormalMap::get_normal_map);

    ClassDB::bind_method(D_METHOD("generate_and_save_from_height_map"),&BNormalMap::generate_and_save_from_height_map);

    ADD_PROPERTYNZ( PropertyInfo(Variant::OBJECT,"normal_map/normal_map",PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_normal_map", "get_normal_map");

    //ClassDB::bind_method(D_METHOD("save"),&BNormalMap::save);*/
}

BNormalMap::BNormalMap() {
}

BNormalMap::~BNormalMap() {
}


