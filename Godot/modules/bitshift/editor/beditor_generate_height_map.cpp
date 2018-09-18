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

#include "beditor_generate_height_map.h"
#include "scene/gui/rich_text_label.h"
#include "../butil.h"

#include "editor/editor_node.h"
#include "editor/property_editor.h"

class HeightMapSettings : public Reference {
    GDCLASS(HeightMapSettings, Reference);

    bool _set(const StringName &p_name, const Variant &p_value);
    bool _get(const StringName &p_name, Variant &r_ret) const;
    void _get_property_list(List<PropertyInfo> *p_list) const;

public:
    Dictionary d;

    HeightMapSettings();
};

bool HeightMapSettings::_set(const StringName &p_name, const Variant &p_value) {
    d[p_name] = p_value;
    return true;
}

bool HeightMapSettings::_get(const StringName &p_name, Variant &r_ret) const {
    r_ret = d[p_name];
    return true;
}

void HeightMapSettings::_get_property_list(List<PropertyInfo> *p_list) const {
    p_list->push_back(PropertyInfo(Variant::STRING,"input_texture_path", PROPERTY_HINT_FILE, "tif"));
    p_list->push_back(PropertyInfo(Variant::STRING,"output_texture_path", PROPERTY_HINT_FILE, "tif"));
    p_list->push_back(PropertyInfo(Variant::REAL, "blur", PROPERTY_HINT_RANGE, "0,128"));
    p_list->push_back(PropertyInfo(Variant::INT, "size", PROPERTY_HINT_RANGE, "0,8192"));
}

HeightMapSettings::HeightMapSettings() {
    _set("input_texture_path", "");
    _set("output_texture_path", "");
    _set("blur", 3);
    _set("size", 4096);
}

//////////////////

void BEditorGenerateHeightMap::popup() {
    popup_centered(Size2(500, 500) * EDSCALE);
#if 0 // TODO: FIXME
    property_editor->edit(height_map_settings);
#endif
}

void BEditorGenerateHeightMap::ok_pressed() {

    // for now, just bust it through the existing python script
    String input_file_name = ProjectSettings::get_singleton()->globalize_path(String(height_map_settings->d["input_texture_path"]));
    String output_file_name = ProjectSettings::get_singleton()->globalize_path(String(height_map_settings->d["output_texture_path"]));

    List<String> args;
    args.push_back("-i");
    args.push_back(input_file_name);
    args.push_back("-o");
    args.push_back(output_file_name);
    args.push_back("-b");
    args.push_back(String(height_map_settings->d["blur"]));
    args.push_back("-s");
    args.push_back(String(height_map_settings->d["size"]));

    String resourceDir = OS::get_singleton()->get_resource_dir();
    String scriptPath = resourceDir + "/Editor/generate_height_map.py";

    EditorNode::progress_add_task("height_map", "Generating", 1);

    String pipe;
    Error err = OS::get_singleton()->execute(scriptPath, args, true, NULL, &pipe);
    BUtil::get_singleton()->log_editor_message(pipe);

    EditorNode::progress_end_task("height_map");

    results_label->set_text(pipe);
    results->popup_centered(Size2(600, 300) * EDSCALE);
}

void BEditorGenerateHeightMap::_results_confirm() {

}

void BEditorGenerateHeightMap::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_results_confirm"), &BEditorGenerateHeightMap::_results_confirm);
}

BEditorGenerateHeightMap::BEditorGenerateHeightMap() {
#if 0 // TODO: FIXME
    height_map_settings = memnew(HeightMapSettings);

    VBoxContainer *vbc = memnew(VBoxContainer);
    add_child(vbc);

    property_editor = memnew(CustomPropertyEditor);
    vbc->add_child(property_editor);

    property_editor->set_name(TTR("Options"));
    property_editor->hide_top_label();
    property_editor->set_v_size_flags(SIZE_EXPAND_FILL);

    get_ok()->set_text(TTR("Generate"));
    set_title(TTR("Generate Height Map"));
    set_hide_on_ok(false);

    // popup results
    results = memnew(ConfirmationDialog);
    results->set_title(TTR("Results"));
    add_child(results);
    vbc = memnew(VBoxContainer);
    results->add_child(vbc);

    results_label = memnew(RichTextLabel);
    results_label->set_v_size_flags(SIZE_EXPAND_FILL);
    vbc->add_child(results_label);
    results->connect("confirmed", this, "_results_confirm");
#endif
}

BEditorGenerateHeightMap::~BEditorGenerateHeightMap() {
    memdelete(height_map_settings);
}
