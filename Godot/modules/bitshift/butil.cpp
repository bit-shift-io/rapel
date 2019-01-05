 
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
#include "bglobals.h"
#include "butil.h"
#include "bterrain.h"
#include "bwater.h"
#include "bboundary_map.h"
#include "core/io/http_client.h"
#include "core/bind/core_bind.h"
#include "core/ustring.h"
#include "scene/2d/canvas_item.h"
#include "scene/main/node.h"
#include "scene/main/timer.h"
#include "scene/main/viewport.h"
#include "scene/resources/surface_tool.h"
#include "scene/3d/collision_polygon.h"
#include "scene/gui/texture_button.h"
#include "editor/editor_node.h"
#include <assert.h>

BUtil *BUtil::singleton=NULL;

void BUtil::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_aabb_from_points_2d"),&BUtil::get_aabb_from_points_2d);
    ClassDB::bind_method(D_METHOD("is_child_of"),&BUtil::is_child_of);
    ClassDB::bind_method(D_METHOD("is_class_name"),&BUtil::is_class_name);
    ClassDB::bind_method(D_METHOD("find_parent_by_class_name"),&BUtil::find_parent_by_class_name);
    ClassDB::bind_method(D_METHOD("find_children_by_class_name"),&BUtil::find_children_by_class_name, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("find_first_child_by_class_name"),&BUtil::find_first_child_by_class_name, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("queue_delete_children"),&BUtil::queue_delete_children);
    ClassDB::bind_method(D_METHOD("delete_children"),&BUtil::delete_children);
    ClassDB::bind_method(D_METHOD("set_children_visible"),&BUtil::set_children_visible);
    ClassDB::bind_method(D_METHOD("find_child"),&BUtil::find_child, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("get_username"),&BUtil::get_username);
    ClassDB::bind_method(D_METHOD("get_terrain"),&BUtil::get_terrain);
    ClassDB::bind_method(D_METHOD("get_water"),&BUtil::get_water);
    ClassDB::bind_method(D_METHOD("get_boundary_map"),&BUtil::get_boundary_map);
    ClassDB::bind_method(D_METHOD("create_tessellated_quad"),&BUtil::create_tessellated_quad);
    ClassDB::bind_method(D_METHOD("get_resource_dir"),&BUtil::get_resource_dir);
    ClassDB::bind_method(D_METHOD("set_click_mask_from_normal_alpha"),&BUtil::set_click_mask_from_normal_alpha);
    ClassDB::bind_method(D_METHOD("remove_from_parent"),&BUtil::remove_from_parent);
    ClassDB::bind_method(D_METHOD("log_editor_message"),&BUtil::log_editor_message);
    ClassDB::bind_method(D_METHOD("get_rich_text_label_height"),&BUtil::get_rich_text_label_height);

    ClassDB::bind_method(D_METHOD("_add_demo_timer"),&BUtil::_add_demo_timer);
    ClassDB::bind_method(D_METHOD("_on_demo_expire"),&BUtil::_on_demo_expire);
    ClassDB::bind_method(D_METHOD("get_scene_tree"),&BUtil::get_scene_tree);
}

int BUtil::get_rich_text_label_height(Control* p_rich_text) {
    RichTextLabel* rich_text = Object::cast_to<RichTextLabel>(p_rich_text);
    //message_label.set_custom_minimum_size(Vector2(message_label.rect_min_size.x, 10));
    //message_label.set_size(Vector2(message_label.rect_min_size.x, 10));
    Size2 size = rich_text->get_size();
    rich_text->set_size(Size2(size.width, 0));
    rich_text->scroll_to_line(0); // force vscroll to be updated
    int height = rich_text->get_v_scroll()->get_max();
    return height;
}

void BUtil::log_editor_message(const String& msg) {
#ifdef TOOLS_ENABLED
    if (EditorNode::get_singleton() && EditorNode::get_log())
        EditorNode::get_log()->add_message(msg);
#endif
}

void BUtil::remove_from_parent(Node* node) {
    if (node && node->get_parent())
        node->get_parent()->remove_child(node);
}

bool BUtil::set_click_mask_from_normal_alpha(Control* p_texture_button) {
    TextureButton* texture_button = Object::cast_to<TextureButton>(p_texture_button);
    Ref<Texture> texture = texture_button->get_normal_texture();
    String res_path = texture->get_path();

    Ref<Image> image = texture->get_data();
    Ref<BitMap> mask;
    mask.instance();
    mask->create_from_image_alpha(image);
    texture_button->set_click_mask(mask);
    return true;
}

AABB BUtil::get_aabb_from_points_2d(const Array& p_point_list) {
    AABB aabb;
    if (p_point_list.size() <= 0)
        return aabb;

    Vector2 pt = p_point_list[0];
    aabb.position = Vector3(pt.x, pt.y, 0.f);
    for (int i = 1; i < p_point_list.size(); ++i) {
        Vector2 pt = p_point_list[i];
        aabb.expand_to(Vector3(pt.x, pt.y, 0.f));
    }
    return aabb;
}

String BUtil::get_resource_dir() const {
	return OS::get_singleton()->get_resource_dir();
}

String BUtil::get_username()
{
    return itch.get_username();
}

bool BUtil::is_child_of(Node* possible_child, Node* possible_parent) {
    Node* node = possible_child;
    while (node) {
        if (node == possible_parent)
            return true;

        node = node->get_parent();
    }

    return false;
}

bool BUtil::_is_class_name(Node* node, const String& class_name, const String& scriptClassName) {
    if (!node)
        return false;

    Ref<Script> script = node->get_script();
    while (script.is_valid()) {
        String scriptName = script->get_path().get_file();
        if (scriptName == scriptClassName)
            return true;

        script = script->get_base_script();
    }

    if (ClassDB::is_parent_class(node->get_class(), class_name))
        return true;

    return false;
}

bool BUtil::is_class_name(Node* node, String class_name) {
    return _is_class_name(node, class_name, class_name + ".gd");
}

Node* BUtil::find_parent_by_class_name(Node* node, String class_name) {
    if (!node)
        return NULL;

    Node* n = node;
    String scriptClassName = class_name + ".gd";
    while (n && !(n->get_name() == (const char*)"root")) {
        if (_is_class_name(n, class_name, scriptClassName)) {
            return n;
        }

        n = n->get_parent();
    }

    return NULL;
}

Array BUtil::find_children_by_class_name(Node* node, String class_name, bool recursive) {
    if (!node)
        return Array();

    Node* n = node;
    Array results;
    for (int i = 0; i < n->get_child_count(); ++i)
    {
        _find_children_by_class_name(n->get_child(i), class_name, recursive, class_name + ".gd", results);
    }
    return results;
}

Node* BUtil::find_first_child_by_class_name(Node* node, String class_name, bool recursive) {
	Array children = find_children_by_class_name(node, class_name, recursive);
	if (children.size()) 
		return children[0];
	
	return NULL;
}

void BUtil::_find_children_by_class_name(Node* node, const String& class_name, bool recursive, const String& scriptClassName, Array& results) {
    if (!node)
        return;

    if (_is_class_name(node, class_name, scriptClassName)) {
        results.append(Variant(node));
    }

    if (!recursive)
        return;
    
    for (int i = 0; i < node->get_child_count(); ++i)
    {
        _find_children_by_class_name(node->get_child(i), class_name, recursive, scriptClassName, results);
    }
}

BTerrain *BUtil::get_terrain() {
    return BTerrain::get_singleton();
}

BWater *BUtil::get_water() {
    return BWater::get_singleton();
}

BBoundaryMap *BUtil::get_boundary_map() {
    return BBoundaryMap::get_singleton();
}

void BUtil::queue_delete_children(Node* node) {
    while( node->get_child_count() ) {
            Node *child = node->get_child(0);
            child->queue_delete();
    }
}

void BUtil::delete_children(Node* node) {
    // kill children as cleanly as possible
    while( node->get_child_count() ) {
            Node *child = node->get_child(0);
            node->remove_child(child);
            memdelete( child );
    }
}

void BUtil::set_children_visible(Node* node, bool visible) {
    for (int i = 0; i < node->get_child_count(); ++i) {
        Node* n = node->get_child(i);
        CanvasItem *child = Object::cast_to<CanvasItem>(n); //n->cast_to<CanvasItem>();
        assert(child);
        if (child)
            child->set_visible(visible);
    }
}

Node *BUtil::_find_child(Node *p_node, const String &p_mask, bool p_recursive, Node* p_parent, int p_depth) const {
    Node *n = p_node;
    String node_path_to_parent = n->get_name();
    n = p_node->get_parent();
    do {
        node_path_to_parent = n->get_name().operator String() + "/" + node_path_to_parent;
        n = p_node->get_parent();
    } while (n && n != p_parent);

    if (node_path_to_parent.match(p_mask))
        return p_node;

    if (p_recursive == false)
        return NULL;

    for (int i = 0; i < p_node->get_child_count(); ++i) {
        Node* n = p_node->get_child(i);
        Node *r = _find_child(n, p_mask, p_recursive, p_node, p_depth + 1);
        if (r)
            return r;
    }

    return NULL;
}

// The problem with Node::find_node is:
// 1. sometimes it works, sometimes it doesn't
// 2. it cant take a string like: NodeName/*/AnotherNodeName where * can be some arbitary number of nodes in between
Node *BUtil::find_child(Node *p_node, const String &p_mask, bool p_recursive) const {
    if (!p_node)
        return NULL;

    String modified_mask;

    // user is doing some manualling it if they have a /
    if (p_mask.find("/") == -1)
        modified_mask = "*/" + p_mask;
    else
        modified_mask = p_mask;

    for (int i = 0; i < p_node->get_child_count(); ++i) {
        Node* n = p_node->get_child(i);
        Node *r = _find_child(n, modified_mask, p_recursive, p_node, 0);
        if (r)
            return r;
    }

    return NULL;
    /*
    List<Node*> stack;
    stack.push_back(p_node);

    while (stack.size()) {
        Node* node = stack.pop_front();

        String node_path_to_parent;
        Node* p = node;
        do {
            node_path_to_parent = p->get_name() + node_path_to_parent;
        } while (p != p_node);

        if (node_path_to_parent.operator String().match(p_mask))
            return node;

        for () {

        }
    }
*/

    /*
     *
Node *Node::find_node(const String &p_mask, bool p_recursive, bool p_owned) const {

    Node *const *cptr = data.children.ptr();
    int ccount = data.children.size();
    for (int i = 0; i < ccount; i++) {
        if (p_owned && !cptr[i]->data.owner)
            continue;
        if (cptr[i]->data.name.operator String().match(p_mask))
            return cptr[i];

        if (!p_recursive)
            continue;

        Node *ret = cptr[i]->find_node(p_mask, true, p_owned);
        if (ret)
            return ret;
    }
    return NULL;
}
    */
}

Ref<ArrayMesh> BUtil::create_tessellated_quad(int resolution) {       
    float size = 1.0;
    Vector3 origin(-size/2.0, 0.0, -size/2.0);
    float resolution_step = size / resolution;
    float uv_step = 1.0 / resolution;
    
    int pointsWidth = (resolution + 1);
    int pointCount = (resolution + 1) * (resolution + 1);
    int triCount = resolution * resolution * 2;
    int indexCount = triCount * 3;
    
   
    PoolVector<Vector3> points;
    points.resize(pointCount);
    PoolVector<Vector3>::Write pointsw = points.write();
        
    PoolVector<Vector3> normals;
    normals.resize(pointCount);
    PoolVector<Vector3>::Write normalsw = normals.write();
        
    PoolVector<Vector2> uvs;
    uvs.resize(pointCount);
    PoolVector<Vector2>::Write uvsw = uvs.write();

    PoolVector<int> indices;
    indices.resize(indexCount);
    PoolVector<int>::Write indicesw = indices.write();
    
    // generate verts
    for (int y = 0; y < pointsWidth; ++y)
    {
        for (int x = 0; x < pointsWidth; ++x)
        {
            int pi = (y * pointsWidth) + x;
            
            pointsw[pi] = Vector3(x * resolution_step, 0.0, y * resolution_step) + origin;
            normalsw[pi] = Vector3(0, 1, 0);
            uvsw[pi] = Vector2(x * uv_step, y * uv_step);
        }
    }
    
    // generate indicies
    int q = 0; // what quad are we generating?
    int qi = 0; // quad index
    for (int i = 0; i < indexCount; i += 6)
    {
        indicesw[i + 0] = qi;
        indicesw[i + 1] = qi + 1;
        indicesw[i + 2] = qi + pointsWidth + 1;
        
        indicesw[i + 3] = qi;
        indicesw[i + 4] = qi + pointsWidth + 1;
        indicesw[i + 5] = qi + pointsWidth;
        
        ++qi;
        ++q;
        
        // skip the extra vert at the end of the row as we jump back to the start
        if (q != 0 && (q % resolution) == 0) {
            ++qi;
        }
    }
        
    Array arr;
    arr.resize(VS::ARRAY_MAX);
    arr[VS::ARRAY_VERTEX]=points;
    arr[VS::ARRAY_NORMAL]=normals;
    arr[VS::ARRAY_TEX_UV]=uvs;
    arr[VS::ARRAY_INDEX]=indices;
            
    Ref<ArrayMesh> mesh = memnew( ArrayMesh() );
    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arr);
    return mesh;
}

void BUtil::_add_demo_timer() {
    // add demo timer
    demo_timer = memnew(Timer);
    demo_timer->set_wait_time(DEMO_TIME);
    demo_timer->start();
    demo_timer->connect("timeout", this, "_on_demo_expire");

    get_scene_tree()->get_root()->add_child(demo_timer);
}

void BUtil::_on_demo_expire() {
    get_scene_tree()->quit();
}

SceneTree* BUtil::get_scene_tree() {
    MainLoop *ml = OS::get_singleton()->get_main_loop();
    SceneTree *sml = Object::cast_to<SceneTree>(ml);
    return sml;
}

String BUtil::url_encode(const String& str) {
    String r = str;
    r = r.replace("%", "%25");
    r = r.replace(" ", "%20");
    r = r.replace("&", "%26");
    r = r.replace(";", "%3B");
    r = r.replace("\"", "%22");
    r = r.replace("'", "%27");
    return r;
}

BUtil::BUtil() {
    singleton = this;
    demo_timer = NULL;
    //itch.auth();

#ifdef DEMO_BUILD
    print_line("www.bitshift.io - Demo Build");
    call_deferred("_add_demo_timer");
    #pragma message "COMPILING WITH DEMO_BUILD ENABLED"
#else
    print_line("www.bitshift.io");
#endif
}


BUtil::~BUtil() {

}
