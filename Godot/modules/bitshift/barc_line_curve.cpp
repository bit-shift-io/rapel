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
#include "barc_line_curve.h"
#include "butil.h"
#include "bterrain.h"
#include "core/core_string_names.h"
#include "core/map.h"
#include <assert.h>

void BArcLineCurve::_dirty() {
    baked_cache_dirty = true;
    _change_notify();
    emit_signal(CoreStringNames::get_singleton()->changed);
}

void BArcLineCurve::_bake() {
	if (!baked_cache_dirty) 
		return;
	
	curve.clear();
	for (int i = 0; i < points.size() - 1; ++i) {		
        //print_line("_bake_segment start");
        _bake_segment(i, &points.write[i], &points.write[i + 1]); //points[i].pos, points[i].out, points[i + 1].pos, points[i + 1].in);
        //print_line("_bake_segment end");
	}
	
	baked_cache_dirty = false;
}

BArcLineCurve::GeometricPrimitive BArcLineCurve::GeometricPrimitive::create(const Point* p_a, const Point* p_b) {
	GeometricPrimitive prim;
	prim.points[0] = p_a;
	prim.points[1] = p_b;
	return prim;
}

void BArcLineCurve::GeometricPrimitive::start_arc(const Vector3 &p_centre, float p_radius, const Vector3& p_start_tangent_point, const Vector3& p_start_tangent_dir) {
	type = GPT_ARC;
	arc.angle_arc = 0.f;
	arc.radius = p_radius;
	
	arc.centre[0] = p_centre.x;
	arc.centre[1] = p_centre.y;
	arc.centre[2] = p_centre.z;
	
	arc.begin[0] = p_start_tangent_point.x;
	arc.begin[1] = p_start_tangent_point.y;
	arc.begin[2] = p_start_tangent_point.z;
	arc.angle_begin = compute_arc_angle(arc.get_begin());
	
	// compute the direction of the movement
	//
	// https://math.stackexchange.com/questions/116042/finding-the-component-of-a-vector-tangent-to-a-circle
	// for computing clockwise or counterclockwise
	//
	// I can just use the corss product of the vector from centre to tangent point with the tangent direction
	arc.arc_direction = compute_tangent_point_direction(p_start_tangent_point, p_start_tangent_dir);
	
	int nothing = 0;
	++nothing;
}

int BArcLineCurve::GeometricPrimitive::compute_tangent_point_direction(const Vector3 &p_tangent_point, const Vector3 &p_tangent_dir) {
	assert(type == GPT_ARC);
	
	if (p_tangent_dir.length_squared() <= 0.f) 
		return 0;
	
	Vector3 vec_to_tangent_pt = (p_tangent_point - arc.get_centre());
	
	Vector3 cross = vec_to_tangent_pt.cross(p_tangent_dir);
    if (cross.y == 0.f) {
        print_line("Cant compute compute_tangent_point_direction, circle likely has 0 radius");
		return 0;
    }
	
	float dir = cross.y > 0.f ? -1 : 1;
	return dir;
}

int BArcLineCurve::GeometricPrimitive::end_arc_from_external_pt(const Vector3 &p_point) {
	assert(type == GPT_ARC);
	
	Vector3 ta;
	Vector3 tb;
	int result = compute_arc_tangent_points_from_external_point(p_point, &ta, &tb);
	if (result != 1) {
		return result;
	}
	
	Vector3 ta_dir = p_point - ta;
	Vector3 tb_dir = p_point - tb;
	
	int a_dir = compute_tangent_point_direction(ta, ta_dir);
	int b_dir = compute_tangent_point_direction(tb, tb_dir);

	// no tangent specified, so simply choose the shortest route
	if (arc.arc_direction == 0) {
		
		//float delta_aa = compute_angle_between(arc.get_begin(), ta);
		//float delta_ab = compute_angle_between(arc.get_begin(), tb);
        print_line("error in end_arc_from_external_pt - should really never get here");
		assert(false); // I don't know if we will ever get here!
	
		return 1;
	}
	// tangent ta is moving in the same direction as the start of the arc, then use that!
	else if (arc.arc_direction == a_dir) {
		end_arc_at_tangent_pt(ta);
		return 1;
	}
	// tangent tb is moving in the same direction as the start of the arc, then use that!
	else if (arc.arc_direction == b_dir) {
		end_arc_at_tangent_pt(tb);
		return 1;
	}
	
    // likely we are on the circle, and ta == tb == p_point
    //
    if (tb.length_squared()) {
        end_arc_at_tangent_pt(tb);
        return 1;
    }
    else if (ta.length_squared()) {
        end_arc_at_tangent_pt(ta);
        return 1;
    }

    print_line("error in end_arc_from_external_pt - should never get here");
	assert(false); // huh?
	return 0;
}

void BArcLineCurve::GeometricPrimitive::end_arc_at_tangent_pt(const Vector3 &p_tangent_point) {
	arc.end[0] = p_tangent_point.x;
	arc.end[1] = p_tangent_point.y;
	arc.end[2] = p_tangent_point.z;
	arc.angle_arc = Math::abs(compute_angle_between(arc.get_begin(), p_tangent_point, arc.arc_direction));
    assert(!isnan(arc.angle_arc));
}

float BArcLineCurve::GeometricPrimitive::compute_angle_between(const Vector3 &p_start_tangent, const Vector3 &p_end_tangent, int direction) {
	// given the direction, compute the angle from p_start_tangent to p_end_tangent
	
	// http://stackoverflow.com/questions/23408572/angle-between-two-points-0-360-cw
	
	float start_angle = compute_arc_angle(p_start_tangent);
	float end_angle = compute_arc_angle(p_end_tangent);
	float result_angle;
	
	if (Math::is_equal_approx(start_angle, end_angle)) {
		return 0.f;
	}
	
	// counterclock wise
	if (direction == 1) {
		
		if (start_angle > end_angle) {
			result_angle = (Math_PI * 2) + (end_angle - start_angle);
		}
		else {
			result_angle = end_angle - start_angle;
		}
	}
	// clockwise
	else {
		
		if (start_angle > end_angle) {
			result_angle = -(end_angle - start_angle);
		}
		else {
			result_angle = (Math_PI * 2) - (end_angle - start_angle);
		}
	}

    if (fabs(result_angle) >= (2 * Math_PI)) {
        print_line("Error, angle is greater than 360 degrees");
    }

    assert(!isnan(result_angle));
	return result_angle;
}

void BArcLineCurve::GeometricPrimitive::start_line(const Vector3 &p_start) {
	type = GPT_LINE;
	line.begin[0] = p_start.x;
	line.begin[1] = p_start.y;
	line.begin[2] = p_start.z;
}

void BArcLineCurve::GeometricPrimitive::end_line(const Vector3 &p_end) {
	assert(type == GPT_LINE);
	line.end[0] = p_end.x;
	line.end[1] = p_end.y;
	line.end[2] = p_end.z;
}

void BArcLineCurve::GeometricPrimitive::reverse() {
	if (type == GPT_LINE) {
		Vector3 begin = line.get_begin();
		Vector3 end = line.get_end();
		
		line.end[0] = begin[0];
		line.end[1] = begin[1];
		line.end[2] = begin[2];
		
		line.begin[0] = end[0];
		line.begin[1] = end[1];
		line.begin[2] = end[2];
	}
	else if (type == GPT_ARC) {
		Arc copy = arc;
		arc.end[0] = copy.begin[0];
		arc.end[1] = copy.begin[1];
		arc.end[2] = copy.begin[2];
		
		arc.begin[0] = copy.end[0];
		arc.begin[1] = copy.end[1];
		arc.begin[2] = copy.end[2];
		
		arc.angle_begin = compute_arc_angle(arc.get_begin());		
		arc.arc_direction = -copy.arc_direction;
	}
}

float BArcLineCurve::GeometricPrimitive::compute_arc_angle(const Vector3 &p_tangent_point) {
	assert(type == GPT_ARC);
	
	// circle formula for a point on the circle
	// given a centre, and radius
	// (x, y) = centre + (sin(theta), cos(theta)) * radius
	// (x, y) = (sin(theta) * r + centre.x, cos(theta) * r + centre.y)
	//
	// x = cos(theta) * r + centre.x
	// y = sin(theta) * r + centre.y
	//
	// (x - centre.x) / r = cos(theta)
	// (y - centre.y) / r = sin(theta)
	//
	// theta = cos-1((x - centre.x) / r)
	// theta = sin-1((y - centre.y) / r)
	
	// TODO: check this more
	//float arc_start_angle = asin((p_tangent_point.x - arc.centre[0]) / arc.radius); // something wrong here!
	//float arc_start_angle_2 = acos((p_tangent_point.z - arc.centre[2]) / arc.radius);
	//assert(arc_start_angle == arc_start_angle_2);
	
	// https://gamedev.stackexchange.com/questions/33709/get-angle-in-radians-given-a-point-on-a-circle
	float arc_angle = Math::atan2(p_tangent_point.z - arc.centre[2], p_tangent_point.x - arc.centre[0]);
	
	// if the angle goes over 180, it will be a negative number
	// as atan2 ranges from -PI to PI, i change that from 0 to 2 * PI
	if (arc_angle < 0.f)
		arc_angle += 2 * Math_PI;
	
    assert(!isnan(arc_angle));
	return arc_angle;
}

Vector3 BArcLineCurve::GeometricPrimitive::compute_best_arc_centre(float radius, const Vector3 &p_tangent_point, const Vector3 &p_tangent, const Vector3 &p_target_point) {
	
	Vector3 up(0, 1, 0);
	
    Vector3 right_dir = up.cross(p_tangent);
	right_dir.normalize();
	right_dir *= radius;

	Vector3 right_pos = p_tangent_point + right_dir;
	Vector3 left_pos = p_tangent_point - right_dir;

	Vector3 arc_centre;

	// which side is closer to p_b, right_pos or left_pos?
	float left_dir_to_target = (left_pos - p_target_point).length_squared();
	float right_dir_to_target = (right_pos - p_target_point).length_squared();
	if (left_dir_to_target < right_dir_to_target) {
		arc_centre = left_pos;
	}
	else {
		arc_centre = right_pos;
	}
	
	return arc_centre;
}

/* circle_circle_intersection() *
 * Determine the points where 2 circles in a common plane intersect.
 *
 * int circle_circle_intersection(
 *                                // center and radius of 1st circle
 *                                double x0, double y0, double r0,
 *                                // center and radius of 2nd circle
 *                                double x1, double y1, double r1,
 *                                // 1st intersection point
 *                                double *xi, double *yi,              
 *                                // 2nd intersection point
 *                                double *xi_prime, double *yi_prime)
 *
 * This is a public domain work. 3/26/2005 Tim Voght
 *
 */
#include <stdio.h>
#include <math.h>

int circle_circle_intersection(float x0, float y0, float r0,
                               float x1, float y1, float r1,
                               float *xi, float *yi,
                               float *xi_prime, float *yi_prime)
{
  float a, dx, dy, d, h, rx, ry;
  float x2, y2;

  /* dx and dy are the vertical and horizontal distances between
   * the circle centers.
   */
  dx = x1 - x0;
  dy = y1 - y0;

  /* Determine the straight-line distance between the centers. */
  //d = sqrt((dy*dy) + (dx*dx));
  d = hypot(dx,dy); // Suggested by Keith Briggs

  /* Check for solvability. */
  if (d > (r0 + r1))
  {
    /* no solution. circles do not intersect. */
    print_line("no solution. one circle is contained in the other");
    return 0;
  }
  if (d < fabs(r0 - r1))
  {
    /* no solution. one circle is contained in the other */
    print_line("no solution. one circle is contained in the other");
    return 0;
  }

  /* 'point 2' is the point where the line through the circle
   * intersection points crosses the line between the circle
   * centers.  
   */

  /* Determine the distance from point 0 to point 2. */
  a = ((r0*r0) - (r1*r1) + (d*d)) / (2.0 * d) ;

  /* Determine the coordinates of point 2. */
  x2 = x0 + (dx * a/d);
  y2 = y0 + (dy * a/d);

  /* Determine the distance from point 2 to either of the
   * intersection points.
   */
  float s = Math::absf((r0*r0) - (a*a)); // FM: added an absf, or should I just clamp to zero?
  h = sqrt(s);
  assert(!isnan(h));

  /* Now determine the offsets of the intersection points from
   * point 2.
   */
  rx = -dy * (h/d);
  ry = dx * (h/d);

  /* Determine the absolute intersection points. */
  *xi = x2 + rx;
  *xi_prime = x2 - rx;
  *yi = y2 + ry;
  *yi_prime = y2 - ry;

  assert(!isnan(*xi));
  assert(!isnan(*xi_prime));
  assert(!isnan(*yi));
  assert(!isnan(*yi_prime));

  return 1;
}

int BArcLineCurve::GeometricPrimitive::compute_arc_tangent_points_from_external_point(const Vector3 &p_external_point, Vector3 *p_a, Vector3 *p_b) {
	assert(type == GPT_ARC);

	// http://www.mathwarehouse.com/geometry/circle/equation-of-a-circle.php
	// equation for a circle is (x - h)^2 + (y - k)^2 = r^2
	// where r = radius, (h, k) is the centre
	// 
	// using thales theorm
	// http://math.stackexchange.com/questions/697574/how-to-find-the-coordination-of-a-tangent-point-on-a-circle
	// we have two circles intersecting
	//
	// (x - h)^2 + (y - k)^2 = r^2
	// (x - h)^2 = r^2 - (y - k)^2
	// x = sqrt(r^2 - (y - k)^2) + h
	// y = sqrt(r^2 - (x - h)^2) + k

	// sqrt(r1^2 - (y - k1)^2) + h1 = sqrt(r2^2 - (y - k2)^2) + h2

	// c code here:
	// http://paulbourke.net/geometry/circlesphere/
	
	// http://jsfiddle.net/zxqCw/1/
	
    Vector3 arc_centre = Vector3(arc.get_centre().x, 0, arc.get_centre().z);
    Vector3 external_pt = Vector3(p_external_point.x, 0, p_external_point.z);
    Vector3 delta = arc_centre - external_pt;
    float delta_length = delta.length();
    if (delta_length < arc.radius) {
        print_line("external_point is inside the circle!");
    }
    Vector3 mid_point = external_pt + delta * 0.5f;
    float new_circle_radius = delta_length * 0.5f;
	
	// equation for circle:
	// x = cos(theta) * radius + centre.x				y = sin(theta) * radius + centre.y
	//
	// where two circles meet, we want theta:
	// 
	
	
	p_a->y = arc.get_centre().y;
	p_b->y = arc.get_centre().y;

    // radius of zero, means the centre of the arc is the tangent points, it will line on the circle
    if (Math::is_equal_approx(arc.radius, 0.f)) {
        *p_a = arc.get_centre();
        *p_b = arc.get_centre();
        return 1;
    }

	// http://stackoverflow.com/questions/3349125/circle-circle-intersection-points
    int result = circle_circle_intersection(arc.get_centre().x, arc.get_centre().z, arc.radius, //MAX(arc.radius, 0.01),
                               mid_point.x, mid_point.z, new_circle_radius,
                               &p_a->x, &p_a->z,
                               &p_b->x, &p_b->z);

    assert(!isnan(p_a->x));
    assert(!isnan(p_a->z));
    assert(!isnan(p_b->x));
    assert(!isnan(p_b->z));
	return result;
}

Transform BArcLineCurve::GeometricPrimitive::get_transform(float p_percentage) {
	Transform t;
	if (type == GPT_LINE) {		
		t = t.looking_at(line.get_end() - line.get_begin(), Vector3(0, 1, 0));
		t.origin = line.get_begin().linear_interpolate(line.get_end(), p_percentage);
		return t;
	}
	
	// else GPT_ARC
	//float arc_dir = (arc.angle_end > arc.angle_begin) ? 1.f : -1.f; // 1 is CCW, -1 is CW
	float arc_angle = arc.angle_begin + (arc.angle_arc * p_percentage * arc.arc_direction);
	//float forward_arc_angle = arc_angle + (0.01 * arc.arc_direction);
	
	// https://www.mathsisfun.com/sine-cosine-tangent.html
	// hrmm not sure tan can give us a good direction
	//float t_angle = tan(arc_angle); 
	
	Vector3 pos_a = Vector3(Math::cos(arc_angle) * arc.radius, 0.f, Math::sin(arc_angle) * arc.radius) + arc.get_centre();
	
	/* - old tangent computation code, not as accurate as we need!
	Vector3 pos_b = Vector3(Math::cos(forward_arc_angle) * arc.radius, 0.f, Math::sin(forward_arc_angle) * arc.radius) + arc.get_centre();
	Vector3 dir = pos_b - pos_a;
	dir.normalize();
	*/
	Vector3 tangent = Vector3(-Math::sin(arc_angle) * arc.arc_direction, 0.f, Math::cos(arc_angle) * arc.arc_direction);
	
	//t = t.looking_at(tangent, Vector3(0, 1, 0));
	tangent = -tangent;
	Vector3 right = Vector3(0, 1, 0).cross(tangent);
	t.basis.set_axis(0, right);
    //assert(BUtil::is_equal_approx(t.basis.get_axis(0), right));
	t.basis.set_axis(1, Vector3(0, 1, 0));
    //assert(BUtil::is_equal_approx(t.basis.get_axis(1), Vector3(0, 1, 0)));
	t.basis.set_axis(2, tangent);
    //assert(BUtil::is_equal_approx(t.basis.get_axis(2), tangent));
	t.origin = pos_a;	
	return t;
}

bool BArcLineCurve::GeometricPrimitive::closest_distance_to(const Vector3& p_pos, DistanceInfo* p_dist_info, float p_distance_max) {
	assert(p_dist_info);
	Vector3 pos = p_pos;
	
	if (p_distance_max >= 0.f) {
        AABB aabb_expanded = aabb.grow(p_distance_max);;
		if (!aabb_expanded.has_point(pos)) {
			return false;
		}
	}
	
	if (type == GPT_ARC) {
		// https://math.stackexchange.com/questions/2239426/closest-point-on-arc
		
		// compute the line from pos to the centre of the circle
		// normalise and multiply by the radius to find the point where the circle is at its closest
		// to pos
		Vector3 centre_to_pos = pos - arc.get_centre();
		//float dist_to_pos = centre_to_pos.length();
		Vector3 dir_to_pos = centre_to_pos.normalized();
		Vector3 tangent_on_circle = arc.get_centre() + dir_to_pos * arc.radius;
		
		float angle_between = compute_angle_between(arc.get_begin(), tangent_on_circle, arc.arc_direction);
		if (angle_between >= 0 && angle_between <= arc.angle_arc) {
			p_dist_info->closest_point = tangent_on_circle;
			p_dist_info->distance = (p_dist_info->closest_point- pos).length();
			p_dist_info->percent = angle_between / arc.angle_arc;
		}
		else {
			// subtract half the angle so we draw a line down the centre (which is at the half angle)
			// and work out which side the point is on
			float angle_from_arc_half_angle = angle_between - (arc.angle_arc * 0.5f);
			if (angle_from_arc_half_angle < Math_PI) {
				p_dist_info->closest_point = arc.get_end();
				p_dist_info->distance = (pos - arc.get_end()).length();
				p_dist_info->percent = 1.f;
			}
			else {
				p_dist_info->closest_point = arc.get_begin();
				p_dist_info->distance = (pos - arc.get_begin()).length();
				p_dist_info->percent = 0.f;
			}
		}		
	}
	// its a line
	else {		
		Vector3 segment[2] = {
			line.get_begin(),
			line.get_end(),
		};
		
		p_dist_info->closest_point = get_closest_point_to_segment(pos, segment, &p_dist_info->percent);
		p_dist_info->distance = (pos - p_dist_info->closest_point).length();
	}
	
	return true;
}

// modified code from Geometry.h
Vector3 BArcLineCurve::GeometricPrimitive::get_closest_point_to_segment(const Vector3 &p_point, const Vector3 *p_segment, float* p_percent) {

	Vector3 p = p_point - p_segment[0];
	Vector3 n = p_segment[1] - p_segment[0];
	real_t l = n.length();
	if (l < 1e-10)
		return p_segment[0]; // both points are the same, just give any
	n /= l;

	real_t d = n.dot(p);

	if (d <= 0.0) {
		*p_percent = 0.f;
		return p_segment[0]; // before first point
	}
	else if (d >= l) {
		*p_percent = 1.f;
		return p_segment[1]; // after first point
	}
	else {
		*p_percent = d / l;
		return p_segment[0] + n * d; // inside
	}
}

void BArcLineCurve::GeometricPrimitive::add_to_curve(const BArcLineCurve& curve) {
	// finalise any caching we need to do
	
	// compute length
	if (type == GPT_ARC) {
		// https://www.khanacademy.org/math/geometry/hs-geo-circles/hs-geo-arc-length-rad/v/arc-length-from-angle-measure
		length = Math::abs(arc.radius * arc.angle_arc);
		
		// how do i encompass an arc? just add the midpoint?
		aabb.set_position(arc.get_begin());
		aabb.expand_to(arc.get_end());
		Vector3 midpoint = get_transform(0.5f).origin;
		aabb.expand_to(midpoint);
	}
	// its a line
	else {
		length = (line.get_begin() - line.get_end()).length();
		
		aabb.set_position(line.get_begin());
		aabb.expand_to(line.get_end());
	}
	
	// some sanity checking
        //assert(length >= 0 && length < 1000);
        if (length > 1000)
            print_line("Length of BArcLineCurve > 1000, are you sure this is right?");

        if (length <= 0)
            print_line("Length of BArcLineCurve <= 0, are you sure this is right?");
	
	// expand a little if any size is zero, we get issues!
	const float AABB_EXPANSION_SIZE = 0.1f;
	aabb.grow_by(AABB_EXPANSION_SIZE);
	
	if (curve.curve.size() == 0)
		curve.aabb = aabb;
	else
		curve.aabb.merge_with(aabb);
	
	curve.curve.push_back(*this);
}

void BArcLineCurve::_bake_segment(int i, Point* p_a, Point* p_b) {

	// _bake_segment(i, &points[i], &points[i + 1]); //points[i].pos, points[i].out, points[i + 1].pos, points[i + 1].in);
	Vector3 p_out = p_a->out;
	Vector3 p_in = p_b->in;
	Vector3 pt_a = p_a->pos;
	Vector3 pt_b = p_b->pos;

//    print_line("a.out:" + p_out);
//    print_line("b.in:" + p_in);
//    print_line("a:" + pt_a);
//    print_line("b:" + pt_b);
	
	assert(Math::is_equal_approx(p_out.y, 0.f));
	assert(Math::is_equal_approx(p_in.y, 0.f));
	assert(Math::is_equal_approx(pt_a.y, 0.f));
	assert(Math::is_equal_approx(pt_b.y, 0.f));

	// check the point is reachable
	// if not, push p_b out to the min distance
	Vector3 a_to_b = pt_b - pt_a;
	a_to_b.y = 0.f;
	float dist_sqrd_a_to_b = a_to_b.length_squared();
	float radius_sqrd = radius_min * radius_min;
	
	// just fail for now, two points on top of each other!
    if (dist_sqrd_a_to_b <= 0.f) {
        //print_line("a to b is zero or less, aborting");
		return;
    }
	
	Vector3 dir_a_to_b = a_to_b.normalized();
	
	// check if the supplied tangents lie on the line,
	// if so, clear the tangents so a straight line is produced
	bool out_on_line = p_out.length_squared() <= 0.f || BUtil::is_equal_approx(dir_a_to_b, p_out);
	bool in_on_line = p_in.length_squared() <= 0.f || BUtil::is_equal_approx(dir_a_to_b, -p_in);
	if (out_on_line && in_on_line) {
		p_out = Vector3();
		p_in = Vector3();
	}
	
	// if p_out has length, we have a tangent
	// so need to start with a circle
	BArcLineCurve::GeometricPrimitive start_prim = BArcLineCurve::GeometricPrimitive::create(p_a, p_b);
	if (p_out.length_squared() > 0.f) {
		Vector3 arc_centre = start_prim.compute_best_arc_centre(radius_min, pt_a, p_out, pt_b);		
		start_prim.start_arc(arc_centre, radius_min, pt_a, p_out);

        // if point b is inside the circle, then compute the closest point on the circle for point b move out by epsilon to ensure its outside the circle by a hairs breadth!
        Vector3 centre_to_b = (arc_centre - pt_b);
        float dist_centre_to_b = centre_to_b.length();
        if (dist_centre_to_b < radius_min) {
            Vector3 centre_to_b_dir = (pt_b - arc_centre).normalized();
            pt_b = arc_centre + (centre_to_b_dir * (radius_min + CMP_EPSILON));
            p_b->pos = pt_b; // move the point!
            //print_line("a to b is less than diameter, pushing out to closest pt on arc");
        }
	}
	// else we can simple start with a line 
	else {
		start_prim.start_line(pt_a);
	}


	
	// now lets move on to the output values
	// if we have a p_in, we end with a outgoing tangent to pt_b
	if (p_in.length_squared() > 0.f) {
		
		BArcLineCurve::GeometricPrimitive end_prim  = BArcLineCurve::GeometricPrimitive::create(p_a, p_b);
		Vector3 arc_centre = end_prim.compute_best_arc_centre(radius_min, pt_b, p_in, pt_a);
		end_prim.start_arc(arc_centre, radius_min, pt_b, p_in);
		
		float len_sqrd_between_arc_centres = (arc_centre - start_prim.arc.get_centre()).length_squared();
		bool same_circle = Math::is_equal_approx(0.f, len_sqrd_between_arc_centres);
			
		// check if we started with a line
		// here we are finishing with an arc
		if (start_prim.type == GPT_LINE) {
						
			int result = end_prim.end_arc_from_external_pt(pt_a);
			if (result != 1) {
				// failure
				print_line("could not finish arc, pt_a probably inside the circle!");
				return;
			}
			
			// now reverse the arc to make it the correct way!
			end_prim.reverse();
			
			// finish the start_prim by connecting it to the start of the end arc
			start_prim.end_line(end_prim.arc.get_begin());
			start_prim.add_to_curve(*this);
			
			end_prim.add_to_curve(*this);
			return;
		}
		
		// else we started with an arc, and we are finishing with an arc
		//
		// two possibilities, outer tangent or inner tangent
		// https://en.wikipedia.org/wiki/Tangent_lines_to_circles
		// this case starts with a circle, and ends with a circle
		// and has a line in the middle
		//
		// end direction in relation to start direction determines if we need inner or outer tangents
		
		int start_arc_direction = start_prim.arc.arc_direction;
		int end_arc_direction = -end_prim.arc.arc_direction; // reverse direction because end is not yet finished (it is reversed when we are finished with it!)
		
		// they go in the same direction, so we need outer tangent
		if (start_arc_direction == end_arc_direction) {

			// this means the input point and output point lie on the same circle
			// and their tangents to in the same direction
			// so we have a single arc
			if (same_circle) {
				start_prim.end_arc_at_tangent_pt(pt_b);
				start_prim.add_to_curve(*this);
				return;
			}
			
            // because the radius of the start and end circles is the same
            // computing the tangent points on the circle is really simple!

            Vector3 up(0, 1, 0);
            Vector3 dir_start_to_end = (end_prim.arc.get_centre() - start_prim.arc.get_centre()).normalized();
            Vector3 centre_to_tangent_dir = up.cross(dir_start_to_end);

            // now I just need to work out which tangent point we need based on the direction of the arc
            centre_to_tangent_dir *= start_arc_direction;

            Vector3 start_tangent_pt = start_prim.arc.get_centre() + centre_to_tangent_dir * start_prim.arc.radius;
            Vector3 end_tangent_pt = end_prim.arc.get_centre() + centre_to_tangent_dir * end_prim.arc.radius;

            start_prim.end_arc_at_tangent_pt(start_tangent_pt);
            end_prim.end_arc_at_tangent_pt(end_tangent_pt);
            end_prim.reverse();

            BArcLineCurve::GeometricPrimitive middle_prim = BArcLineCurve::GeometricPrimitive::create(p_a, p_b);

            middle_prim.start_line(start_tangent_pt);
            middle_prim.end_line(end_tangent_pt);

            start_prim.add_to_curve(*this);
            middle_prim.add_to_curve(*this);
            end_prim.add_to_curve(*this);

            // old code below here:
            // is buggy and doesnt work when the two radius' are equal
            // just keeping this here incase we have curves with different radius' in future

            /*
			// deal with outer tangents
			// https://en.wikipedia.org/wiki/Tangent_lines_to_circles
			//			
			// from the two circles: start_prim and end_prim
			// we construct a new circle, circle_prim
            // this new circle is positioned at end_prim, and has the radius of (end_prim.radius - start_prim.radius) (or which ever has the larger radius)
			// this reduces the the smaller circle to be a point and so we have a tangent from an external point
			// problem to be solved!
            // we then compute the tangent points on this new larger circle and move it out based on the smaller circle radius
			// then we have the direction of tangency we can apply to the end_prim (or the larger circle)
			// which we can use as a -ve value to compute the tangent points on the start circle also!
			//
			// https://en.wikipedia.org/wiki/File:Aeussere_tangente.svg
			//
			
			BArcLineCurve::GeometricPrimitive smaller_circle = start_prim;
			BArcLineCurve::GeometricPrimitive larger_circle = end_prim;
			if (start_prim.arc.radius > end_prim.arc.radius) {
				larger_circle = start_prim;
				smaller_circle = end_prim;
				assert(false); // test this code
			}
					
			BArcLineCurve::GeometricPrimitive circle_prim = BArcLineCurve::GeometricPrimitive::create(p_a, p_b);
			circle_prim.start_arc(larger_circle.arc.get_centre(), larger_circle.arc.radius - smaller_circle.arc.radius, Vector3(), Vector3());
			
			Vector3 ta;
			Vector3 tb;
			
            // if ta_dir is zero length, it means circle_prim.radius == 0, which means the tangent direction
            // is the vector from small circle to large circle
            if (Math::is_equal_approx(0.f, circle_prim.arc.radius)) {
                Vector3 up(0, 1, 0);
                Vector3 tangent = (larger_circle.arc.get_centre() - smaller_circle.arc.get_centre()).normalized();
                Vector3 centre_to_tangent = up.cross(tangent) * larger_circle.arc.radius;
                ta = smaller_circle.arc.get_centre() + centre_to_tangent;
                tb = smaller_circle.arc.get_centre() - centre_to_tangent;
            }
            else {
                int result = circle_prim.compute_arc_tangent_points_from_external_point(smaller_circle.arc.get_centre(), &ta, &tb);
                if (result != 1) {
                    print_line("could not finish arc, probably intersecting circles!");
                    return;
                }
            }

            // ta_dir is the vector from the centre to the tangent point
			Vector3 ta_dir = smaller_circle.arc.get_centre() - ta;
			//Vector3 tb_dir = smaller_circle.arc.get_centre() - tb;

			int a_dir = circle_prim.compute_tangent_point_direction(ta, ta_dir);
			//int b_dir = circle_prim.compute_tangent_point_direction(tb, tb_dir);
			
			// now we know which tangent we need, we choose the one that is moving in the same direction 
			Vector3 tangent_point;
			if (start_arc_direction == -a_dir) {
				tangent_point = ta;
			}
			else {
				tangent_point = tb;
			}
			
			Vector3 centre_to_tangent_dir = tangent_point - end_prim.arc.get_centre();
			centre_to_tangent_dir.normalize();
			
			Vector3 start_tangent_pt = start_prim.arc.get_centre() + centre_to_tangent_dir * start_prim.arc.radius;
			Vector3 end_tangent_pt = end_prim.arc.get_centre() + centre_to_tangent_dir * end_prim.arc.radius;
			
			start_prim.end_arc_at_tangent_pt(start_tangent_pt);
			end_prim.end_arc_at_tangent_pt(end_tangent_pt);
			end_prim.reverse();
			
            BArcLineCurve::GeometricPrimitive middle_prim = BArcLineCurve::GeometricPrimitive::create(p_a, p_b);
			
			middle_prim.start_line(start_tangent_pt);
			middle_prim.end_line(end_tangent_pt);
			
			start_prim.add_to_curve(*this);
			middle_prim.add_to_curve(*this);
            end_prim.add_to_curve(*this);*/
			return;
		}
		// they go in different directions, so we need to compute the inner tangents
		else {
			// deal with inner tangents
			// https://en.wikipedia.org/wiki/Tangent_lines_to_circles
			//			
			// from the two circles: start_prim and end_prim
			// we construct a new circle, circle_prim
			// this new circle is positioned at end_prim, and has the radius of start_prim + end_prim
			// we then compute the tengent points on this new larger circle
			// then we have the direction of tangency we can apply to the end_prim
			// which we can use as a -ve value to compute the tangent points on the start circle also!
			//
			// https://en.wikipedia.org/wiki/Tangent_lines_to_circles#/media/File:Innere_tangente.svg
			//
			BArcLineCurve::GeometricPrimitive circle_prim = BArcLineCurve::GeometricPrimitive::create(p_a, p_b);
			circle_prim.start_arc(end_prim.arc.get_centre(), start_prim.arc.radius + end_prim.arc.radius, Vector3(), Vector3());
			
			Vector3 ta;
			Vector3 tb;
			int result = circle_prim.compute_arc_tangent_points_from_external_point(start_prim.arc.get_centre(), &ta, &tb);
			if (result != 1) {
				print_line("could not finish arc, probably intersecting circles!");
				return;
			}

			Vector3 ta_dir = start_prim.arc.get_centre() - ta;
			//Vector3 tb_dir = start_prim.arc.get_centre() - tb;

			int a_dir = circle_prim.compute_tangent_point_direction(ta, ta_dir);
			//int b_dir = circle_prim.compute_tangent_point_direction(tb, tb_dir);
			
			// now we know which tangent we need, we choose the one that is moving in the same direction 
			Vector3 tangent_point;
			if (end_prim.arc.arc_direction == a_dir) {
				tangent_point = ta;
			}
			else {
				tangent_point = tb;
			}
			
			Vector3 centre_to_tangent_dir = tangent_point - end_prim.arc.get_centre();
			centre_to_tangent_dir.normalize();
			
			Vector3 start_tangent_pt = start_prim.arc.get_centre() - centre_to_tangent_dir * start_prim.arc.radius;
			Vector3 end_tangent_pt = end_prim.arc.get_centre() + centre_to_tangent_dir * end_prim.arc.radius;
			
			start_prim.end_arc_at_tangent_pt(start_tangent_pt);
			end_prim.end_arc_at_tangent_pt(end_tangent_pt);
			end_prim.reverse();
			
			BArcLineCurve::GeometricPrimitive middle_prim = BArcLineCurve::GeometricPrimitive::create(p_a, p_b);
			middle_prim.start_line(start_tangent_pt);
			middle_prim.end_line(end_tangent_pt);
			
			start_prim.add_to_curve(*this);
			middle_prim.add_to_curve(*this);
			end_prim.add_to_curve(*this);
			return;
		}
	}
	// else we can simply end with a line
	else {
		// check if we started with a line, and we are ending with a line
		// easy!
		if (start_prim.type == GPT_LINE) {
			start_prim.end_line(pt_b);
			start_prim.add_to_curve(*this);
			return;
		}

		// else we are dealing with an arc at the start, and a line at the end
		// finish the shape!				

		int result = start_prim.end_arc_from_external_pt(pt_b);
		if (result != 1) {
			print_line("error computing arc from pt_b, probably inside the circle!");
			return;
		}
		start_prim.add_to_curve(*this);
		
		BArcLineCurve::GeometricPrimitive end_prim = BArcLineCurve::GeometricPrimitive::create(p_a, p_b);
		end_prim.start_line(start_prim.arc.get_end());
		end_prim.end_line(pt_b);
		end_prim.add_to_curve(*this);		
		return;
	}
	
	assert(false); // shouldnt get here!
}

Dictionary BArcLineCurve::closest_distance_to(const Vector3& p_pos, float p_distance_max) {
	
	assert(!local); // debugging
	
	//Vector3 local_pos = global_transform.xform_inv(p_pos); // convert from world space to local space
	Vector3 local_pos = p_pos;
	local_pos.y = 0.f;
	
	Dictionary d;
	if (baked_cache_dirty)
		_bake();
	
	if (p_distance_max >= 0.f) {
        AABB aabb_expanded = aabb.grow(p_distance_max);;
		if (!aabb_expanded.has_point(local_pos)) {
			return d;
		}
	}
	
	DistanceInfo closest_dist_info;
	float closest_index = -1;
	for (int i = 0; i < curve.size(); ++i) {			
		DistanceInfo dist_info;
        bool ok = curve.write[i].closest_distance_to(local_pos, &dist_info, p_distance_max);
		if (ok) {
			if (dist_info.distance < closest_dist_info.distance || closest_index == -1) {
				closest_dist_info = dist_info;
				closest_index = i;
			}
		}
	}
	
	// not close enough to the curve to register the distance
	if (p_distance_max >= 0.f) {
		if (closest_dist_info.distance > p_distance_max || closest_index == -1) {
			return d;
		}
	}
	
	// convert to a distance along the curve
	// which can be used with get_transform
	float length = 0.f;
	for (int i = 0; i < curve.size(); ++i) {
		if (i == closest_index) {
			length += curve[i].length * closest_dist_info.percent;
			
			d["distance_to_curve"] = closest_dist_info.distance; 
			d["distance_along_curve"] = length;
			d["point_on_curve"] = closest_dist_info.closest_point;
			d["curve"] = this;
			return d;
		}
		length += curve[i].length;
	}
	
	// curve is empty
	return d;
}

float BArcLineCurve::get_baked_length() {
	if (baked_cache_dirty)
		_bake();
	
	// should this be cached when we do a _bake?
	float length = 0.f;
	for (int i = 0; i < curve.size(); ++i) {
		length += curve[i].length;
	}
	
	return length;
}

void BArcLineCurve::apply_transform_to_points(const Transform& p_transform) {
	for (int i = 0; i < points.size(); ++i) {
		Vector3 pos = p_transform.xform(points[i].pos);
		Vector3 in = p_transform.basis.xform(points[i].in); // not sure this is right!
		Vector3 out = p_transform.basis.xform(points[i].out);
		
		pos.y = 0.f;
		
		in.y = 0.f;
		in.normalize();
		
		out.y = 0.f;
		out.normalize();
		
        points.write[i].pos = pos;
        points.write[i].in = in;
        points.write[i].out = out;
	}
	
	validate_points();
    _dirty();
}

int BArcLineCurve::get_point_count() const {
	return points.size();
}

int BArcLineCurve::add_point(const Vector3 &p_pos, const Vector3 &p_in, const Vector3 &p_out, int p_atpos) {
    Point n;
    n.pos = p_pos;
    n.pos.y = 0.f;

    n.in = p_in;
    n.in.y = 0.f;
    n.in.normalize();

    n.out = p_out;
    n.out.y = 0.f;
    n.out.normalize();

    int atpos = -1;
    if (p_atpos >= 0 && p_atpos < points.size()) {
            points.insert(p_atpos, n);
            atpos = p_atpos;
    }
    else {
            atpos = points.size();
            points.push_back(n);
    }

    _dirty();
    //_bake();
    //assert(atpos != -1);
    //validate_points();

    int length = get_baked_length();
    if (length > 1000)
        print_line("Length is > 1000, probably not a good idea?");

    return atpos;
}

void BArcLineCurve::set_point_pos(int p_index, const Vector3 &p_pos) {
	ERR_FAIL_INDEX(p_index, points.size());
	Vector3 pos = p_pos;
	pos.y = 0.f;	
    points.write[p_index].pos = pos;
    _dirty();
    //_bake();
    //validate_points();
    //emit_signal(CoreStringNames::get_singleton()->changed);
    //_change_notify();
}

Vector3 BArcLineCurve::get_point_pos(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, points.size(), Vector3());
	return points[p_index].pos;
}

void BArcLineCurve::set_point_in(int p_index, const Vector3 &p_in) {
	ERR_FAIL_INDEX(p_index, points.size());

	Vector3 in = p_in;
	in.y = 0.f;	
	in.normalize();
	
    points.write[p_index].in = in;

    _dirty();
    //baked_cache_dirty = true;
	
    //validate_points();
    //emit_signal(CoreStringNames::get_singleton()->changed);
    //_change_notify();
}
Vector3 BArcLineCurve::get_point_in(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, points.size(), Vector3());
	return points[p_index].in;
}

void BArcLineCurve::set_point_out(int p_index, const Vector3 &p_out) {
	ERR_FAIL_INDEX(p_index, points.size());

	Vector3 out = p_out;
	out.y = 0.f;	
	out.normalize();
	
    points.write[p_index].out = out;

    _dirty();
    //baked_cache_dirty = true;
	
    //validate_points();
    //emit_signal(CoreStringNames::get_singleton()->changed);
    //_change_notify();
}

Vector3 BArcLineCurve::get_point_out(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, points.size(), Vector3());
	return points[p_index].out;
}

void BArcLineCurve::remove_point(int p_index) {
	ERR_FAIL_INDEX(p_index, points.size());
	validate_points();
	points.remove(p_index);

    _dirty();

    //baked_cache_dirty = true;
    //emit_signal(CoreStringNames::get_singleton()->changed);
    //_change_notify();
}

void BArcLineCurve::clear_points() {
	points.clear();

    _dirty();
    //baked_cache_dirty = true;
    //emit_signal(CoreStringNames::get_singleton()->changed);
    //_change_notify();
}

Transform BArcLineCurve::get_transform_from_distance_along_curve(float p_offset) {
	if (baked_cache_dirty)
		_bake();
	
	validate_points();
	
        if (!curve.size())
            return Transform();
	
	if (p_offset < 0)
		p_offset = 0.0f;
		
	// find which segment/primitive we are in and query it giving it a percentage
	float length = 0.f;
	for (int i = 0; i < curve.size(); ++i) {
		float end_length = length + curve[i].length;
		if (p_offset >= length && p_offset < end_length) {
			float percent = (p_offset - length) / curve[i].length;
            return curve.write[i].get_transform(percent); //modify_transform(curve[i].get_transform(percent));
		}
		
		length = end_length;
	}
	
	// past the end of the spline, return the end point
    return curve.write[curve.size() - 1].get_transform(1.0f); //modify_transform(curve[curve.size() - 1].get_transform(1.0f));
}

/* Moved to bcurve_mesh
Transform BArcLineCurve::modify_transform(const Transform& t) {

	validate_points();
	
	// TODO: optimise!
	// note: this is temporary, and should be not enforced to occur,
	// ie. trains will never to a raycast
	// only when we want it to deform to the terrain
	// maybe some adaptive subdivision to follow the terrain to also reduce jitter and bumps
	// only divide the spline when it deviates to far from the terrain
	BTerrain* terrain = BTerrain::get_singleton();
	if (!terrain) {
		return t;
	}
	
	Transform tm = t;
	//tm = global_transform * tm;
	
	const float p_push_along_normal_distance = 1.0f;
	
	tm = terrain->move_transform_to_ground(tm);

    // raycast down till we reach the terrain to see if there is any physics objects
    // blocking us
    Ref<World> world = terrain->get_world();
    PhysicsDirectSpaceState* space_state = world->get_direct_space_state();
    if (space_state) {
        Vector3 from = tm.origin + tm.get_basis().get_axis(1) * 1000.0f;
        Vector3 to = tm.origin;

        Set<RID> exclude_set;
        PhysicsDirectSpaceState::RayResult ray_result;
        bool result = space_state->intersect_ray(from, to, ray_result, exclude_set);
        if (result) {
            Vector3 out_normal = ray_result.normal;
            Vector3 binormal =  tm.get_basis().get_axis(0).cross(out_normal);
            Vector3 tangent = out_normal.cross(tm.get_basis().get_axis(2));
            tm.basis.set_axis(0, tangent);
            tm.basis.set_axis(1, out_normal);
            tm.basis.set_axis(2, binormal);

            tm.origin = ray_result.position;
        }
    }

    // push along normal a small distance
    tm.origin += tm.get_basis().get_axis(1) * p_push_along_normal_distance;


	return tm;
}*/

PoolVector3Array BArcLineCurve::tesselate_along_terrain(float p_interval, float p_push_along_normal_distance) {
	if (baked_cache_dirty)
		_bake();
	
	validate_points();
	
	// TODO: optimise
	// hrmm this is wasteful processing generating transforms here!
	PoolVector3Array results;
	float length = get_baked_length();
	for (float i = 0.f; i < length; i += p_interval) {
		Transform t = get_transform_from_distance_along_curve(i);
		results.push_back(t.origin);
	}
	
	Transform t = get_transform_from_distance_along_curve(1.0f);
	results.push_back(t.origin);
	
	return results;
}
/*
void BArcLineCurve::set_global_transform(const Transform& p_transform) {
	global_transform = p_transform;
}

Transform BArcLineCurve::get_global_transform() const {
	return global_transform;
}
*/
void BArcLineCurve::copy(const Ref<BArcLineCurve>& other) {
	clear_points();
    radius_min = other->radius_min;
	for (int i = 0; i < other->points.size(); ++i) {
		points.push_back(other->points[i]);
	}

    _dirty();
    //baked_cache_dirty = true;
    //validate_points();
    //_change_notify();
}

Variant BArcLineCurve::pack() const {	
	Array array;
    array.push_back(radius_min); // pack the radius
	for (int i = 0; i < points.size(); ++i) {
		Array point_arr;
		
		point_arr.push_back(points[i].pos);
		point_arr.push_back(points[i].in);
		point_arr.push_back(points[i].out);
		
		
		array.push_back(point_arr);
	}
	return array;
}

void BArcLineCurve::unpack(const Variant& variant) {
	assert(variant.is_array());
	Array array = variant;
	clear_points();
    radius_min = array.pop_front();
	for (int i = 0; i < array.size(); ++i) {
		Array point_arr = array[i];
        Variant p0 = point_arr[0];
        Variant p1 = point_arr[1];
        Variant p2 = point_arr[2];

        add_point(p0, p1, p2);
	}	
	
	validate_points();
}

Array BArcLineCurve::subdivide(const Array& distances_along_curve) {
	Array sorted_dist = distances_along_curve;
	sorted_dist.sort();
	
	Array results;
	
	float length = get_baked_length();	
	float start = 0.f;
	float end = 0.f;
	for (int i = 0; i < sorted_dist.size(); ++i) {
		end = sorted_dist[i];			
		Ref<BArcLineCurve> sub_curve = get_sub_curve(start, end);
		if (sub_curve.is_valid() && sub_curve->points.size() >= 2)
			results.push_back(sub_curve);
		
		start = end;
	}
	// for now lets get the first circle done right
	if (end < length) {
		end = length;
		Ref<BArcLineCurve> sub_curve = get_sub_curve(start, end);
		if (sub_curve.is_valid() && sub_curve->points.size() >= 2)
			results.push_back(sub_curve);
	}
	
	return results;
}

Ref<BArcLineCurve> BArcLineCurve::get_sub_curve(float start, float end) {
	if (baked_cache_dirty)
		_bake();
	
	// zero length
	if (Math::is_equal_approx(start, end)) {
		return Ref<BArcLineCurve>();
	}
	
	Ref<BArcLineCurve> result = Ref<BArcLineCurve>(memnew(BArcLineCurve));
	Map<const Point*, bool> used_points;
	bool started = false;
	float length = 0.f;
	int i = 0;
	int points_size = points.size();
	int curve_size = curve.size();
	for (; i < curve_size; ++i) {
        GeometricPrimitive& prim = curve.write[i];
		float end_length = length + prim.length;
			
		// some middle point
		if (started) {
			int p = 0;
			if (!used_points.has(prim.points[p])) {
				result->points.push_back(*prim.points[p]);
				used_points[prim.points[p]] = true;
			}
		}
		
		// start point found
		if (start >= length && start < end_length) {
			// starting at the very start!
			if (start <= 0.f) {
				int p = 0;
				result->points.push_back(*prim.points[p]);
				used_points[prim.points[p]] = true;
			}
			else {
				float percent = (start - length) / prim.length;
				Transform t = prim.get_transform(percent);
				int atpos = result->add_point(t.origin);

				// if we start on a curve we must include a starting tangent
				// else the curve will start with a line instead of a curve
				if (prim.type == GPT_ARC) {
					result->set_point_out(atpos, -t.basis.get_axis(2));
				}
			}
			started = true;
		}
		
		// end point found
		if (end >= length && end < end_length) {
			assert(started);
			float percent = (end - length) / prim.length;
			Transform t = prim.get_transform(percent);
			int atpos = result->add_point(t.origin);
			
			// if we end on a curve we must include a tangent
			// else the curve will end with a line instead of a curve
			if (prim.type == GPT_ARC) {
				result->set_point_in(atpos, t.basis.get_axis(2));
			}
			
			/*
			if (prim.type == GPT_ARC) {
				BArcLineCurve::GeometricPrimitive p0  = BArcLineCurve::GeometricPrimitive::create(NULL, NULL);
				Vector3 arc_centre0 = p0.compute_best_arc_centre(radius_min, result->points[0].pos, result->points[0].out, result->points[1].pos);
				assert(BUtil::is_equal_approx(arc_centre0, prim.arc.get_centre()));
				
				BArcLineCurve::GeometricPrimitive p1  = BArcLineCurve::GeometricPrimitive::create(NULL, NULL);
				Vector3 arc_centre1 = p1.compute_best_arc_centre(radius_min, t.origin, -t.basis.get_axis(2), result->points[0].pos);
				assert(BUtil::is_equal_approx(arc_centre1, prim.arc.get_centre()));
				
				assert(BUtil::is_equal_approx(arc_centre0, arc_centre1));
			}
			
			result->_bake(); // temporary whilsde debugging
			 */ 
			assert(result->points.size() >= 2);
			return result;
		}
		
		// ensure the first point used by this primitive is not used
		// we have passed it at this point
		used_points[prim.points[0]] = true;
		
		length = end_length;
	}
	
	// if we are here, we have go to the end and need to add the end point
	assert(started);
	assert(i == curve.size());
	int p = 1;
    GeometricPrimitive& prim = curve.write[i - 1];
	result->points.push_back(*prim.points[p]);	
	/*
	result->_bake(); // temporary while debugging
	 */
	assert(result->points.size() >= 2);
	return result;
}

Ref<BArcLineCurve> BArcLineCurve::get_reversed() {
	Ref<BArcLineCurve> result = Ref<BArcLineCurve>(memnew(BArcLineCurve));
	result->copy(this);
	result->points.invert();
    //result->baked_cache_dirty = true;
	
	// TODO: reverse the in/our direction vectors of each point
	
    //validate_points();
    //result->validate_points();
    //result->_change_notify();
    result->_dirty();

	return result;
}

void BArcLineCurve::validate_points() {
	/*
	for (int i = 0; i < points.size(); ++i) {
		assert(Math::is_equal_approx(points[i].out.y, 0.f));
		assert(Math::is_equal_approx(points[i].in.y, 0.f));
		assert(Math::is_equal_approx(points[i].pos.y, 0.f));
	}*/
}

void BArcLineCurve::set_radius(float radius) {
    radius_min = radius;
    _dirty();
    //baked_cache_dirty = true;
    //_change_notify();
}

void BArcLineCurve::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_point_count"), &BArcLineCurve::get_point_count);
	ClassDB::bind_method(D_METHOD("set_point_pos", "idx", "pos"), &BArcLineCurve::set_point_pos);
	ClassDB::bind_method(D_METHOD("get_point_pos", "idx"), &BArcLineCurve::get_point_pos);
	ClassDB::bind_method(D_METHOD("add_point", "pos", "in", "out", "atpos"), &BArcLineCurve::add_point, DEFVAL(Vector3()), DEFVAL(Vector3()), DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("get_baked_length"), &BArcLineCurve::get_baked_length);
	ClassDB::bind_method(D_METHOD("set_point_in", "idx", "pos"), &BArcLineCurve::set_point_in);
	ClassDB::bind_method(D_METHOD("get_point_in", "idx"), &BArcLineCurve::get_point_in);
	ClassDB::bind_method(D_METHOD("set_point_out", "idx", "pos"), &BArcLineCurve::set_point_out);
	ClassDB::bind_method(D_METHOD("get_point_out", "idx"), &BArcLineCurve::get_point_out);
	ClassDB::bind_method(D_METHOD("remove_point", "idx"), &BArcLineCurve::remove_point);
	ClassDB::bind_method(D_METHOD("clear_points"), &BArcLineCurve::clear_points);
	ClassDB::bind_method(D_METHOD("get_transform_from_distance_along_curve", "offset"),&BArcLineCurve::get_transform_from_distance_along_curve);
	ClassDB::bind_method(D_METHOD("tesselate_along_terrain", "interval", "push_along_normal_distance"),&BArcLineCurve::tesselate_along_terrain);
	ClassDB::bind_method(D_METHOD("closest_distance_to", "position", "distance_max"),&BArcLineCurve::closest_distance_to, DEFVAL(-1));	
//	ClassDB::bind_method(D_METHOD("set_global_transform"), &BArcLineCurve::set_global_transform);
//	ClassDB::bind_method(D_METHOD("get_global_transform"), &BArcLineCurve::get_global_transform);
	ClassDB::bind_method(D_METHOD("copy"), &BArcLineCurve::copy);

    ClassDB::bind_method(D_METHOD("get_radius"), &BArcLineCurve::get_radius);
    ClassDB::bind_method(D_METHOD("set_radius"), &BArcLineCurve::set_radius);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "radius", PROPERTY_HINT_NONE, "BArcLineCurve", PROPERTY_USAGE_NOEDITOR), "set_radius", "get_radius");
	
	ClassDB::bind_method(D_METHOD("pack"), &BArcLineCurve::pack);
	ClassDB::bind_method(D_METHOD("unpack"), &BArcLineCurve::unpack);
	
	ClassDB::bind_method(D_METHOD("subdivide"), &BArcLineCurve::subdivide);
	ClassDB::bind_method(D_METHOD("get_reversed"), &BArcLineCurve::get_reversed);
	
	ClassDB::bind_method(D_METHOD("unit_test"), &BArcLineCurve::unit_test);

    ADD_SIGNAL(MethodInfo(CoreStringNames::get_singleton()->changed));
}

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

void BArcLineCurve::unit_test() {

    {
        // trying to fix an assert bug
        /*
        a.out:-0.761644, 0, 0.647996
        b.in:0, 0, 0
        a:-50.493961, 0, 138.846207
        b:-62.670567, 0, 126.16227
        */

        add_point(Vector3(-50.493961, 0, 138.846207), Vector3(), Vector3(-0.761644, 0, 0.647996));
        add_point(Vector3(-62.670567, 0, 126.16227), Vector3(0, 0, 0), Vector3());

        _bake();
    }

    {
        // trying to fix the prezel bug - the loop that occurs in the track

        /*
        a.out:0.47992, 0, -0.877312
        b.in:-0.822768, 0, -0.568377
        a:-99.742065, 0, 160.144012
        b:-20.190918, 0, 60.912788
        */

        add_point(Vector3(-99.742065, 0, 160.144012), Vector3(), Vector3(0.47992, 0, -0.877312));
        add_point(Vector3(-20.190918, 0, 60.912788), Vector3(-0.822768, 0, -0.568377), Vector3());

        _bake();
        //assert(curve.size() == 1);
        //assert(curve[0].type == GPT_ARC);
        //clear_points();

        int nothnig = 0;
        ++nothnig;

    }
	return;
	
	{
		// test that when we get a transform, we can "undo" it 
		// this is needed for when we subdivide the track
		float percent = 0.25f;
		Vector3 offset = Vector3(0, 0, 0); //-121, 0, -19);		
		
		Vector3 p0 = Vector3(-radius_min, 0, 0) + offset;
		Vector3 d0 = Vector3(0, 0, 1);
		
		Vector3 p1 = Vector3(radius_min, 0, 0) + offset;
	
		BArcLineCurve::GeometricPrimitive prim;
		Vector3 arc_centre0 = prim.compute_best_arc_centre(radius_min, p0, d0, p1);
		prim.start_arc(arc_centre0, radius_min, p0, d0);
		prim.end_arc_at_tangent_pt(p1);

		Transform t = prim.get_transform(percent);
		
		// create a new arc centre from this mid point to ensure all is tidy
		BArcLineCurve::GeometricPrimitive prim_2  = BArcLineCurve::GeometricPrimitive::create(NULL, NULL);
		Vector3 arc_centre1 = prim_2.compute_best_arc_centre(radius_min, t.origin, -t.basis.get_axis(2), p1);
		assert(BUtil::is_equal_approx(arc_centre1, arc_centre0));
		
		clear_points();
	}
	
	
	{		
		// testing two points the lie on the same circle
		// should only result in a single curve
		
		// I have a quarter of a circle here
		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 d0 = Vector3(1, 0, 0);
		
		Vector3 p1 = Vector3(radius_min, 0, radius_min);
		Vector3 d1 = Vector3(0, 0, 1);
		
		add_point(p0, Vector3(), d0);
		add_point(p1, -d1, Vector3());
		
		_bake();
		assert(curve.size() == 1);
		assert(curve[0].type == GPT_ARC);
		clear_points();
	}
	
	{		
		// testing two points the lie on the same circle
		// should only result in a single curve
		
		// I have half a circle here
		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 d0 = Vector3(1, 0, 0);
		
		Vector3 p1 = Vector3(0, 0, radius_min * 2);
		Vector3 d1 = Vector3(-1, 0, 0);
		
		add_point(p0, Vector3(), d0);
		add_point(p1, -d1, Vector3());
		
		_bake();
		assert(curve.size() == 1);
		assert(curve[0].type == GPT_ARC);
		clear_points();
	}
	
	{		
		// testing tangents that lie on the line
		// should only result in a single line primitive
		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(0, 0, 10);
		Vector3 dir = Vector3(0, 0, 1);
		
		add_point(p0, Vector3(), dir);
		add_point(p1, -dir, Vector3());
		
		_bake();
		assert(curve.size() == 1);
		assert(curve[0].type == GPT_LINE);
		clear_points();
	}
		
			
	{
		// TODO: test reversing of in/out vectors
		
		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(0, 0, 10);
		
		add_point(Vector3(0, 0, 0));
		add_point(Vector3(0, 0, 10));
		Ref<BArcLineCurve> reversed = get_reversed();
		
		assert(BUtil::is_equal_approx(reversed->get_point_pos(0), p1));
		assert(BUtil::is_equal_approx(reversed->get_point_pos(1), p0));
		clear_points();
	}
		
	{
		add_point(Vector3(0, 0, 0));
		add_point(Vector3(0, 0, 10));
		float curve_length = get_baked_length();
		assert(Math::is_equal_approx(curve_length, 10));
		clear_points();
	}
	
	// testing get_sub_curve
	{
		add_point(Vector3(0, 0, 0));
		add_point(Vector3(0, 0, 100));
		//add_point(Vector3(100, 0, 100));
		float dist_to_grab = 10;
		Ref<BArcLineCurve> sub_curve = get_sub_curve(0, dist_to_grab);
		float curve_length = get_baked_length();
		float sub_curve_length = sub_curve->get_baked_length();
		assert(Math::is_equal_approx(sub_curve_length, dist_to_grab));
		clear_points();
	}
		
	{
		for (int i = 0; i < 1; ++i) {
			GeometricPrimitive circle = BArcLineCurve::GeometricPrimitive::create(NULL, NULL);
			Vector3 centre(10, 0, 10);
			float radius = 10;

			int dir = 1;
			float start_angle = 1.28; //-5.f; //Math::random(-2 * Math_PI, 2 * Math_PI);
			Vector3 start_tangent = centre + Vector3(Math::cos(start_angle), 0, Math::sin(start_angle)) * radius;
			Vector3 start_tangent_dir = Vector3(Math::cos(start_angle + (0.1 * dir)), 0, Math::sin(start_angle + (0.1 * dir))) * radius;
			circle.start_arc(centre, radius, start_tangent, start_tangent_dir);
			assert(circle.arc.arc_direction == dir);
			//assert(Math::is_equal_approx(circle.arc.angle_begin, start_angle)); this is right

			float end_angle = 2.6f; //Math::random(-2 * Math_PI, 2 * Math_PI);
			Vector3 end_tangent = centre + Vector3(Math::cos(end_angle), 0, Math::sin(end_angle)) * radius;
			circle.end_arc_at_tangent_pt(end_tangent);
			assert(Math::is_equal_approx(circle.arc.angle_begin + circle.arc.angle_arc * dir, end_angle));
			

			float percent = 0.5f;
			float percent_angle_between = ((end_angle - start_angle) * percent);
			float external_angle = start_angle + percent_angle_between;
			Vector3 external_pt_1 = centre + Vector3(Math::cos(external_angle), 0, Math::sin(external_angle)) * radius * 2.f;

			float angle_between = circle.compute_angle_between(start_tangent, external_pt_1, dir);
			assert(Math::is_equal_approx(angle_between, percent_angle_between));
			assert(Math::is_equal_approx((circle.arc.angle_begin + circle.arc.angle_arc * dir * percent), external_angle));
			
			Transform xform = circle.get_transform(CLAMP(percent, 0.f, 1.f));

			DistanceInfo dist_info;
			circle.closest_distance_to(external_pt_1, &dist_info);
			assert(Math::is_equal_approx(dist_info.percent, CLAMP(percent, 0.f, 1.f)));
			if (percent >= 0.f && percent <= 1.f) {
				assert(Math::is_equal_approx(dist_info.distance, radius));
			}

			assert(BUtil::is_equal_approx(xform.origin, dist_info.closest_point));
		}
		clear_points();
	}
	
	
	{
		// testing collision detection for lines
		GeometricPrimitive line = GeometricPrimitive::create(NULL, NULL);
		line.start_line(Vector3(0.f, 0.f, 0.f));
		line.end_line(Vector3(100.f, 0.f, 0.f));
		
		DistanceInfo dist_info_1;
		line.closest_distance_to(Vector3(0, 0, 10), &dist_info_1);
		assert(Math::is_equal_approx(dist_info_1.distance, 10.f));
		assert(Math::is_equal_approx(dist_info_1.percent, 0.f));
		
		DistanceInfo dist_info_2;
		line.closest_distance_to(Vector3(50, 0, -10), &dist_info_2);
		assert(Math::is_equal_approx(dist_info_2.distance, 10.f));
		assert(Math::is_equal_approx(dist_info_2.percent, 0.5f));

		DistanceInfo dist_info_3;
		line.closest_distance_to(Vector3(150, 0, 0), &dist_info_3);
		assert(Math::is_equal_approx(dist_info_3.distance, 50.f));
		assert(Math::is_equal_approx(dist_info_3.percent, 1.f));

		clear_points();		
	}
	
	{
		// test collision detection for circles
		// set up an arc from 0 to 90 degrees
		GeometricPrimitive circle = GeometricPrimitive::create(NULL, NULL);
		circle.start_arc(Vector3(), 1.f, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f));
		circle.end_arc_at_tangent_pt(Vector3(0.f, 0.f, 1.f));
		assert(circle.arc.arc_direction == 1);
				
		// 0 degrees
		DistanceInfo dist_info_1;
		circle.closest_distance_to(Vector3(10, 0, 0), &dist_info_1);
		assert(Math::is_equal_approx(dist_info_1.distance, 9.f));
		assert(Math::is_equal_approx(dist_info_1.percent, 0.f));
		
		// 45 degrees
		DistanceInfo dist_info_3;
		circle.closest_distance_to(Vector3(10, 0, 10), &dist_info_3);
		assert(Math::is_equal_approx(dist_info_3.distance, Vector3(10, 0, 10).length() - 1.f));
		assert(Math::is_equal_approx(dist_info_3.percent, 0.5f));
		
		// ensure what closest_distance_to correlates to get_transform
		Transform xform = circle.get_transform(0.5f);
		assert(BUtil::is_equal_approx(xform.origin, dist_info_3.closest_point));
		
		// 90 degrees
		DistanceInfo dist_info_2;
		circle.closest_distance_to(Vector3(0, 0, 10), &dist_info_2);
		assert(Math::is_equal_approx(dist_info_2.distance, 9.f));
		assert(Math::is_equal_approx(dist_info_2.percent, 1.f));
		
		// 180 degrees
		DistanceInfo dist_info_4;
		circle.closest_distance_to(Vector3(-10, 0, 0), &dist_info_4);
		float expected_dist_4 = (Vector3(-10, 0, 0) - Vector3(0.f, 0.f, 1.f)).length();
		assert(Math::is_equal_approx(dist_info_4.distance, expected_dist_4));
		assert(Math::is_equal_approx(dist_info_4.percent, 1.f));
		
		// 270 degrees
		DistanceInfo dist_info_5;
		circle.closest_distance_to(Vector3(0, 0, -10), &dist_info_5);
		float expected_dist_5 = (Vector3(0, 0, -10) - Vector3(1.f, 0.f, 0.f)).length();
		assert(Math::is_equal_approx(dist_info_5.distance, expected_dist_5));
		assert(Math::is_equal_approx(dist_info_5.percent, 0.f));
		
		
		// test collision detection for circles
		// set up an arc from 0 to -90 degrees
		GeometricPrimitive circle_cw = GeometricPrimitive::create(NULL, NULL);
		circle_cw.start_arc(Vector3(), 1.f, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, -1.f));
		circle_cw.end_arc_at_tangent_pt(Vector3(0.f, 0.f, -1.f));
		assert(circle_cw.arc.arc_direction == -1);
		
		// 0 degrees
		DistanceInfo dist_info_10;
		circle_cw.closest_distance_to(Vector3(10, 0, 0), &dist_info_10);
		assert(Math::is_equal_approx(dist_info_10.distance, 9.f));
		assert(Math::is_equal_approx(dist_info_10.percent, 0.f));
		Transform xform_10 = circle.get_transform(0.f);
		assert(BUtil::is_equal_approx(xform_10.origin, dist_info_10.closest_point));
		
		// 45 degrees
		DistanceInfo dist_info_30;
		circle_cw.closest_distance_to(Vector3(10, 0, -10), &dist_info_30);
		assert(Math::is_equal_approx(dist_info_30.distance, Vector3(10, 0, 10).length() - 1.f));
		assert(Math::is_equal_approx(dist_info_30.percent, 0.5f));
		Transform xform_30 = circle_cw.get_transform(0.5f);
		assert(BUtil::is_equal_approx(xform_30.origin, dist_info_30.closest_point));
		
		// 90 degrees
		DistanceInfo dist_info_20;
		circle_cw.closest_distance_to(Vector3(0, 0, -10), &dist_info_20);
		assert(Math::is_equal_approx(dist_info_20.distance, 9.f));
		assert(Math::is_equal_approx(dist_info_20.percent, 1.f));
		Transform xform_20 = circle_cw.get_transform(1.f);
		assert(BUtil::is_equal_approx(xform_20.origin, dist_info_20.closest_point));
		
		// 180 degrees
		DistanceInfo dist_info_40;
		circle_cw.closest_distance_to(Vector3(-10, 0, 0), &dist_info_40);
		float expected_dist_40 = (Vector3(-10, 0, 0) - Vector3(0.f, 0.f, 1.f)).length();
		assert(Math::is_equal_approx(dist_info_40.distance, expected_dist_40));
		assert(Math::is_equal_approx(dist_info_40.percent, 1.f));
		
		// 270 degrees
		DistanceInfo dist_info_50;
		circle_cw.closest_distance_to(Vector3(0, 0, 10), &dist_info_50);
		float expected_dist_50 = (Vector3(0, 0, 10) - Vector3(1.f, 0.f, 0.f)).length();
		assert(Math::is_equal_approx(dist_info_50.distance, expected_dist_50));
		assert(Math::is_equal_approx(dist_info_50.percent, 0.f));
		
		clear_points();
	}
		
	{
		// test collision detection
		add_point(Vector3(0, 0, 0), Vector3(), Vector3(0, 0, 1)); // clockwise if we look down on this
		add_point(Vector3(100, 0, 0));

		Dictionary d_1 = closest_distance_to(Vector3(10, 0, 0));
		Dictionary d_2 = closest_distance_to(Vector3(50, 0, 0));		
		
		clear_points();
	}
	
	{
		// testing compute_arc_angle
		
		GeometricPrimitive circle = GeometricPrimitive::create(NULL, NULL);
		circle.start_arc(Vector3(), 1.f, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f));
		float angle_0 = circle.compute_arc_angle(Vector3(1.f, 0.f, 0.f));
		assert(Math::is_equal_approx(angle_0, 0.f));
		
		float angle_90 = circle.compute_arc_angle(Vector3(0.f, 0.f, 1.f));
		assert(Math::is_equal_approx(angle_90, Math_PI * 0.5f));
		
		float angle_180 = circle.compute_arc_angle(Vector3(-1.f, 0.f, 0.f));
		assert(Math::is_equal_approx(angle_180, Math_PI));
		
		float angle_270 = circle.compute_arc_angle(Vector3(0.f, 0.f, -1.f));
		assert(Math::is_equal_approx(angle_270, Math_PI * 1.5f));
		
		float angle_0_to_90_ccw = circle.compute_angle_between(Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), 1); // CCW
		assert(Math::is_equal_approx(angle_0_to_90_ccw, Math_PI * 0.5f));
		
		float angle_0_to_270_ccw = circle.compute_angle_between(Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, -1.f), 1); // CCW
		assert(Math::is_equal_approx(angle_0_to_270_ccw, Math_PI * 1.5f));
		
		float angle_0_to_90_cw = circle.compute_angle_between(Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), -1); // CW
		assert(Math::is_equal_approx(angle_0_to_90_cw, Math_PI * 1.5f));
		
		float angle_0_to_270_cw = circle.compute_angle_between(Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, -1.f), -1); // CW
		assert(Math::is_equal_approx(angle_0_to_270_cw, Math_PI * 0.5f));
		
		
		float angle_90_to_0_ccw = circle.compute_angle_between(Vector3(0.f, 0.f, 1.f), Vector3(1.f, 0.f, 0.f), 1); // CCW
		assert(Math::is_equal_approx(angle_90_to_0_ccw, Math_PI * 1.5f));
		
		float angle_270_to_0_ccw = circle.compute_angle_between(Vector3(0.f, 0.f, -1.f), Vector3(1.f, 0.f, 0.f), 1); // CCW
		assert(Math::is_equal_approx(angle_270_to_0_ccw, Math_PI * 0.5f));
		
		float angle_90_to_0_cw = circle.compute_angle_between(Vector3(0.f, 0.f, 1.f), Vector3(1.f, 0.f, 0.f), -1); // CW
		assert(Math::is_equal_approx(angle_90_to_0_cw, Math_PI * 0.5f));
		
		float angle_270_to_0_cw = circle.compute_angle_between(Vector3(0.f, 0.f, -1.f), Vector3(1.f, 0.f, 0.f), -1); // CW
		assert(Math::is_equal_approx(angle_270_to_0_cw, Math_PI * 1.5f));
		
		clear_points();
	}
	
	{
		// test with input tangent and no output tangent
		add_point(Vector3(0, 0, 0), Vector3(), Vector3(0, 0, 1)); // clockwise if we look down on this
		add_point(Vector3(100, 0, 0));
		_bake();
		Transform t0 = get_transform_from_distance_along_curve(0);
		Transform t1 = get_transform_from_distance_along_curve(get_baked_length());
		clear_points();
	}
	
	{
		// test with input tangent and no output tangent
		add_point(Vector3(0, 0, 0), Vector3(), Vector3(0, 0, -1)); // counter clockwise if we look down on this
		add_point(Vector3(100, 0, 0));
		_bake();
		Transform t0 = get_transform_from_distance_along_curve(0);
		Transform t1 = get_transform_from_distance_along_curve(get_baked_length());
		clear_points();
	}
	
	{
		// test a point on top of itself
		add_point(Vector3(-93, 5, 15));
		add_point(Vector3(-93, 5, 15), Vector3(0, -8, -1));
		_bake();
		Transform t0 = get_transform_from_distance_along_curve(0);
		Transform t1 = get_transform_from_distance_along_curve(get_baked_length());
		clear_points();
	}
	
	{
		// test with no input tangent and output tangent
		add_point(Vector3(0, 0, 0));
		add_point(Vector3(100, 0, 0), Vector3(0, 0, 1));
		_bake();
		Transform t0 = get_transform_from_distance_along_curve(0);
		Transform t1 = get_transform_from_distance_along_curve(get_baked_length());
		clear_points();
	}
	
	{
		// test with no tangents specified
		add_point(Vector3(0, 0, 0));
		add_point(Vector3(100, 0, 0));
		_bake();
		Transform t0 = get_transform_from_distance_along_curve(0);
		Transform t1 = get_transform_from_distance_along_curve(get_baked_length());
		clear_points();
	}
	
	{
		// test with tangents pointing the same way
		add_point(Vector3(0, 0, 0), Vector3(), Vector3(0, 0, 1));
		add_point(Vector3(100, 0, 0), Vector3(0, 0, 1));
		_bake();
		Transform t0 = get_transform_from_distance_along_curve(0);
		Transform t1 = get_transform_from_distance_along_curve(get_baked_length());
		clear_points();
	}
	
	{
		// test with tangents pointing the opposite way
		add_point(Vector3(0, 0, 0), Vector3(), Vector3(0, 0, 1));
		add_point(Vector3(100, 0, 0), Vector3(0, 0, 1));
		_bake();
		Transform t0 = get_transform_from_distance_along_curve(0);
		Transform t1 = get_transform_from_distance_along_curve(get_baked_length());
		clear_points();
	}
	
	{
		// test with the distance to close to each other
	}
}

#pragma GCC diagnostic pop

BArcLineCurve::BArcLineCurve() {
    radius_min = 10.0;
	baked_cache_dirty = false;
	local = false;
}

BArcLineCurve::~BArcLineCurve() {
}
