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

#ifndef EDITOR_GENERATE_NORMAL_MAP_H
#define EDITOR_GENERATE_NORMAL_MAP_H

#include "scene/gui/dialogs.h"

class PropertyEditor;
class NormalMapSettings;
class RichTextLabel;

class BEditorGenerateNormalMap : public ConfirmationDialog {
    GDCLASS(BEditorGenerateNormalMap, ConfirmationDialog);

    ConfirmationDialog *results;
    RichTextLabel *results_label;

    PropertyEditor *property_editor;
    NormalMapSettings *normal_map_settings;

    virtual void ok_pressed();

    void _results_confirm();
    static void _bind_methods();

public:
    void popup();
    void show_message(const String& msg);

    BEditorGenerateNormalMap();
    ~BEditorGenerateNormalMap();
};


#endif // EDITOR_GENERATE_NORMAL_MAP_H
