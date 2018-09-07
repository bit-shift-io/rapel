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
#include "bboundary_map.h"
#include "bterrain.h"
#include "butil.h"
#include <assert.h>

BBoundaryMap *BBoundaryMap::singleton=NULL;

void BBoundaryMap::set_boundary_map(const Ref<Texture>& cm) {
    boundary_map.set_texture(cm);
}

Ref<Texture> BBoundaryMap::get_boundary_map() const {
    return boundary_map.texture;
}

void BBoundaryMap::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_boundary_map","texture"),&BBoundaryMap::set_boundary_map);
    ClassDB::bind_method(D_METHOD("get_boundary_map"),&BBoundaryMap::get_boundary_map);

    ADD_PROPERTYNZ( PropertyInfo(Variant::OBJECT,"boundary_map/boundary_map",PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_boundary_map", "get_boundary_map");

    ClassDB::bind_method(D_METHOD("is_in_bounds"),&BBoundaryMap::is_in_bounds);
    ClassDB::bind_method(D_METHOD("is_out_of_bounds"),&BBoundaryMap::is_out_of_bounds);
}

bool BBoundaryMap::is_in_bounds(const Vector3& position, int channel) const {
    BTerrain* terrain = BTerrain::get_singleton();
    ERR_FAIL_COND_V(!terrain, false);

    Vector2 texture_space = terrain->world_to_texture_space(position);

    float px = texture_space.x * boundary_map.width_m1;
    float py = texture_space.y * boundary_map.height_m1;

    if (!boundary_map.is_pixel_valid(px, py))
        return false;

    Vector3 c = boundary_map.get_pixel_as_vec3(px, py);
    return c[channel] != 0.0f;
}

bool BBoundaryMap::is_out_of_bounds(const Vector3& position, int channel) const {
    return !is_in_bounds(position, channel);
}

BBoundaryMap::BBoundaryMap() {
    singleton = this;
}

BBoundaryMap::~BBoundaryMap() {
    if (singleton == this) {
        singleton = NULL;
    }
}


