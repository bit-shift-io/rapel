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
#ifndef BDIALOGS_H
#define BDIALOGS_H

#include "scene/gui/panel.h"
#include "scene/gui/popup.h"
#include "scene/gui/texture_button.h"

class BWindowDialog : public Panel {

    GDCLASS(BWindowDialog, Panel);

    enum DRAG_TYPE {
        DRAG_NONE = 0,
        DRAG_MOVE = 1,
        DRAG_RESIZE_TOP = 1 << 1,
        DRAG_RESIZE_RIGHT = 1 << 2,
        DRAG_RESIZE_BOTTOM = 1 << 3,
        DRAG_RESIZE_LEFT = 1 << 4
    };

    TextureButton *close_button;
    String title;
    int drag_type;
    Point2 drag_offset;
    Point2 drag_offset_far;
    bool resizable;

    TextureButton *minimize_button;
    bool closable;
    bool minimizable;
    Size2 expanded_size;

    void _gui_input(const Ref<InputEvent> &p_event);
    void _closed();
    int _drag_hit_test(const Point2 &pos) const;

    void _minimize_toggled(bool pressed);
    void _bring_to_front();

protected:
    virtual void _post_popup();
    virtual void _fix_size();
    virtual void _close_pressed() {}
    virtual bool has_point(const Point2 &p_point) const;
    void _notification(int p_what);
    static void _bind_methods();

public:
    TextureButton *get_close_button();
    TextureButton *get_minimize_button();

    void set_title(const String &p_title);
    String get_title() const;
    void set_resizable(bool p_resizable);
    bool get_resizable() const;

    Size2 get_minimum_size() const;

    void set_closable(bool p_closable);
    bool get_closable() const;

    void set_minimizable(bool p_minimizable);
    bool get_minimizable() const;
    bool is_minimized() const;

    BWindowDialog();
    ~BWindowDialog();
};


#endif
