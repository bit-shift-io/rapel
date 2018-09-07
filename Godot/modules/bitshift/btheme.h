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
#ifndef BTHEME_H
#define BTHEME_H

#include "reference.h"
#include "scene/resources/theme.h"

/**
        @author Fabian Mathews <supagu@gmail.com>
*/

class BTheme : public Theme {

    GDCLASS(BTheme,Theme)

    static BTheme *singleton;

    Ref<Theme> old_default;

protected:

    static void _bind_methods();

public:

    bool apply();
    void copy(Ref<Theme> other);
    void shutdown();

    PoolVector<String> get_icon_list(const String &p_type) const { return _get_icon_list(p_type); }
    PoolVector<String> get_stylebox_list(const String &p_type) const { return _get_stylebox_list(p_type); }
    PoolVector<String> get_stylebox_types(void) const { return _get_stylebox_types(); }
    PoolVector<String> get_font_list(const String &p_type) const { return _get_font_list(p_type); }
    PoolVector<String> get_color_list(const String &p_type) const { return _get_color_list(p_type); }
    PoolVector<String> get_constant_list(const String &p_type) const { return _get_constant_list(p_type); }
    PoolVector<String> get_type_list(const String &p_type) const { return _get_type_list(p_type); }

    static BTheme *get_singleton() { return singleton; }

    BTheme();
    ~BTheme();

};

#endif // BTHEME_H
