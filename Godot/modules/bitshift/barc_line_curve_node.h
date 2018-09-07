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
#ifndef BARC_LINE_NODE_H
#define BARC_LINE_NODE_H

#include "scene/3d/spatial.h"
#include "core/vector.h"
#include <assert.h>

class BArcLineCurve;

/**
	@author Fabian Mathews <supagu@gmail.com>
 
 A curve made of arcs and lines, but this is a node form
*/

class BArcLineCurveNode : public Spatial  {

    GDCLASS(BArcLineCurveNode, Spatial);
    
	Ref<BArcLineCurve> curve;

protected:

	static void _bind_methods();
	
	//void _notification(int p_what);
	
public:
	
	Ref<BArcLineCurve> get_curve();
	void set_curve(const Ref<BArcLineCurve>& p_curve);
		
    BArcLineCurveNode();
    ~BArcLineCurveNode();       
};

#endif // BARC_LINE_NODE_H
