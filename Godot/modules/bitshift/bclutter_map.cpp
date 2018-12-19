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
#include "bclutter_map.h"
#include "bterrain.h"
#include "butil.h"
#include "scene/resources/packed_scene.h"
#include "scene/3d/mesh_instance.h"
#include "core/engine.h"
#include <assert.h>


void BClutterMap::_notification(int p_what)
{
    switch (p_what)
    {
        case NOTIFICATION_READY: {
            _dirty();			
        } break;

        case NOTIFICATION_ENTER_TREE: {
            inTree = true;
            _dirty();
        } break;
        
        case NOTIFICATION_EXIT_TREE: {
            inTree = false;
        } break;
        
        case NOTIFICATION_ENTER_WORLD: {
            _dirty();
        } break;
        
        case NOTIFICATION_EXIT_WORLD: {
            BUtil::get_singleton()->delete_children(this);
        } break;
			
        default: {}
    }
}

void BClutterMap::_dirty() {
    if (!inTree) {
        return;
    }
    
    if (clutter_map.is_null()) {
        print_line("Please specify a clutter_map");
        return;
    }
    
    if (dirty) {
        return;
    }
    
    dirty = true;
    call_deferred("_update");
}

void BClutterMap::_update()
{
    if (!dirty) {
        return;
    }
    dirty = false;
	
	// disabled in editor as its just to slow for some reason
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
    
    BUtil::get_singleton()->delete_children(this);
	
	Spatial* lod = memnew(Spatial());
    lod->set_name("lod");
	for (int i = 0; i < prop_groups.size(); ++i) {
		_spawn_prop_group(lod, prop_groups[i]);
	}
	add_child(lod); // reparent
}

void BClutterMap::_spawn_prop_group(Spatial* lod, const PropGroup& prop_group) {
	Color c = prop_group.colour;
	//Color c_rgb(Math::round(c.r * 255.f), Math::round(c.g * 255.f), Math::round(c.b * 255.f));
		
	Ref<PackedScene> sd = ResourceLoader::load(prop_group.path);
	ERR_FAIL_COND(!sd.is_valid());
	
	BTerrain* terrain = BTerrain::get_singleton();
	if (!terrain) {
		print_line("Map contains no terrain, aborting clutter map");
		return;
	}
	
	// TODO: this should be replaced with something like terrain->get_texture_to_world_transform()
	// compute how to scale from x, y in clutter_map space to an x, y in height_map space
    float height_scale = float(terrain->height_map.get_height_map_tool().height) / float(clutter_map.height);
    float width_scale = float(terrain->height_map.get_height_map_tool().width) / float(clutter_map.width);
    float w_div_size = terrain->height_map.get_height_map_tool().width_m1 / terrain->size;
	float halfSize = terrain->size / 2.0;
	Transform xform = terrain->get_global_transform();
	
	for (uint32_t y = 0; y < clutter_map.height; ++y) {
		for (uint32_t x = 0; x < clutter_map.width; ++x) {
			Color pixel_colour = clutter_map.get_pixel(x, y);
			
			if (pixel_colour.g != 0)
			{
                //print_line("Pixel color: (" + itos(int(pixel_colour.r * 255)) + "," + itos(int(pixel_colour.g * 255)) + "," + itos(int(pixel_colour.b * 255)) + ")");
				int nothing = 0;
				++nothing;
			}
			
			//
			// ENSURE that "premult alpha" is disabled in the clutter_map texture!!!!
			// else this code will fail
			//
			// see https://github.com/godotengine/godot/issues/8344 for more info
			//
            if (BUtil::is_equal_approx(pixel_colour, c)) {
				Spatial* prop = Object::cast_to<Spatial>(sd->instance()); //sd->instance()->cast_to<Spatial>(); // GeometryInstance
				ERR_FAIL_COND(!prop);
				
				float h_x = x * width_scale;
				float h_y = y * height_scale;
                float height = terrain->height_map.get_height_from_pixel(h_x, h_y); //.get_pixel_as_vec3(h_x, h_y).x * terrain->height;
				
				// convert from heightmap texture space to local space
				Vector3 localPos = Vector3((h_x / w_div_size) - halfSize, height, (h_y / w_div_size) - halfSize);

				// local to world
				Vector3 worldPos = xform.xform(localPos);
				
				// apply jitter - random rotation of the prop, and in a random part of the circle
				Transform jitter;
				float rotation = Math::random(-Math_PI, Math_PI);
				float radius = Math::random(0.f, prop_group.jitter_radius);
				Vector3 offset(radius * Math::cos(rotation), 0, radius * Math::sin(rotation));
				//jitter = jitter.rotated(Vector3(0, 1, 0), Math::random(-Math_PI, Math_PI)); // rotate around up vector a random amount
			
				//float scale = Math::random(prop_group.scale_min, prop_group.scale_max);
				//jitter = jitter.scaled(Vector3(scale, scale, scale));
				
				Transform p_xform;
				p_xform.origin = worldPos + offset;
				p_xform.basis = jitter.basis;
				prop->set_transform(p_xform);
				lod->add_child(prop);
			}
		}
	}
}

void BClutterMap::set_clutter_map(const Ref<Texture>& cm) {
	clutter_map.set_texture(cm);
}

Ref<Texture> BClutterMap::get_clutter_map() const {
	return clutter_map.texture;
}

void BClutterMap::set_prop_group_colour(int p_group, const Color &colour) {
	ERR_FAIL_INDEX(p_group, prop_groups.size());
    prop_groups.write[p_group].colour = colour;
}

Color BClutterMap::get_prop_group_colour(int p_group) const {
	ERR_FAIL_INDEX_V(p_group, prop_groups.size(), Color());
    return prop_groups[p_group].colour;
}

void BClutterMap::set_prop_group_path(int p_group, const String &p_path) {
	ERR_FAIL_INDEX(p_group, prop_groups.size());
    prop_groups.write[p_group].path = p_path;
}

String BClutterMap::get_prop_group_path(int p_group) const {
	ERR_FAIL_INDEX_V(p_group, prop_groups.size(), String());
    return prop_groups[p_group].path;
}

void BClutterMap::set_prop_group_jitter_radius(int p_group, float p_radius) {
	ERR_FAIL_INDEX(p_group, prop_groups.size());
    prop_groups.write[p_group].jitter_radius = p_radius;
}

float BClutterMap::get_prop_group_jitter_radius(int p_group) const {
	ERR_FAIL_INDEX_V(p_group, prop_groups.size(), 0.f);
	return prop_groups[p_group].jitter_radius;
}

void BClutterMap::set_prop_group_size(int p_count) {
	ERR_FAIL_COND(p_count < 1);
	prop_groups.resize(p_count);
	_change_notify();
}

int BClutterMap::get_prop_group_count() const {
	return prop_groups.size();
}

void BClutterMap::_validate_property(PropertyInfo &property) const {

	if (property.name.begins_with("prop_group_") && !property.name.begins_with("prop_group_size")) {
		int slice_count = property.name.get_slice_count("_");
		int index = property.name.get_slicec('_', slice_count - 1).to_int() - 1;
		if (index >= prop_groups.size()) {
			property.usage = 0;
			return;
		}
	}
}

void BClutterMap::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_update"),&BClutterMap::_update);
    
    ClassDB::bind_method(D_METHOD("set_clutter_map","texture"),&BClutterMap::set_clutter_map);
    ClassDB::bind_method(D_METHOD("get_clutter_map"),&BClutterMap::get_clutter_map);
    ADD_PROPERTY( PropertyInfo(Variant::OBJECT,"clutter_map/clutter_map",PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_clutter_map", "get_clutter_map");
	
	ClassDB::bind_method(D_METHOD("set_prop_group_size", "size"), &BClutterMap::set_prop_group_size);
	ClassDB::bind_method(D_METHOD("get_prop_group_count"), &BClutterMap::get_prop_group_count);

	ClassDB::bind_method(D_METHOD("set_prop_group_colour", "group", "colour"), &BClutterMap::set_prop_group_colour);
	ClassDB::bind_method(D_METHOD("get_prop_group_colour", "group"), &BClutterMap::get_prop_group_colour);
	
	ClassDB::bind_method(D_METHOD("set_prop_group_path", "group", "path"), &BClutterMap::set_prop_group_path);
	ClassDB::bind_method(D_METHOD("get_prop_group_path", "group"), &BClutterMap::get_prop_group_path);
	
	ClassDB::bind_method(D_METHOD("set_prop_group_jitter_radius", "group", "radius"), &BClutterMap::set_prop_group_jitter_radius);
	ClassDB::bind_method(D_METHOD("get_prop_group_jitter_radius", "group"), &BClutterMap::get_prop_group_jitter_radius);
		
	ADD_GROUP("Prop Group", "prop_group_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "prop_group_size", PROPERTY_HINT_RANGE, "0," + itos(MAX_PROP_GROUPS) + ",1"), "set_prop_group_size", "get_prop_group_count");
	for (int i = 0; i < MAX_PROP_GROUPS; i++) {
		ADD_PROPERTYI(PropertyInfo(Variant::STRING, "prop_group_path_" + itos(i + 1), PROPERTY_HINT_FILE, "*.dae,*.tscn"), "set_prop_group_path", "get_prop_group_path", i);
		ADD_PROPERTYI(PropertyInfo(Variant::COLOR, "prop_group_colour_" + itos(i + 1)), "set_prop_group_colour", "get_prop_group_colour", i);
		ADD_PROPERTYI(PropertyInfo(Variant::REAL, "prop_group_jitter_radius_" + itos(i + 1)), "set_prop_group_jitter_radius", "get_prop_group_jitter_radius", i);
	}
	
	BIND_CONSTANT(MAX_PROP_GROUPS);
}

BClutterMap::BClutterMap() { 
    dirty = false;
    inTree = false;
	
	set_prop_group_size(1);
}

BClutterMap::~BClutterMap() {
}


