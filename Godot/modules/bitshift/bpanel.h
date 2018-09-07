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
#ifndef BPANEL_H
#define BPANEL_H

#include "scene/gui/panel.h"
#include "scene/gui/popup.h"
#include "scene/gui/texture_button.h"

class BPanel : public Control {

    GDCLASS(BPanel, Control);

public:
    enum DrawType {
        DRAW_PANEL,
        DRAW_WINDOW,
        DRAW_NONE,
    };

    enum DragType {
        DRAG_NONE = 0,
        DRAG_RESIZE_TOP = 1 << 0,
        DRAG_RESIZE_RIGHT = 1 << 1,
        DRAG_RESIZE_BOTTOM = 1 << 2,
        DRAG_RESIZE_LEFT = 1 << 3,
        DRAG_MOVE = 1 << 4,
    };

protected:

    int drag_type;
    Point2 drag_offset;
    Point2 drag_offset_far;
    bool resizable;

    int resize_flags;

    DrawType draw_type;

    bool dragable;

    bool default_visibility;

    void _gui_input(const Ref<InputEvent> &p_event);
    void _closed();
    int _drag_hit_test(const Point2 &pos) const;

    void _minimize_toggled(bool pressed);
    void _bring_to_front();

protected:
    bool _set(const StringName &p_name, const Variant &p_value);
    bool _get(const StringName &p_name, Variant &r_ret) const;
    void _get_property_list(List<PropertyInfo> *p_list) const;

    virtual void _post_popup();
    virtual void _fix_size();
    virtual void _close_pressed() {}
    virtual bool has_point(const Point2 &p_point) const;
    void _notification(int p_what);
    static void _bind_methods();

public:

    void set_resize_flags(int flags);
    int get_resize_flags() const;

    void set_default_visibility(bool p_default_visibility);
    bool get_default_visibility() const;

    //TextureButton *get_close_button();
    //TextureButton *get_minimize_button();

    //void set_title(const String &p_title);
    //String get_title() const;
    void set_resizable(bool p_resizable);
    bool get_resizable() const;

    //Size2 get_minimum_size() const;

    //void set_closable(bool p_closable);
    //bool get_closable() const;

    //void set_minimizable(bool p_minimizable);
    //bool get_minimizable() const;
    //bool is_minimized() const;

    void set_dragable(bool p_dragable);
    bool get_dragable() const;

    void set_draw_type(DrawType type);
    DrawType get_draw_type() const;

    BPanel();
    ~BPanel();
};

VARIANT_ENUM_CAST(BPanel::DrawType)
VARIANT_ENUM_CAST(BPanel::DragType)

#endif
