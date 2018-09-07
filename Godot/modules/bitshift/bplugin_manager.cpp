 
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
#include "bplugin_manager.h"
#include "core/bind/core_bind.h"
#include "scene/resources/packed_scene.h"
#include "butil.h"
#include "bglobals.h"
#include <assert.h>

BPluginManager *BPluginManager::singleton=NULL;

Node* BPluginManager::_load_descriptor(const String& p_resource_path) {
    String plugin_path = p_resource_path + "/Plugin.tscn";

    _Directory dir;
    if (!dir.file_exists(plugin_path))
        return NULL;

    Ref<PackedScene> res = ResourceLoader::load(plugin_path, "PackedScene");
    if (res.is_null()) {
        print_line("Failed to load plugin: " + plugin_path + ", Couldn't load PackedScene");
        return NULL;
    }
    Node *descriptor = res->instance();

    // check the type is correct
    bool is_plugin = BUtil::get_singleton()->is_class_name(descriptor, "Plugin");
    if (!is_plugin) {
        print_line("Failed to load plugin: " + plugin_path + ", not of type Plugin");
        return NULL;
    }

    return descriptor;
}

bool BPluginManager::add_plugin(const String& p_resource_path) {
    // only allow the example mod in demo builds
    Node *descriptor = _load_descriptor(p_resource_path);
    if (!descriptor) {
        return false;
    }

    PluginInfo info;
    info.resource_path = p_resource_path;
    info.descriptor = descriptor;
    plugins[info.resource_path] = info;
    return true;
}

bool BPluginManager::scan_directory(const String& p_directory) {
    
#ifdef DEMO_BUILD
    String path;
    path = DEMO_MAP_1;
    add_plugin(path);
    path = DEMO_MAP_2;
    add_plugin(path);
    path = DEMO_MOD;
    add_plugin(path);
#else
    _Directory dir;
    if (dir.open(p_directory) != OK) {
        ERR_PRINTS("BPluginManager::scan_directory failed to open directory: " + p_directory);
        return false;
    }
    
    dir.list_dir_begin();
    String file_name = dir.get_next();
    while (file_name != "") {
        if (dir.current_is_dir()) {
            String resource_path = p_directory + "/" + file_name;
            add_plugin(resource_path);
        }
        
        file_name = dir.get_next();
    }
#endif

    return true;
}
    
Dictionary BPluginManager::get_plugins() {
    Dictionary results;
    for (Map<String, PluginInfo>::Element *E = plugins.front(); E; E = E->next()) {
        results[E->key()] = E->value().descriptor;
    }

    return results;
}

Node* BPluginManager::get_descriptor(const String& p_resource_path) {
    return plugins[p_resource_path].descriptor;
}

void BPluginManager::_bind_methods() {
    ClassDB::bind_method(D_METHOD("scan_directory"),&BPluginManager::scan_directory);
    ClassDB::bind_method(D_METHOD("get_plugins"),&BPluginManager::get_plugins);
    ClassDB::bind_method(D_METHOD("get_descriptor"),&BPluginManager::get_descriptor);
}

BPluginManager::BPluginManager() {
    singleton = this;
}

BPluginManager::~BPluginManager() {
    for (Map<String, PluginInfo>::Element *E = plugins.front(); E; E = E->next()) {
        memdelete( E->value().descriptor );
        E->value().descriptor = NULL;
    }
}
