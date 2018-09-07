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
#ifndef BTEXTURE_IO_TOOL_H
#define BTEXTURE_IO_TOOL_H

#include "reference.h"
#include "core/image.h"
#include "butil.h"
#include "math/vector3.h"
#include "scene/resources/texture.h"
#include "thirdparty/pvrtccompressor/ColorRgba.h"
#include <assert.h>

class Texture;
using namespace Javelin;

/**
	@author Fabian Mathews <supagu@gmail.com>
 * 
 * This serves as a wrapper around a Texture and stores some vars to make for quick pixel looks
 *
 * Pixel space = the x and y refer to the pixel in the texture as an int from 0 to texture size
 * texel space = texture space, values from 0 to 1
*/

class BTextureIOTool  {

        
protected:

        
public:

	// are the p_x and p_y coords inside the texture?
	inline bool is_pixel_valid(uint32_t p_x, uint32_t p_y) const {
		if (p_x < 0 || p_x >= width)
			return false;
		
		if (p_y < 0 || p_y >= height)
			return false;
		
		return true;
	}
	
    inline void texture_space_to_pixel_space(float& p_x, float& p_y) const {
        p_x *= width_m1;
        p_y *= height_m1;
    }

    inline void pixel_space_to_texel_space(float& p_x, float& p_y) const {
        p_x /= width_m1;
        p_y /= height_m1;
    }

    // p_x, p_y are values from 0 to 1
    inline bool is_texel_valid(const Vector2& t) const { //float p_x, float p_y) const {
        /*
        float p_x = t.x;
        float p_y = t.y;
        texture_space_to_pixel_space(p_x, p_y);
        return is_pixel_valid(Math::floor(p_x), Math::floor(p_y)) && is_pixel_valid(Math::ceil(p_x), Math::ceil(p_y));*/

        if ((t.x >= 0.f && t.x <= 1.f)
           && (t.y >= 0.f && t.y <= 1.f)) {
            return true;
        }

        return false;
    }
    
    // bilinear
    // https://en.wikipedia.org/wiki/Bilinear_interpolation
    inline Vector3 get_texel_as_vec3_bilinear(const Vector2& t) const {
        float p_x = t.x;
        float p_y = t.y;
        texture_space_to_pixel_space(p_x, p_y);

        // round down
        int32_t i_x = Math::floor(p_x);
        int32_t i_y = Math::floor(p_y);

        // remainder
        float r_x = p_x - i_x;
        float r_y = p_y - i_y;
        
        // + or - 1 based on remainder
        int32_t e_x = r_x > 0.f ? 1 : -1;
        int32_t e_y = r_y > 0.f ? 1 : -1;
        
        //
        // a             b
        //  -------------
        //  |           |
        //  |           |
        //  |           |
        //  -------------
        // d             c
        
        Vector3 a = get_pixel_as_vec3(i_x, i_y);
        Vector3 b = get_pixel_as_vec3(CLAMP(i_x + e_x, 0, width_m1), i_y);
        Vector3 c = get_pixel_as_vec3(CLAMP(i_x + e_x, 0, width_m1), CLAMP(i_y + e_y, 0, height_m1));
        Vector3 d = get_pixel_as_vec3(i_x, CLAMP(i_y + e_y, 0, height_m1));
        
        Vector3 x_1 = lerp(a, b, r_x);
        Vector3 x_2 = lerp(d, c, r_x);
        
        Vector3 y = lerp(x_1, x_2, r_y);
        return y;

        //return a;
    }

    // t is a value that goes from 0 to 1 to interpolate in a C1 continuous way across uniformly sampled data points.
    // when t is 0, this will return B.  When t is 1, this will return C.  Inbetween values will return an interpolation
    // between B and C.  A and B are used to calculate slopes at the edges.
    inline float CubicHermite(float A, float B, float C, float D, float t) const
    {
        float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
        float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
        float c = -A / 2.0f + C / 2.0f;
        float d = B;

        return a*t*t*t + b*t*t + c*t + d;
    }

    // bicubic
    // https://en.wikipedia.org/wiki/Bicubic_interpolation
    // https://blog.demofox.org/2015/08/15/resizing-images-with-bicubic-interpolation/
    inline Vector3 get_texel_as_vec3_bicubic(const Vector2& t) const {
        float p_x = t.x;
        float p_y = t.y;
        texture_space_to_pixel_space(p_x, p_y);

        // round down
        int32_t i_x = p_x;
        int32_t i_y = p_y;

        // remainder
        float xfract = p_x - Math::floor(p_x);
        float yfract = p_y - Math::floor(p_y);

        // 1st row
        auto p00 = get_pixel_as_vec3_clamped(i_x - 1, i_y - 1);
        auto p10 = get_pixel_as_vec3_clamped(i_x + 0, i_y - 1);
        auto p20 = get_pixel_as_vec3_clamped(i_x + 1, i_y - 1);
        auto p30 = get_pixel_as_vec3_clamped(i_x + 2, i_y - 1);

        // 2nd row
        auto p01 = get_pixel_as_vec3_clamped(i_x - 1, i_y + 0);
        auto p11 = get_pixel_as_vec3_clamped(i_x + 0, i_y + 0);
        auto p21 = get_pixel_as_vec3_clamped(i_x + 1, i_y + 0);
        auto p31 = get_pixel_as_vec3_clamped(i_x + 2, i_y + 0);

        // 3rd row
        auto p02 = get_pixel_as_vec3_clamped(i_x - 1, i_y + 1);
        auto p12 = get_pixel_as_vec3_clamped(i_x + 0, i_y + 1);
        auto p22 = get_pixel_as_vec3_clamped(i_x + 1, i_y + 1);
        auto p32 = get_pixel_as_vec3_clamped(i_x + 2, i_y + 1);

        // 4th row
        auto p03 = get_pixel_as_vec3_clamped(i_x - 1, i_y + 2);
        auto p13 = get_pixel_as_vec3_clamped(i_x + 0, i_y + 2);
        auto p23 = get_pixel_as_vec3_clamped(i_x + 1, i_y + 2);
        auto p33 = get_pixel_as_vec3_clamped(i_x + 2, i_y + 2);

        // interpolate bi-cubically!
        // Clamp the values since the curve can put the value below 0 or above 255
        float ret[3];
        for (int i = 0; i < 3; ++i)
        {
            float col0 = CubicHermite(p00[i], p10[i], p20[i], p30[i], xfract);
            float col1 = CubicHermite(p01[i], p11[i], p21[i], p31[i], xfract);
            float col2 = CubicHermite(p02[i], p12[i], p22[i], p32[i], xfract);
            float col3 = CubicHermite(p03[i], p13[i], p23[i], p33[i], xfract);
            float value = CubicHermite(col0, col1, col2, col3, yfract);
            //CLAMP(value, 0.0f, 255.0f);
            ret[i] = value;
        }

        return Vector3(ret[0], ret[1], ret[2]);
    }

    inline Vector3 lerp(const Vector3& a, const Vector3& b, float c) const {
        return a + (b - a) * c;
    }
        
    // inline for speeed - decomppress normal packed in xy
    inline Vector3 decompress_normal_xy(const Vector3& compressed) const {
        Vector2 normal = (Vector2(compressed.x, compressed.y)  * 2.f) - Vector2(1.f, 1.f);
        normal.y = -normal.y;
        float z = Math::sqrt(MAX(0.f, 1.f - (normal.x * normal.x) - (normal.y * normal.y)));

        // check for dodgey normal map
#ifdef DEBUG_ENABLED
        if (Math::is_nan(z)) {
            print_error("Normalmap has computed NaN for Z! BAD NORMALMAP!!!");
        }
#endif
        return Vector3(normal.x, z, normal.y);
    }

    inline Vector3 decompress_normal_xyz(const Vector3& compressed) const {
        Vector3 normal = (compressed * 2.f) - Vector3(1.f, 1.f, 1.f);
        normal.y = -normal.y;
        return Vector3(normal.x, normal.z, normal.y);
    }
    
    inline Vector3 get_pixel_as_vec3_clamped(uint32_t p_x, uint32_t p_y) const {
        return BUtil::to_vector3(image->get_pixel(CLAMP(p_x, 0, width_m1), CLAMP(p_y, 0, height_m1)));
    }

    // inline for speeed
    inline Vector3 get_pixel_as_vec3(uint32_t p_x, uint32_t p_y) const {
        assert(p_x >= 0 && p_x < width);
        assert(p_y >= 0 && p_y < height);
        return BUtil::to_vector3(image->get_pixel(p_x, p_y));
    }

    // inline for speeed
    inline Vector3 get_texel_as_vec3(const Vector2& uv) const { //float p_x, float p_y) const {
        float p_x = uv.x;
        float p_y = uv.y;
        texture_space_to_pixel_space(p_x, p_y);
        return get_pixel_as_vec3(p_x, p_y);
    }

    inline Color get_texel(const Vector2& uv) const {
        float x = uv.x;
        float y = uv.y;
        texture_space_to_pixel_space(x, y);
        return image->get_pixel(x, y);
    }
    
    inline Color get_pixel(int x, int y) const {
        return image->get_pixel(x, y);
    }

    // clamped to stop buffer under/over runs
    inline Color get_pixel_safe(int x, int y) const {
        return image->get_pixel(CLAMP(x, 0, width_m1), CLAMP(y, 0, height_m1));
    }

    inline void set_pixel(int x, int y, const Color& color) const {
        image->set_pixel(x, y, color);
    }
    
    void set_texture(const Ref<Texture>& texture);
    void set_image(const Ref<Image>& image);
    
    bool is_null() const { return image.is_null(); }
    bool is_valid() const { return image.is_valid(); }
    
    bool is_square() const { return width == height; }

    void compute_min_max(float& min, float& max);
    void gaussian_blur(int size, float sigma);
     
    // width minus 1
    uint32_t width_m1;
    uint32_t height_m1;
    
    uint32_t width;
    uint32_t height;
    Ref<Texture> texture;
    mutable Ref<Image> image;
    
    BTextureIOTool();
    ~BTextureIOTool();      
        
};

#endif // BTEXTURE_IO_TOOL_H
