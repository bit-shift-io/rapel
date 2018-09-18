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
#ifndef BCONFIG_H
#define BCONFIG_H

#include "core/reference.h"
#include "core/io/config_file.h"

/**
        @author Fabian Mathews <supagu@gmail.com>

        Config system for storing and retriving settings
*/

class BConfig : public Reference {

    GDCLASS(BConfig,Reference)

    static BConfig *singleton;

    bool dirty;

    String settings_file_path;
    Ref<ConfigFile> settings;

    Ref<ConfigFile> project_settings;

    Dictionary _opts; // map options to value

protected:

    static void _bind_methods();

public:

    void save();
    Variant get_setting(String p_key, Variant p_default_value);
    void set_setting(String p_key, Variant p_value);
    bool has_setting(String p_key);

    bool is_build_type(String type);

    Ref<ConfigFile> get_project_settings();

    BConfig();
    ~BConfig();

    static BConfig *get_singleton() { return singleton; }
};

#endif // BCONFIG_H
