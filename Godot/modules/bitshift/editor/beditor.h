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
#ifndef BEDITOR_H
#define BEDITOR_H

#include "scene/main/node.h"

class MenuButton;
class BEditorGenerateHeightMap;
class BEditorGenerateNormalMap;

/**
        @author Fabian Mathews <supagu@gmail.com>
*/

class BEditor : public Node {

    GDCLASS(BEditor, Node)

    static BEditor *singleton;

protected:

    enum MenuOptions {
        BITSHIFT_GENERATE_HEIGHT_MAP,
        BITSHIFT_GENERATE_NORMAL_MAP,
    };


    BEditorGenerateHeightMap *generate_height_map;
    BEditorGenerateNormalMap *generate_normal_map;

    MenuButton* bitshift_menu;

    void _ready();
    void _notification(int p_what);
    void _menu_option(int p_option);

    static void _bind_methods();

public:

    static BEditor *get_singleton() { return singleton; }

    BEditor();
    ~BEditor();

};

#endif // BTHEME_H
