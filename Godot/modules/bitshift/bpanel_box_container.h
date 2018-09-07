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
#ifndef BPANEL_BOX_CONTAINER_H
#define BPANEL_BOX_CONTAINER_H

#include "scene/gui/container.h"

class PanelBoxContainer : public Container {

        GDCLASS(PanelBoxContainer, Container);

public:
        enum AlignMode {
                ALIGN_BEGIN,
                ALIGN_CENTER,
                ALIGN_END
        };

private:
        bool vertical;
        AlignMode align;

        void _resort();

protected:
        void _notification(int p_what);

        static void _bind_methods();

public:
        void add_spacer(bool p_begin = false);

        void set_alignment(AlignMode p_align);
        AlignMode get_alignment() const;

        virtual Size2 get_minimum_size() const;

        PanelBoxContainer(bool p_vertical = false);
};

class HPanelBoxContainer : public PanelBoxContainer {

        GDCLASS(HPanelBoxContainer, PanelBoxContainer);

public:
        HPanelBoxContainer() :
                        PanelBoxContainer(false) {}
};

class MarginContainer;
class VPanelBoxContainer : public PanelBoxContainer {

        GDCLASS(VPanelBoxContainer, PanelBoxContainer);

public:
        MarginContainer *add_margin_child(const String &p_label, Control *p_control, bool p_expand = false);

        VPanelBoxContainer() :
                        PanelBoxContainer(true) {}
};

VARIANT_ENUM_CAST(PanelBoxContainer::AlignMode);

#endif // BOX_CONTAINER_H
