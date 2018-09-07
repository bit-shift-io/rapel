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
#ifndef BNORMAL_MAP_H
#define BNORMAL_MAP_H

#include "scene/3d/spatial.h"
#include "btexture_io_tool.h"
#include "bheight_map.h"

/**
    @author Fabian Mathews <supagu@gmail.com>

    Generate a normalmap from a heightmap
*/

class BNormalMap : public Spatial {

    GDCLASS(BNormalMap, Spatial)

    struct WeightedStep {
        int step;
        float weight;

        WeightedStep() {
        }

        WeightedStep(int p_step, float p_weight) {
            step = p_step;
            weight = p_weight;
        }
    };

    BTextureIOTool normal_map;

    void _sobel_generate_from_height_map(const BHeightMap& height_map, Vector<WeightedStep>& weightedStepList);
    Vector3 _sobel_get_normal_from_height_map(const BHeightMap& height_map, uint32_t x, uint32_t y, int step);

    void _fabian_generate_from_height_map(const BHeightMap& height_map, Vector<WeightedStep>& weightedStepList);
    Vector3 _fabian_get_normal_from_height_map(const BHeightMap& height_map, uint32_t x, uint32_t y, int step);

protected:

    static void _bind_methods();
           
public:

    bool is_valid() const;

    void set_normal_map(const Ref<Texture>& normal_map);
    Ref<Texture> get_normal_map() const;

    Error generate_and_save_from_height_map(const String& path, const BHeightMap& height_map, float blur_size, float blur_sigma);

    Error generate_from_height_map(const BHeightMap& height_map);

    // save as png or exr extension
    Error save(const String& resource_path);
    Error load(const String& path);

    Vector3 get_normal_from_texel(Vector2 texel_coord) const;
    //Vector3 get_normal_from_pixel(int x, int y) const;

    BNormalMap();
    ~BNormalMap();
};

#endif // BNORMAL_MAP_H
