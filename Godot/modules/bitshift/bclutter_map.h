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
#ifndef BCLUTTER_MAP_H
#define BCLUTTER_MAP_H

#include "scene/3d/spatial.h"
#include "btexture_io_tool.h"

class Mesh;

/**
	@author Fabian Mathews <supagu@gmail.com>
*/

class BClutterMap : public Spatial {

	GDCLASS(BClutterMap, Spatial)
         
	BTextureIOTool clutter_map;
	
	enum {
		MAX_PROP_GROUPS = 100
	};
	
	bool dirty;
    bool inTree;
	
	struct PropGroup {
		
		PropGroup() {
			jitter_radius = 0.f;
		}
		Color	colour;
		String	path;
		float	jitter_radius;
	};
	
	Vector<PropGroup> prop_groups;
	
	void _dirty();
    void _update();
	
	void _spawn_prop_group(Spatial* lod, const PropGroup& prop_group);
	
	void set_prop_group_colour(int p_group, const Color &colour);
	Color get_prop_group_colour(int p_group) const;
	
	void set_prop_group_path(int p_group, const String &p_path);
	String get_prop_group_path(int p_group) const;
	
	void set_prop_group_jitter_radius(int p_group, float p_radius);
	float get_prop_group_jitter_radius(int p_group) const;
	
	void set_prop_group_size(int p_count);
	int get_prop_group_count() const;
	
protected:

	static void _bind_methods();
	void _notification(int p_what);
	void _validate_property(PropertyInfo &property) const;
           
public:

	void set_clutter_map(const Ref<Texture>& clutter_map);
    Ref<Texture> get_clutter_map() const;
        
	BClutterMap();
	~BClutterMap();            
};

#endif // BCLUTTER_MAP_H
