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

#include "beditor_generate_normal_map.h"
#include "scene/gui/rich_text_label.h"
#include "../butil.h"
#include "../bnormal_map.h"
#include "../bheight_map.h"
#include "core_string_names.h"

#include "editor/editor_node.h"
#include "editor/property_editor.h"

class NormalMapSettings : public Reference {
    GDCLASS(NormalMapSettings, Reference);

    bool _set(const StringName &p_name, const Variant &p_value);
    bool _get(const StringName &p_name, Variant &r_ret) const;
    void _get_property_list(List<PropertyInfo> *p_list) const;

public:
    Dictionary d;

    NormalMapSettings();
};

bool NormalMapSettings::_set(const StringName &p_name, const Variant &p_value) {
    d[p_name] = p_value;

    if (p_name == "height_map_path" && d["normal_map_path"] == "") {
        String normal_map_path = p_value;
        normal_map_path = normal_map_path.replace("-Height", "-Normal");
        normal_map_path = normal_map_path.replace(".tif", ".png");
        _set("normal_map_path", normal_map_path);
        _change_notify();
        emit_signal(CoreStringNames::get_singleton()->changed);
    }
    return true;
}

bool NormalMapSettings::_get(const StringName &p_name, Variant &r_ret) const {
    r_ret = d[p_name];
    return true;
}

void NormalMapSettings::_get_property_list(List<PropertyInfo> *p_list) const {
    p_list->push_back(PropertyInfo(Variant::STRING,"height_map_path", PROPERTY_HINT_FILE, "tif,png"));
    p_list->push_back(PropertyInfo(Variant::STRING,"normal_map_path", PROPERTY_HINT_FILE, "tif,png"));
    p_list->push_back(PropertyInfo(Variant::REAL, "height"));
    p_list->push_back(PropertyInfo(Variant::REAL, "blur_size"));
    p_list->push_back(PropertyInfo(Variant::REAL, "blur_sigma"));
}

NormalMapSettings::NormalMapSettings() {
    _set("blur_size", 3);
    _set("blur_sigma", 3);
    _set("height", 100);
    _set("height_map_path", "");
    _set("normal_map_path", "");
}

//////////////////

void BEditorGenerateNormalMap::popup() {
    popup_centered(Size2(500, 500) * EDSCALE);
    property_editor->edit(normal_map_settings);
}

void BEditorGenerateNormalMap::ok_pressed() {
    EditorNode::progress_add_task("normal_map", "Generating", 1);

    // for now, just bust it through the existing python script
    String height_map_path = ProjectSettings::get_singleton()->globalize_path(String(normal_map_settings->d["height_map_path"]));
    String normal_map_path = ProjectSettings::get_singleton()->globalize_path(String(normal_map_settings->d["normal_map_path"]));

    float blur_size = normal_map_settings->d["blur_size"];
    float blur_sigma = normal_map_settings->d["blur_sigma"];
    float height = normal_map_settings->d["height"];

    if (!FileAccess::exists(height_map_path)) {
        show_message("ERROR: height_map_path no valid: '" + height_map_path + "'");
        EditorNode::progress_end_task("normal_map");
        return;
    }

    BHeightMap height_map;
    height_map.set_height(height);
    Error error = height_map.load(height_map_path);

    if (error != OK || height_map.get_height_map_tool().is_null()) {
        show_message("ERROR: Heightmap is null, cant generate normal map from '" + height_map_path + "'");
        EditorNode::progress_end_task("normal_map");
        return;
    }

    BNormalMap normalmap;
    error = normalmap.generate_and_save_from_height_map(normal_map_path, height_map, blur_size, blur_sigma);
    if (error != OK) {
        show_message("ERROR: generate_and_save_from_height_map_image failed");
        EditorNode::progress_end_task("normal_map");
        return;
    }

    show_message("Success!");
    EditorNode::progress_end_task("normal_map");
}

void BEditorGenerateNormalMap::show_message(const String& msg) {
    BUtil::get_singleton()->log_editor_message(msg);
    results_label->set_text(msg);
    results->popup_centered(Size2(600, 300) * EDSCALE);
}

void BEditorGenerateNormalMap::_results_confirm() {

}

void BEditorGenerateNormalMap::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_results_confirm"), &BEditorGenerateNormalMap::_results_confirm);
}

BEditorGenerateNormalMap::BEditorGenerateNormalMap() {
    normal_map_settings = memnew(NormalMapSettings);

    VBoxContainer *vbc = memnew(VBoxContainer);
    add_child(vbc);

    property_editor = memnew(PropertyEditor);
    vbc->add_child(property_editor);

    property_editor->set_name(TTR("Options"));
    property_editor->hide_top_label();
    property_editor->set_v_size_flags(SIZE_EXPAND_FILL);

    get_ok()->set_text(TTR("Generate"));
    set_title(TTR("Generate Normal Map"));
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
}

BEditorGenerateNormalMap::~BEditorGenerateNormalMap() {
    memdelete(normal_map_settings);
}
