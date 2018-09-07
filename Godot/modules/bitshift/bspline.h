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
#ifndef BSPLINE_H
#define BSPLINE_H

#include "scene/resources/curve.h"
#include "bterrain.h"
#include <assert.h>


/**
	@author Fabian Mathews <supagu@gmail.com>
*/

class BSpline : public Curve3D  {

    GDCLASS(BSpline, Curve3D);
    
protected:

	static void _bind_methods();
	
public:
	
	PoolVector3Array tesselate_along_terrain(float p_interval, float p_push_along_normal_distance);
	
	Transform get_transform_from_distance_along_curve(float p_offset, float p_look_ahead = 0.01);
    	
    BSpline();
    ~BSpline();       
};

#endif // BSPLINE_H
