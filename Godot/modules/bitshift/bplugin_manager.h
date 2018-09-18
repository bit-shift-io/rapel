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
#ifndef BPLUGIN_MANAGER_H
#define BPLUGIN_MANAGER_H

#include "core/reference.h"

/**
	@author Fabian Mathews <supagu@gmail.com>
*/

class BPluginManager : public Reference {

    GDCLASS(BPluginManager, Reference)

    static BPluginManager *singleton;
    
    struct PluginInfo {
        String resource_path;
        Node *descriptor;
    };
    
    Map<String, PluginInfo> plugins;
    
protected:

    static void _bind_methods();

    Node* _load_descriptor(const String& p_resource_path);

public:

    bool scan_directory(const String& p_directory);
    
    Dictionary get_plugins();

    Node* get_descriptor(const String& p_resource_path);
        
    bool add_plugin(const String& p_resource_path);

    BPluginManager();
    ~BPluginManager();

    static BPluginManager *get_singleton() { return singleton; }
};

#endif // BPLUGIN_MANAGER_H
