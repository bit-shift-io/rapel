
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
#include "bconfig.h"
#include "core/project_settings.h"
#include <assert.h>

BConfig *BConfig::singleton=NULL;

Ref<ConfigFile> BConfig::get_project_settings() {
    return project_settings;
}

void BConfig::save() {
    if (dirty) {
        settings->save(settings_file_path);
    }

    dirty = false;
}

Variant BConfig::get_setting(String p_key, Variant p_default_value) {
    // 1. check for local override (in future: commandline and environment vars)
    if (_opts.has(p_key)) {
        return _opts[p_key];
    }

    // 2. check settings file
    int first_slash = p_key.find("/");
    String section = first_slash != -1 ? p_key.substr(0, first_slash) : "";

    String key = first_slash != -1 ? p_key.substr(first_slash + 1, p_key.length()) : p_key;
    if (settings->has_section_key(section, key))
        return settings->get_value(section, key);

    // 3. check globals
    if (ProjectSettings::get_singleton()->has_setting(p_key))
        return ProjectSettings::get_singleton()->get(p_key);

    // failure on every level!
    return p_default_value;
}

void BConfig::set_setting(String p_key, Variant p_value) {
    int first_slash = p_key.find("/");
    String section = first_slash != -1 ? p_key.substr(0, first_slash) : "";
    String key = first_slash != -1 ? p_key.substr(first_slash + 1, p_key.length()) : p_key;
    settings->set_value(section, key, p_value);
    dirty = true;
}

bool BConfig::has_setting(String p_key) {
    // 1. check for local override (in future: commandline and environment vars)
    if (_opts.has(p_key))
        return true;

    // 2. check settings file
    int first_slash = p_key.find("/");
    String section = first_slash != -1 ? p_key.substr(0, first_slash) : "";

    String key = first_slash != -1 ? p_key.substr(first_slash + 1, p_key.length()) : p_key;
    if (settings->has_section_key(section, key))
        return true;

    // 3. check globals
    if (ProjectSettings::get_singleton()->has_setting(p_key))
        return true;

    // failure on every level!
    return false;
}

bool BConfig::is_build_type(String type) {
    /*
    const BT_DEMO = "demo";
    const BT_FULL = "full";
    const BT_DEVELOPER = "developer";
    */

#ifdef DEMO_BUILD
    return "demo" == type;
#else
    return get_setting("application/build_type", "demo") == type;
#endif
}

void BConfig::_bind_methods() {
    ClassDB::bind_method(D_METHOD("save"),&BConfig::save);
    ClassDB::bind_method(D_METHOD("get_setting", "key", "default_value"),&BConfig::get_setting, DEFVAL(Variant()));
    ClassDB::bind_method(D_METHOD("set_setting"),&BConfig::set_setting);
    ClassDB::bind_method(D_METHOD("has_setting"),&BConfig::has_setting);
    ClassDB::bind_method(D_METHOD("is_build_type"),&BConfig::is_build_type);
    ClassDB::bind_method(D_METHOD("get_project_settings"),&BConfig::get_project_settings);
}

BConfig::BConfig() {
    singleton = this;
    dirty = false;

    settings_file_path = GLOBAL_DEF("bitshift/config/settings_file_path", "res://settings.cfg");
    Ref<ConfigFile> cf = memnew(ConfigFile);
    settings = cf;
    Error err = settings->load(settings_file_path);
    if (err != OK) {
        print_line("Error loading settings: " + settings_file_path);
    }

    String project_settings_file_path = "res://project.godot";
    Ref<ConfigFile> cf2 = memnew(ConfigFile);
    project_settings = cf2;
    err = project_settings->load(project_settings_file_path);
    if (err != OK) {
        err = project_settings->load("res://project.binary");
        if (err != OK) {
            print_line("Error loading settings: res://project.godot or res://project.binary");
        }
    }
}

BConfig::~BConfig() {
    save();
}
