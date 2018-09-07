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

#include "bdialogs.h"
#include "butil.h"
#include "scene/gui/line_edit.h"
#include "print_string.h"
#include "translation.h"
#include "scene/main/viewport.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#endif

// BWindowDialog

void BWindowDialog::_post_popup() {

    drag_type = DRAG_NONE; // just in case
}

void BWindowDialog::_fix_size() {

    // Perhaps this should be called when the viewport resizes as well or windows go out of bounds...

    // Ensure the whole window is visible.
    Point2i pos = get_global_position();
    Size2i size = get_size();
    Size2i viewport_size = get_viewport_rect().size;

    // Windows require additional padding to keep the window chrome visible.
    Ref<StyleBox> panel = get_stylebox("panel", "WindowDialog");
    float top = 0;
    float left = 0;
    float bottom = 0;
    float right = 0;
    // Check validity, because the theme could contain a different type of StyleBox
    if (panel->get_class() == "StyleBoxTexture") {
        Ref<StyleBoxTexture> panel_texture = Object::cast_to<StyleBoxTexture>(*panel);
        top = panel_texture->get_expand_margin_size(MARGIN_TOP);
        left = panel_texture->get_expand_margin_size(MARGIN_LEFT);
        bottom = panel_texture->get_expand_margin_size(MARGIN_BOTTOM);
        right = panel_texture->get_expand_margin_size(MARGIN_RIGHT);
    } else if (panel->get_class() == "StyleBoxFlat") {
        Ref<StyleBoxFlat> panel_flat = Object::cast_to<StyleBoxFlat>(*panel);
        top = panel_flat->get_expand_margin_size(MARGIN_TOP);
        left = panel_flat->get_expand_margin_size(MARGIN_LEFT);
        bottom = panel_flat->get_expand_margin_size(MARGIN_BOTTOM);
        right = panel_flat->get_expand_margin_size(MARGIN_RIGHT);
    }

    pos.x = MAX(left, MIN(pos.x, viewport_size.x - size.x - right));
    pos.y = MAX(top, MIN(pos.y, viewport_size.y - size.y - bottom));
    set_global_position(pos);

    if (resizable) {
        size.x = MIN(size.x, viewport_size.x - left - right);
        size.y = MIN(size.y, viewport_size.y - top - bottom);
        set_size(size);
    }
}

bool BWindowDialog::has_point(const Point2 &p_point) const {

    Rect2 r(Point2(), get_size());

    // Enlarge upwards for title bar.
    int title_height = get_constant("title_height", "WindowDialog");
    r.position.y -= title_height;
    r.size.y += title_height;

    // Inflate by the resizable border thickness.
    if (resizable) {
        int scaleborder_size = get_constant("scaleborder_size", "WindowDialog");
        r.position.x -= scaleborder_size;
        r.size.width += scaleborder_size * 2;
        r.position.y -= scaleborder_size;
        r.size.height += scaleborder_size * 2;
    }

    return r.has_point(p_point);
}

void BWindowDialog::_gui_input(const Ref<InputEvent> &p_event) {

    Ref<InputEventMouseButton> mb = p_event;

    if (mb.is_valid() && mb->get_button_index() == BUTTON_LEFT) {

        if (mb->is_pressed()) {
            // Begin a possible dragging operation.
            drag_type = _drag_hit_test(Point2(mb->get_position().x, mb->get_position().y));
            if (drag_type != DRAG_NONE)
                drag_offset = get_global_mouse_position() - get_position();
            drag_offset_far = get_position() + get_size() - get_global_mouse_position();
        } else if (drag_type != DRAG_NONE && !mb->is_pressed()) {
            // End a dragging operation.
            drag_type = DRAG_NONE;
        }
    }

    Ref<InputEventMouseMotion> mm = p_event;

    if (mm.is_valid()) {

        if (drag_type == DRAG_NONE) {
            // Update the cursor while moving along the borders.
            CursorShape cursor = CURSOR_ARROW;
            if (resizable && !is_minimized()) {
                int preview_drag_type = _drag_hit_test(Point2(mm->get_position().x, mm->get_position().y));
                switch (preview_drag_type) {
                    case DRAG_RESIZE_TOP:
                    case DRAG_RESIZE_BOTTOM:
                        cursor = CURSOR_VSIZE;
                        break;
                    case DRAG_RESIZE_LEFT:
                    case DRAG_RESIZE_RIGHT:
                        cursor = CURSOR_HSIZE;
                        break;
                    case DRAG_RESIZE_TOP + DRAG_RESIZE_LEFT:
                    case DRAG_RESIZE_BOTTOM + DRAG_RESIZE_RIGHT:
                        cursor = CURSOR_FDIAGSIZE;
                        break;
                    case DRAG_RESIZE_TOP + DRAG_RESIZE_RIGHT:
                    case DRAG_RESIZE_BOTTOM + DRAG_RESIZE_LEFT:
                        cursor = CURSOR_BDIAGSIZE;
                        break;
                }
            }
            if (get_cursor_shape() != cursor)
                set_default_cursor_shape(cursor);
        } else {
            // Update while in a dragging operation.
            Point2 global_pos = get_global_mouse_position();
            global_pos.y = MAX(global_pos.y, 0); // Ensure title bar stays visible.

            Rect2 rect = get_rect();
            Size2 min_size = get_minimum_size();

            if (drag_type == DRAG_MOVE) {
                rect.position = global_pos - drag_offset;
            } else {
                if (drag_type & DRAG_RESIZE_TOP) {
                    int bottom = rect.position.y + rect.size.height;
                    int max_y = bottom - min_size.height;
                    rect.position.y = MIN(global_pos.y - drag_offset.y, max_y);
                    rect.size.height = bottom - rect.position.y;
                } else if (drag_type & DRAG_RESIZE_BOTTOM) {
                    rect.size.height = global_pos.y - rect.position.y + drag_offset_far.y;
                }
                if (drag_type & DRAG_RESIZE_LEFT) {
                    int right = rect.position.x + rect.size.width;
                    int max_x = right - min_size.width;
                    rect.position.x = MIN(global_pos.x - drag_offset.x, max_x);
                    rect.size.width = right - rect.position.x;
                } else if (drag_type & DRAG_RESIZE_RIGHT) {
                    rect.size.width = global_pos.x - rect.position.x + drag_offset_far.x;
                }
            }

            set_size(rect.size);
            set_position(rect.position);
        }
    }
}

void BWindowDialog::_notification(int p_what) {

    switch (p_what) {
        case NOTIFICATION_DRAW: {
            RID canvas = get_canvas_item();

            // Draw the background.
            Ref<StyleBox> panel = get_stylebox("panel", "WindowDialog");
            Size2 size = get_size();
            panel->draw(canvas, Rect2(0, 0, size.x, size.y));

            // Draw the title bar text.
            Ref<Font> title_font = get_font("title_font", "WindowDialog");
            Color title_color = get_color("title_color", "WindowDialog");
            int title_height = get_constant("title_height", "WindowDialog");
            int font_height = title_font->get_height() - title_font->get_descent() * 2;
            int x = (size.x - title_font->get_string_size(title).x) / 2;
            int y = (-title_height + font_height) / 2;
            title_font->draw(canvas, Point2(x, y), title, title_color, size.x - panel->get_minimum_size().x);
        } break;

        case NOTIFICATION_THEME_CHANGED:
        case NOTIFICATION_ENTER_TREE: {
            float x = get_constant("close_h_ofs", "WindowDialog");
            close_button->set_normal_texture(get_icon("close", "WindowDialog"));
            close_button->set_pressed_texture(get_icon("close", "WindowDialog"));
            close_button->set_hover_texture(get_icon("close_highlight", "WindowDialog"));
            close_button->set_anchor(MARGIN_LEFT, ANCHOR_END);
            close_button->set_begin(Point2(-x, -get_constant("close_v_ofs", "WindowDialog")));

            float y = 0;
            if (closable) {
                y = x;
                x *= 2;
            }

            minimize_button->set_normal_texture(get_icon("radio_checked", "CheckBox"));
            minimize_button->set_pressed_texture(get_icon("radio_unchecked", "CheckBox"));
            //minimize_button->set_hover_texture(get_icon("close_highlight", "WindowDialog"));
            minimize_button->set_anchor(MARGIN_LEFT, ANCHOR_END);
            minimize_button->set_begin(Point2(-x, -get_constant("close_v_ofs", "WindowDialog")));
            minimize_button->set_end(Point2(-y, -get_constant("close_v_ofs", "WindowDialog")));
        } break;

        case NOTIFICATION_MOUSE_EXIT: {
            // Reset the mouse cursor when leaving the resizable window border.
            if (resizable && !drag_type) {
                if (get_default_cursor_shape() != CURSOR_ARROW)
                    set_default_cursor_shape(CURSOR_ARROW);
            }
        } break;
#if 0
#ifdef TOOLS_ENABLED
        case NOTIFICATION_POST_POPUP: {
            if (get_tree() && Engine::get_singleton()->is_editor_hint() && EditorNode::get_singleton())
                EditorNode::get_singleton()->dim_editor(true);
        } break;
        case NOTIFICATION_POPUP_HIDE: {
            if (get_tree() && Engine::get_singleton()->is_editor_hint() && EditorNode::get_singleton())
                EditorNode::get_singleton()->dim_editor(false);
        } break;
#endif
#endif
    }
}

void BWindowDialog::_closed() {

    _close_pressed();
    hide();
}

int BWindowDialog::_drag_hit_test(const Point2 &pos) const {
    int drag_type = DRAG_NONE;

    if (resizable) {
        int title_height = get_constant("title_height", "WindowDialog");
        int scaleborder_size = get_constant("scaleborder_size", "WindowDialog");

        Rect2 rect = get_rect();

        if (pos.y < (-title_height + scaleborder_size))
            drag_type = DRAG_RESIZE_TOP;
        else if (pos.y >= (rect.size.height - scaleborder_size))
            drag_type = DRAG_RESIZE_BOTTOM;
        if (pos.x < scaleborder_size)
            drag_type |= DRAG_RESIZE_LEFT;
        else if (pos.x >= (rect.size.width - scaleborder_size))
            drag_type |= DRAG_RESIZE_RIGHT;
    }

    if (drag_type == DRAG_NONE && pos.y < 0)
        drag_type = DRAG_MOVE;

    return drag_type;
}

void BWindowDialog::set_title(const String &p_title) {

    title = tr(p_title);
    update();
}
String BWindowDialog::get_title() const {

    return title;
}

void BWindowDialog::set_resizable(bool p_resizable) {
    resizable = p_resizable;
}
bool BWindowDialog::get_resizable() const {
    return resizable;
}

Size2 BWindowDialog::get_minimum_size() const {

    Ref<Font> font = get_font("title_font", "WindowDialog");
    int msx = close_button->get_combined_minimum_size().x;
    msx += font->get_string_size(title).x;

    msx += minimize_button->get_combined_minimum_size().x;

    return Size2(msx, 1);
}

TextureButton *BWindowDialog::get_close_button() {

    return close_button;
}

void BWindowDialog::_bind_methods() {

    ClassDB::bind_method(D_METHOD("_gui_input"), &BWindowDialog::_gui_input);
    ClassDB::bind_method(D_METHOD("set_title", "title"), &BWindowDialog::set_title);
    ClassDB::bind_method(D_METHOD("get_title"), &BWindowDialog::get_title);
    ClassDB::bind_method(D_METHOD("set_resizable", "resizable"), &BWindowDialog::set_resizable);
    ClassDB::bind_method(D_METHOD("get_resizable"), &BWindowDialog::get_resizable);
    ClassDB::bind_method(D_METHOD("_closed"), &BWindowDialog::_closed);
    ClassDB::bind_method(D_METHOD("get_close_button"), &BWindowDialog::get_close_button);

    ClassDB::bind_method(D_METHOD("set_closable", "closable"), &BWindowDialog::set_closable);
    ClassDB::bind_method(D_METHOD("get_closable"), &BWindowDialog::get_closable);
    ClassDB::bind_method(D_METHOD("set_minimizable", "minimizable"), &BWindowDialog::set_minimizable);
    ClassDB::bind_method(D_METHOD("get_minimizable"), &BWindowDialog::get_minimizable);
    ClassDB::bind_method(D_METHOD("get_close_button:TextureButton"), &BWindowDialog::get_close_button);
    ClassDB::bind_method(D_METHOD("_minimize_toggled"), &BWindowDialog::_minimize_toggled);
    ClassDB::bind_method(D_METHOD("get_minimize_button:TextureButton"), &BWindowDialog::get_minimize_button);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "window_title", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT_INTL), "set_title", "get_title");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "resizable", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT_INTL), "set_resizable", "get_resizable");

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "closable", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT_INTL), "set_closable", "get_closable");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "minimizable", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT_INTL), "set_minimizable", "get_minimizable");
}

BWindowDialog::BWindowDialog() {

    //title="Hello!";
    drag_type = DRAG_NONE;
    resizable = false;
    close_button = memnew(TextureButton);
    add_child(close_button);
    close_button->connect("pressed", this, "_closed");

    set_closable(false);
    minimize_button = memnew(TextureButton);
    add_child(minimize_button);
    minimize_button->set_toggle_mode(true);
    minimize_button->connect("toggled", this, "_minimize_toggled");
    set_minimizable(false);
}

BWindowDialog::~BWindowDialog() {
}

TextureButton *BWindowDialog::get_minimize_button() {
    return minimize_button;
}

void BWindowDialog::set_closable(bool p_closable) {
    closable = p_closable;
    close_button->set_visible(closable);
}

bool BWindowDialog::get_closable() const {
    return closable;
}

void BWindowDialog::set_minimizable(bool p_minimizable) {
    minimizable = p_minimizable;
    minimize_button->set_visible(minimizable);
}

bool BWindowDialog::get_minimizable() const {
    return minimizable;
}

bool BWindowDialog::is_minimized() const {
    return minimize_button->is_pressed();
}

void BWindowDialog::_minimize_toggled(bool pressed) {
    BUtil::get_singleton()->set_children_visible(this, !pressed);

    // don't hide the items in the title bar!
    close_button->set_visible(closable);
    minimize_button->set_visible(minimizable);

    if (pressed) {
        Size2 size = get_size();
        expanded_size = size;
        size.height = 0; //get_minimum_size().height;
        set_size(size);
    } else {
        set_size(expanded_size);
    }
}

void BWindowDialog::_bring_to_front() {
    // remove all children and put our self as the first child
    // then add all the other children back
    Control* p = get_parent_control();
    if (p->get_child(p->get_child_count() - 1) == this)
            return;

    p->remove_child(this);
    p->add_child(this);
}
