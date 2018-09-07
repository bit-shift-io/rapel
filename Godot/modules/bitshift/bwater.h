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
#ifndef BWATER_H
#define BWATER_H

#include "scene/3d/spatial.h"
#include "self_list.h"
#include "func_ref.h"

class CanvasItemMaterial;
class BMeshGenTool;

/**
	@author Fabian Mathews <supagu@gmail.com>
*/

class BWater : public Spatial {

	GDCLASS(BWater,Spatial)
        
	static BWater *singleton;
 
    Ref<Material> surface_material;
    Ref<Material> skirt_material;

    float size;
    float surface_height; // world surface level
    float depth;
    int resolution;
    int chunks;

    bool dirty;
    bool inTree;

    void _update();
    void _dirty();

    void set_size(float size);
    float get_size() const;

    void set_surface_height(float height);
    float get_surface_height() const;

    void set_depth(float depth);
    float get_depth() const;

    void set_resolution(int resolution);
    int get_resolution() const;

    void set_chunks(int chunks);
    int get_chunks() const;

    Spatial* create_lod(const String& name, int near, int far, Ref<Mesh>& mesh);

    void add_skirt_verts_for_point(BMeshGenTool& mesh_gen, int x, int y, int resolution, const Transform& uvXform);
    Ref<Mesh> create_skirt(int resolution, const Transform& uvXform);

protected:

	static void _bind_methods();
    void _notification( int p_what);
    void _material_changed();
    void _changed_callback(Object *p_changed, const char *p_prop);
        
public:

    void set_regenerate(bool regen);
    bool get_regenerate() const;

    void set_surface_material(const Ref<Material>& material);
    Ref<Material> get_surface_material() const;

    void set_skirt_material(const Ref<Material>& material);
    Ref<Material> get_skirt_material() const;
        
	BWater();
	~BWater();            

    static BWater *get_singleton() { return singleton; }
};

#endif // BWATER_H
