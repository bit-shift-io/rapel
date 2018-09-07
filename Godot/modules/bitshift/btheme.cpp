
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
#include "btheme.h"
#include "editor/editor_settings.h"
#include "editor/editor_themes.h"
#include <assert.h>

#include "core/engine.h"
#include "core/io/resource_loader.h"
#include "editor_fonts.h"
#include "editor_icons.gen.h"
#include "editor_scale.h"
#include "editor_settings.h"
#include "modules/svg/image_loader_svg.h"
#include "time.h"

#ifndef TOOLS_ENABLED
    #define EDSCALE 1
#endif

BTheme *BTheme::singleton=NULL;

void BTheme::_bind_methods() {
    ClassDB::bind_method(D_METHOD("apply"),&BTheme::apply);
}

Ref<ImageTexture> _editor_generate_icon(int p_index, bool p_convert_color, float p_scale = EDSCALE, bool p_force_filter = false) {

    Ref<ImageTexture> icon = memnew(ImageTexture);
    Ref<Image> img = memnew(Image);

    // dumb gizmo check
    bool is_gizmo = String(editor_icons_names[p_index]).begins_with("Gizmo");

    ImageLoaderSVG::create_image_from_string(img, editor_icons_sources[p_index], p_scale, true, p_convert_color);

    if ((p_scale - (float)((int)p_scale)) > 0.0 || is_gizmo || p_force_filter)
        icon->create_from_image(img); // in this case filter really helps
    else
        icon->create_from_image(img, 0);

    return icon;
}

#ifndef ADD_CONVERT_COLOR
#define ADD_CONVERT_COLOR(dictionary, old_color, new_color) dictionary[Color::html(old_color)] = Color::html(new_color)
#endif

void _editor_register_and_generate_icons(Ref<Theme> p_theme, bool p_dark_theme = true, int p_thumb_size = 32, bool p_only_thumbs = false) {

//#ifdef SVG_ENABLED
    Dictionary dark_icon_color_dictionary;
    if (!p_dark_theme) {
        //convert color:                              FROM       TO
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#e0e0e0", "#4f4f4f"); // common icon color
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#ffffff", "#000000"); // white
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#b4b4b4", "#000000"); // script darker color

        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#cea4f1", "#bb6dff"); // animation
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#fc9c9c", "#ff5f5f"); // spatial
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#a5b7f3", "#6d90ff"); // 2d
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#708cea", "#0843ff"); // 2d dark
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#a5efac", "#29d739"); // control

        // rainbow
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#ff7070", "#ff2929"); // red
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#ffeb70", "#ffe337"); // yellow
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#9dff70", "#74ff34"); // green
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#70ffb9", "#2cff98"); // aqua
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#70deff", "#22ccff"); // blue
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#9f70ff", "#702aff"); // purple
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#ff70ac", "#ff2781"); // pink

        // audio gradient
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#ff8484", "#ff4040"); // red
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#e1dc7a", "#d6cf4b"); // yellow
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#84ffb1", "#00f010"); // green

        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#ffd684", "#fea900"); // mesh (orange)
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#40a2ff", "#68b6ff"); // shape (blue)

        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#ff8484", "#ff3333"); // remove (red)
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#84ffb1", "#00db50"); // add (green)
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#84c2ff", "#5caeff"); // selection (blue)

        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#ea686c", "#e3383d"); // key xform (red)

        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#69ecbd", "#25e3a0"); // VS variant
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#8da6f0", "#6d8eeb"); // VS bool
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#7dc6ef", "#4fb2e9"); // VS int
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#61daf4", "#27ccf0"); // VS float
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#6ba7ec", "#4690e7"); // VS string
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#bd91f1", "#ad76ee"); // VS vector2
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#f191a5", "#ee758e"); // VS rect
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#e286f0", "#dc6aed"); // VS vector3
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#c4ec69", "#96ce1a"); // VS transform2D
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#f77070", "#f77070"); // VS plane
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#ec69a3", "#ec69a3"); // VS quat
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#ee7991", "#ee7991"); // VS aabb
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#e3ec69", "#b2bb19"); // VS basis
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#f6a86e", "#f49047"); // VS transform
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#6993ec", "#6993ec"); // VS path
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#69ec9a", "#2ce573"); // VS rid
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#79f3e8", "#12d5c3"); // VS object
        ADD_CONVERT_COLOR(dark_icon_color_dictionary, "#77edb1", "#57e99f"); // VS dict
    }

    // these ones should be converted even if we are using a dark theme
    const Color error_color = p_theme->get_color("error_color", "Editor");
    const Color success_color = p_theme->get_color("success_color", "Editor");
    const Color warning_color = p_theme->get_color("warning_color", "Editor");
    dark_icon_color_dictionary[Color::html("#ff5d5d")] = error_color;
    dark_icon_color_dictionary[Color::html("#45ff8b")] = success_color;
    dark_icon_color_dictionary[Color::html("#ffdd65")] = warning_color;

    List<String> exceptions;
    exceptions.push_back("EditorPivot");
    exceptions.push_back("EditorHandle");
    exceptions.push_back("Editor3DHandle");
    exceptions.push_back("Godot");
    exceptions.push_back("PanoramaSky");
    exceptions.push_back("ProceduralSky");
    exceptions.push_back("EditorControlAnchor");
    exceptions.push_back("DefaultProjectIcon");
    exceptions.push_back("GuiCloseCustomizable");
    exceptions.push_back("GuiGraphNodePort");
    exceptions.push_back("GuiResizer");
    exceptions.push_back("ZoomMore");
    exceptions.push_back("ZoomLess");
    exceptions.push_back("ZoomReset");
    exceptions.push_back("LockViewport");
    exceptions.push_back("GroupViewport");
    exceptions.push_back("StatusError");
    exceptions.push_back("StatusSuccess");
    exceptions.push_back("StatusWarning");
    exceptions.push_back("NodeWarning");

    //clock_t begin_time = clock();

    ImageLoaderSVG::set_convert_colors(&dark_icon_color_dictionary);

    // generate icons
    if (!p_only_thumbs)
        for (int i = 0; i < editor_icons_count; i++) {
            List<String>::Element *is_exception = exceptions.find(editor_icons_names[i]);
            if (is_exception) exceptions.erase(is_exception);
            Ref<ImageTexture> icon = _editor_generate_icon(i, !is_exception);
            p_theme->set_icon(editor_icons_names[i], "EditorIcons", icon);
        }

    // generate thumb files with the given thumb size
    bool force_filter = !(p_thumb_size == 64 && p_thumb_size == 32); // we don't need filter with original resolution
    if (p_thumb_size >= 64) {
        float scale = (float)p_thumb_size / 64.0 * EDSCALE;
        for (int i = 0; i < editor_bg_thumbs_count; i++) {
            int index = editor_bg_thumbs_indices[i];
            List<String>::Element *is_exception = exceptions.find(editor_icons_names[index]);
            if (is_exception) exceptions.erase(is_exception);
            Ref<ImageTexture> icon = _editor_generate_icon(index, !p_dark_theme && !is_exception, scale, force_filter);
            p_theme->set_icon(editor_icons_names[index], "EditorIcons", icon);
        }
    } else {
        float scale = (float)p_thumb_size / 32.0 * EDSCALE;
        for (int i = 0; i < editor_md_thumbs_count; i++) {
            int index = editor_md_thumbs_indices[i];
            List<String>::Element *is_exception = exceptions.find(editor_icons_names[index]);
            if (is_exception) exceptions.erase(is_exception);
            Ref<ImageTexture> icon = _editor_generate_icon(index, !p_dark_theme && !is_exception, scale, force_filter);
            p_theme->set_icon(editor_icons_names[index], "EditorIcons", icon);
        }
    }

    ImageLoaderSVG::set_convert_colors(NULL);

    //clock_t end_time = clock();

    //double time_d = (double)(end_time - begin_time) / CLOCKS_PER_SEC;
//#else
//   print_line("Sorry no icons for you");
//#endif
}

bool BTheme::apply() {
    // set old theme as current
    copy(old_default.ptr());

    // grab icons from the editor theme
    // code grabbed from editor_themes.cpp
    {
        const bool dark_theme = true;
        const float default_contrast = 0.25;

        //Theme settings
        Color accent_color = Color::html("#699ce8");
        Color base_color = Color::html("#323b4f");
        float contrast = default_contrast;

        Color success_color = accent_color.linear_interpolate(Color(0.2, 1, 0.2), 0.6) * 1.2;
        Color warning_color = accent_color.linear_interpolate(Color(1, 1, 0), 0.7) * 1.2;
        Color error_color = accent_color.linear_interpolate(Color(1, 0, 0), 0.8) * 1.7;

        set_color("success_color", "Editor", success_color);
        set_color("warning_color", "Editor", warning_color);
        set_color("error_color", "Editor", error_color);

        const int thumb_size = 64; //EDITOR_DEF("filesystem/file_dialog/thumbnail_size", 64);
        //set_constant("scale", "Editor", EDSCALE);
        set_constant("thumb_size", "Editor", thumb_size);
        set_constant("dark_theme", "Editor", dark_theme);
        _editor_register_and_generate_icons(this, true, thumb_size);

        /*
        if (!EditorSettings::get_singleton())
            EditorSettings::create();

        Ref<Theme> editor_theme = create_editor_theme();
        List<StringName> type_list;
        editor_theme->get_type_list(&type_list);
        for (List<StringName>::Element *E = type_list.front(); E; E = E->next()) {
                const StringName& key = E->get();

                List<StringName> icon_list;
                editor_theme->get_icon_list(key, &icon_list);
                for (List<StringName>::Element *F = icon_list.front(); F; F = F->next()) {
                        Ref<Texture> value = editor_theme->get_icon(F->get(), key);
                        set_icon(F->get(), key, value);
                }
        }

        EditorSettings::destroy();
        */
    }

    // load theme from file
    Ref<Theme> theme = ResourceLoader::load("res://Theme/default.theme");
    if (!theme.is_valid()) {
        ERR_EXPLAIN("Failed to load theme: res://Theme/default.theme");
        ERR_FAIL_V(false);
    }

    // apply overrides from that we have loaded
    List<StringName> type_list;
    theme->get_type_list(&type_list);
    for (List<StringName>::Element *E = type_list.front(); E; E = E->next()) {
            const StringName& key = E->get();

            List<StringName> stylebox_list;
            theme->get_stylebox_list(key, &stylebox_list);
            for (List<StringName>::Element *F = stylebox_list.front(); F; F = F->next()) {
                    Ref<StyleBox> value = theme->get_stylebox(F->get(), key);
                    set_stylebox(F->get(), key, value);
            }

            List<StringName> font_list;
            theme->get_font_list(key, &font_list);
            for (List<StringName>::Element *F = font_list.front(); F; F = F->next()) {
                    Ref<Font> value = theme->get_font(F->get(), key);
                    set_font(F->get(), key, value);
            }

            List<StringName> icon_list;
            theme->get_icon_list(key, &icon_list);
            for (List<StringName>::Element *F = icon_list.front(); F; F = F->next()) {
                    Ref<Texture> value = theme->get_icon(F->get(), key);
                    set_icon(F->get(), key, value);
            }

            List<StringName> color_list;
            theme->get_color_list(key, &color_list);
            for (List<StringName>::Element *F = color_list.front(); F; F = F->next()) {
                    Color value = theme->get_color(F->get(), key);
                    set_color(F->get(), key, value);
            }

            List<StringName> constant_list;
            theme->get_constant_list(key, &constant_list);
            for (List<StringName>::Element *F = constant_list.front(); F; F = F->next()) {
                int value = theme->get_constant(F->get(), key);
                set_constant(F->get(), key, value);
            }
    }
    return true;
}

void BTheme::copy(Ref<Theme> other) {
    Ref<Theme> def = get_default();
    Theme::set_default(other);
    copy_default_theme();
    Theme::set_default(def);
}

void BTheme::shutdown() {
    Theme::set_default(Ref<Theme>());
    old_default = Ref<Theme>();
    //Theme::set_default(old_default);
}

BTheme::BTheme() {
    singleton = this;
/*
    // dont change the theme in the editor
    if (Engine::get_singleton()->is_editor_hint())
        return;
*/
    old_default = Theme::get_default();

    // bitshift specific values
    old_default->set_constant("separation", "HPanelBoxContainer", 4);
    old_default->set_constant("separation", "VPanelBoxContainer", 4);

    Theme::set_default(this);
    apply();
}

BTheme::~BTheme() {

}
