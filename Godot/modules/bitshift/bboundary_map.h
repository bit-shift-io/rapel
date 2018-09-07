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
#ifndef BBOUNDARY_MAP_H
#define BBOUNDARY_MAP_H

#include "scene/3d/spatial.h"
#include "btexture_io_tool.h"

class Mesh;

/**
	@author Fabian Mathews <supagu@gmail.com>
*/

class BBoundaryMap : public Spatial {

    GDCLASS(BBoundaryMap, Spatial)

    static BBoundaryMap *singleton;

    BTextureIOTool boundary_map;

protected:

    static void _bind_methods();
           
public:

    void set_boundary_map(const Ref<Texture>& boundary_map);
    Ref<Texture> get_boundary_map() const;

    bool is_in_bounds(const Vector3& position, int channel) const;
    bool is_out_of_bounds(const Vector3& position, int channel) const;

    BBoundaryMap();
    ~BBoundaryMap();

    static BBoundaryMap *get_singleton() { return singleton; }
};

#endif // BBOUNDARY_MAP_H
