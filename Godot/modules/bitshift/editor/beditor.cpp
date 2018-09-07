
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
#include "beditor.h"
#include "beditor_generate_height_map.h"
#include "beditor_generate_normal_map.h"
#include "editor/editor_node.h"
#include <assert.h>

BEditor *BEditor::singleton=NULL;

void BEditor::_bind_methods() {
    ClassDB::bind_method("_menu_option", &BEditor::_menu_option);
}

void BEditor::_ready() {
    PopupMenu *p;

    Panel *gui_base = Object::cast_to<Panel>(EditorNode::get_singleton()->get_gui_base()); //Object::cast_to<Panel>(get_node(NodePath("/root/EditorNode/@@5/@@6")));

    HBoxContainer *left_menu_hb = EditorNode::get_singleton()->get_menu_hb(); //Object::cast_to<HBoxContainer>(get_node(NodePath("/root/EditorNode/@@5/@@6/@@14/@@15/@@53/@@52")));
    assert(left_menu_hb);

    bitshift_menu = memnew(MenuButton);
    bitshift_menu->set_flat(false);
    bitshift_menu->set_text(TTR("Bitshift"));
    bitshift_menu->add_style_override("hover", gui_base->get_stylebox("MenuHover", "EditorStyles"));
    left_menu_hb->add_child(bitshift_menu);

    p = bitshift_menu->get_popup();
    p->add_item(TTR("Generate Height Map"), BITSHIFT_GENERATE_HEIGHT_MAP);
    p->add_item(TTR("Generate Normal Map"), BITSHIFT_GENERATE_NORMAL_MAP);
    //p->add_separator();
    p->connect("id_pressed", this, "_menu_option");


    generate_height_map = memnew(BEditorGenerateHeightMap);
    generate_height_map->set_theme(generate_height_map->get_theme());
    gui_base->add_child(generate_height_map);

    generate_normal_map = memnew(BEditorGenerateNormalMap);
    generate_normal_map->set_theme(generate_normal_map->get_theme());
    gui_base->add_child(generate_normal_map);
}

void BEditor::_menu_option(int p_option) {

        //_menu_option_confirm(p_option, false);

    switch (p_option) {
    case BITSHIFT_GENERATE_HEIGHT_MAP:
        generate_height_map->popup();
        break;

    case BITSHIFT_GENERATE_NORMAL_MAP:
        generate_normal_map->popup();
        break;
    }
}

void BEditor::_notification(int p_what) {
    if (p_what == NOTIFICATION_READY) {
        _ready();
    }
}

static void register_editor_callback() {
/*
        ScriptEditor::register_create_script_editor_function(create_editor);

        ED_SHORTCUT("visual_script_editor/delete_selected", TTR("Delete Selected"));
        ED_SHORTCUT("visual_script_editor/toggle_breakpoint", TTR("Toggle Breakpoint"), KEY_F9);
        ED_SHORTCUT("visual_script_editor/find_node_type", TTR("Find Node Type"), KEY_MASK_CMD + KEY_F);
        ED_SHORTCUT("visual_script_editor/copy_nodes", TTR("Copy Nodes"), KEY_MASK_CMD + KEY_C);
        ED_SHORTCUT("visual_script_editor/cut_nodes", TTR("Cut Nodes"), KEY_MASK_CMD + KEY_X);
        ED_SHORTCUT("visual_script_editor/paste_nodes", TTR("Paste Nodes"), KEY_MASK_CMD + KEY_V);
        */

    EditorNode *editorNode = EditorNode::get_singleton();
    if (editorNode)
        editorNode->add_child(BEditor::get_singleton());
}

BEditor::BEditor() {
    singleton = this;
    EditorNode::add_plugin_init_callback(register_editor_callback);
}

BEditor::~BEditor() {
}
