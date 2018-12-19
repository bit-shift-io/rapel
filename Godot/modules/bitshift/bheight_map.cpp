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
#include "bheight_map.h"
#include "bterrain.h"
#include "butil.h"
#include "bimage_exr.h"
#include "editor/editor_file_system.h"
#include <assert.h>

void BHeightMap::set_height_map(const Ref<Texture>& cm) {
    height_map.set_texture(cm);

    String height_map_path = cm->get_path();

    float min, max;
    height_map.compute_min_max(min, max);
    height_offset = -min;
    height_normalise = 1.f / (max - min);


    // some tests
    float valueA = 32000;
    float heightA = (valueA + height_offset) * height * height_normalise;

    float valueB = 1;
    float heightB = (valueB + height_offset) * height * height_normalise;

    int nothing = 0;
    ++nothing;
}

Ref<Texture> BHeightMap::get_height_map() const {
    return height_map.texture;
}

void BHeightMap::set_height(float h) {
    height = h;
}

float BHeightMap::get_height() const {
    return height;
}

Error BHeightMap::load(const String& path) {
    Ref<StreamTexture> n;
    /*
    if (path.ends_with("png")) {
        ImageFormatLoader* orig_loader = ImageLoader::recognize("png");
        ImageLoader::remove_image_format_loader(orig_loader);

        BImageLoaderPNG new_loader;
        ImageLoader::add_image_format_loader(&new_loader);

        n = ResourceLoader::load(path);

        ImageLoader::remove_image_format_loader(&new_loader);
        ImageLoader::add_image_format_loader(orig_loader);
    }
    else */{
        n = ResourceLoader::load(path);
    }

    if (n.is_null())
        return FAILED;

    set_height_map(n);
    return OK; //error;
}

float BHeightMap::get_height_from_texel(const Vector2& texel_coord) const {
    Vector3 out_pos = height_map.get_texel_as_vec3_bilinear(texel_coord); //texel_coord.x, texel_coord.y);
            //get_texel_as_vec3_bilinear(texel_coord.x, texel_coord.y);
    float result = (out_pos.x + height_offset) * height * height_normalise;
    if (result < 0) {
        int nothing = 0;
        ++nothing;
    }
    return result;
}

float BHeightMap::get_height_from_pixel(int x, int y) const {
    float px = x;
    float py = y;
    height_map.pixel_space_to_texel_space(px, py);
    return get_height_from_texel(Vector2(px, py));
}

bool BHeightMap::is_texel_valid(const Vector2& t/*float x, float y*/) const {
    if (!height_map.is_valid())
        return false;

    return height_map.is_texel_valid(t); //x, y);
}

bool BHeightMap::is_pixel_valid(int x, int y) const {
    return height_map.is_pixel_valid(x, y);
}

void BHeightMap::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_height_map","texture"),&BHeightMap::set_height_map);
    ClassDB::bind_method(D_METHOD("get_height_map"),&BHeightMap::get_height_map);

    ADD_PROPERTY( PropertyInfo(Variant::OBJECT,"height_map/height_map",PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_height_map", "get_height_map");

}

BHeightMap::BHeightMap() {
    height = 0.f;
    height_normalise = 1.f;
    height_offset = 0.f;
}

BHeightMap::~BHeightMap() {
}


