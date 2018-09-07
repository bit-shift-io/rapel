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
#ifndef BUTIL_H
#define BUTIL_H

#include "core/image.h"
//#include "reference.h"
//#include "self_list.h"
//#include "func_ref.h"
#include "itch_io.h"


class BTerrain;
class BWater;
class ArrayMesh;
class CanvasItem;
class Spatial;
class TextureButton;
class BBoundaryMap;
class Timer;
class SceneTree;
class RichTextLabel;

/**
	@author Fabian Mathews <supagu@gmail.com>
*/

class BUtil : public Reference {

	GDCLASS(BUtil,Reference)

	ItchIo itch;

	static BUtil *singleton;

    Timer* demo_timer;
        
protected:

	static void _bind_methods();
        
        bool _is_class_name(Node* node, const String& class_name, const String& scriptClassName);
        void _find_children_by_class_name(Node* node, const String& class_name, bool recursive, const String& scriptClassName, Array& results);

        Node *_find_child(Node *p_node, const String &p_mask, bool p_recursive, Node* p_parent, int p_depth) const;

        void _on_demo_expire();
        void _add_demo_timer();
        
public:

    int get_rich_text_label_height(Control* p_rich_text);

    AABB get_aabb_from_points_2d(const Array& p_point_list);

	String get_resource_dir() const;
	String get_username();

    bool is_child_of(Node* possible_child, Node* possible_parent);
    bool is_class_name(Node* node, String class_name);
	Node* find_parent_by_class_name(Node* node, String class_name);
	Array find_children_by_class_name(Node* node, String class_name, bool recursive = true);
	Node* find_first_child_by_class_name(Node* node, String class_name, bool recursive = true);
	void delete_children(Node* node);
    void queue_delete_children(Node* node);
    void set_children_visible(Node* node, bool visible);

    Node *find_child(Node *p_node, const String &p_mask, bool p_recursive = true) const;

    Ref<ArrayMesh> create_tessellated_quad(int resolution);

    BTerrain *get_terrain();
    BWater *get_water();
    BBoundaryMap *get_boundary_map();

    bool set_click_mask_from_normal_alpha(Control* p_texture_button);
    void remove_from_parent(Node* node);

    void log_editor_message(const String& msg);

    SceneTree* get_scene_tree();

    String url_encode(const String& str);

    BUtil();
    ~BUtil();

    static BUtil *get_singleton() { return singleton; }

    static inline Vector3 to_vector3(const Color& colour) {
            return Vector3(colour.r, colour.g, colour.b);
    }

    static inline Color to_colour(const Vector3& vector) {
        return Color(vector.x, vector.y, vector.z);
    }
	
    static inline bool is_equal_approx(const Vector3& a, const Vector3& b) {
            return Math::is_equal_approx(a.x, b.x) && Math::is_equal_approx(a.y, b.y) && Math::is_equal_approx(a.z, b.z);
    }

    static inline bool is_equal_approx(const Color& a, const Color& b) {
        return Math::is_equal_approx(a.r, b.r) && Math::is_equal_approx(a.g, b.g) && Math::is_equal_approx(a.b, b.b) && Math::is_equal_approx(a.a, b.a);
    }
};

#endif // BUTIL_H
