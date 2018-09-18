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
#include "barc_line_curve_node.h"
#include "barc_line_curve.h"
#include "butil.h"
#include "bterrain.h"
#include "core/core_string_names.h"
#include <assert.h>


Ref<BArcLineCurve> BArcLineCurveNode::get_curve() {
	return curve;
}

void BArcLineCurveNode::set_curve(const Ref<BArcLineCurve>& p_curve) {
	curve = p_curve; 
}
/*
void BArcLineCurveNode::_notification(int p_what) {
	if (p_what == NOTIFICATION_LOCAL_TRANSFORM_CHANGED) {
		curve->set_global_transform(get_global_transform());
	}
}*/

void BArcLineCurveNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_curve"), &BArcLineCurveNode::get_curve);
	ClassDB::bind_method(D_METHOD("set_curve"), &BArcLineCurveNode::set_curve);
	
	ADD_PROPERTYNZ(PropertyInfo(Variant::OBJECT, "curve", PROPERTY_HINT_NONE, ""), "set_curve", "get_curve");
}

BArcLineCurveNode::BArcLineCurveNode() {
	curve = Ref<BArcLineCurve>(memnew(BArcLineCurve)); 
}

BArcLineCurveNode::~BArcLineCurveNode() {
}
