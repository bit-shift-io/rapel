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
#ifndef BHEIGHT_MAP_H
#define BHEIGHT_MAP_H

#include "scene/3d/spatial.h"
#include "btexture_io_tool.h"

/**
    @author Fabian Mathews <supagu@gmail.com>

    Heightmap helper class
*/

class BHeightMap : public Spatial {

    GDCLASS(BHeightMap, Spatial)
public:

    enum NORMALISE_TYPE {
      NT_INT16_MAX, // typically geotiff 16bit int
      NT_BYTE_MAX,  // 8-bit, eg. png
    };

protected:
    BTextureIOTool height_map;

    // in a tif file, we have a int16, which has a max value of 32767, min value of -32767 (see geotiff_to_height.py),
    // so we "normalise" values between -32767 -> 32767 to give us the maximum resolution
    // this value brings that back to 0 -> 1 range so height is correct
    float height_normalise;

    // as above, we need to move the -32767 value to be zero, so requires an offset
    float height_offset;

    // max height
    float height;

protected:

    static void _bind_methods();
           
public:

    const BTextureIOTool& get_height_map_tool() const { return height_map; }
    BTextureIOTool& get_height_map_tool() { return height_map; }

    void set_height(float height);
    float get_height() const;

    // the height that is multiplied with the image pixels to get the height to the set_height
    //float get_normalised_height() const;

    void set_height_map(const Ref<Texture>& height_map);
    Ref<Texture> get_height_map() const;

    float get_height_from_texel(const Vector2& t) const; //Vector2 texel_coord) const;
    float get_height_from_pixel(int x, int y) const;

    bool is_texel_valid(const Vector2& t/*float x, float y*/) const;
    bool is_pixel_valid(int x, int y) const;

    Error load(const String& path); // loads as half float

    BHeightMap();
    ~BHeightMap();
};

#endif // BNORMAL_MAP_H
