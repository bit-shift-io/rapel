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
#include "bspline.h"
#include "bterrain.h"

PoolVector3Array BSpline::tesselate_along_terrain(float p_interval, float p_push_along_normal_distance) {
	
	// can we just move each control point to the terrain
	// then scan along and if there is a point of difference, we divide the curve
	// this way we have a spline that conforms to the terrain nicely still?
	
	
	//PoolVector3Array pointTest = tesselate();
	
	// consider converting this spline to terrain texture space, then tesselating
	set_bake_interval(p_interval);
	//float len = get_baked_length();
	PoolVector3Array points = get_baked_points();
	
	BTerrain* terrain = BTerrain::get_singleton();
	if (!terrain) {
		return points;
	}

	PoolVector3Array::Read r = points.read();
	PoolVector3Array::Write w = points.write();
	
	// TODO: bypass dictionary, bcluttermap does lots or raycasting as well
	// so work together with that to make a nice API so I dont have to do it as manual raycasting everywhere
	for (int i = 0; i < points.size(); ++i) {
		Dictionary d = terrain->raycast_down(r[i]);
		Vector3 pos = d["position"];
		//Vector3 norm = d["normal"];
		w[i] = pos + Vector3(0, p_push_along_normal_distance, 0); // + norm * p_push_along_normal_distance;
	}
	
	return points;
}

Transform BSpline::get_transform_from_distance_along_curve(float p_offset, float p_look_ahead) {
	float len = get_baked_length();
	ERR_FAIL_COND_V(len <= 0, Transform());

	float direction = 1.0f;
	float path_offset = CLAMP(p_offset, 0, len);
	float look_ahead_offset = path_offset + p_look_ahead;
	
	if (look_ahead_offset > len || look_ahead_offset < 0) {
		look_ahead_offset = path_offset - p_look_ahead;
		direction = -1.0f;
	}
	
	assert(path_offset >= 0.f && path_offset <= len);
	assert(look_ahead_offset >= 0.f && look_ahead_offset <= len);
	
	Vector3 pos = interpolate_baked(path_offset, false);
	Vector3 look_ahead_pos = interpolate_baked(look_ahead_offset, false);
	
	/*
	float path_offset = CLAMP(p_offset, p_look_ahead, len - p_look_ahead);
	Vector3 pos = interpolate_baked(path_offset, false);
	Vector3 look_ahead_pos = interpolate_baked(path_offset + p_look_ahead, false);
	*/
	
	Vector3 facing = (look_ahead_pos - pos);
	facing *= direction;
	facing.normalize();
	Transform t;
	t = t.looking_at(facing, Vector3(0, 1, 0));
	t.origin = pos;
	return t;
}

void BSpline::_bind_methods() {
	ClassDB::bind_method(D_METHOD("tesselate_along_terrain", "interval", "push_along_normal_distance"),&BSpline::tesselate_along_terrain);
	ClassDB::bind_method(D_METHOD("get_transform_from_distance_along_curve", "offset", "look_ahead"),&BSpline::get_transform_from_distance_along_curve, DEFVAL(0.01));
}

BSpline::BSpline() {
}

BSpline::~BSpline() {
}