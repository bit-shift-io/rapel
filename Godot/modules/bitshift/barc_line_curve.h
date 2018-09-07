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
#ifndef BARC_LINE_H
#define BARC_LINE_H

#include "core/reference.h"
#include "core/vector.h"
#include <assert.h>


/**
	@author Fabian Mathews <supagu@gmail.com>
 
 A curve made of arcs and lines
*/

class BArcLineCurve : public Reference  {

    GDCLASS(BArcLineCurve, Reference);
    
	enum GeometricPrimitiveType {
		GPT_LINE,
		GPT_ARC
	};
	
	struct Point {

		Vector3 in;
		Vector3 out;
		Vector3 pos;
		float tilt;

		Point() { tilt = 0; }
	};
		
	struct Arc {
		float centre[3];
		float radius;
		float angle_begin;	// angle of beginning point
		float angle_arc;	// the absolute angle between beginning pt and end pt
		int arc_direction;	// 1 = counterclockwise, -1 clockwise, 0 = unknown
		
		float begin[3];
		float end[3];
		
		Vector3 get_centre() { return Vector3(centre[0], centre[1], centre[2]); }
		Vector3 get_begin() { return Vector3(begin[0], begin[1], begin[2]); }
		Vector3 get_end() { return Vector3(end[0], end[1], end[2]); }
	};
	
	struct Line {
		float begin[3];
		float end[3];
		
		Vector3 get_begin() { return Vector3(begin[0], begin[1], begin[2]); }
		Vector3 get_end() { return Vector3(end[0], end[1], end[2]); }
	};
	
	struct DistanceInfo {
		Vector3 closest_point;
		float percent;	// length along the GeometricPrimitive
		float distance; // length from closest_point to the input position. can we make this length squared?
	};
	
	struct GeometricPrimitive {
		union {
			Arc arc;
			Line line;
		};
		
		GeometricPrimitiveType type;
		float length;
        AABB aabb;
		
		const Point* points[2]; // which two points is this primitive generated from?
		
		static GeometricPrimitive create(const Point* p_a, const Point* p_b);
		
		void start_arc(const Vector3 &p_centre, float p_radius, const Vector3& p_start_tangent_point, const Vector3& p_start_tangent_dir);
		//void end_arc(const Vector3& p_end_tangent_point_a, const Vector3& p_end_tangent_point_b);
		int end_arc_from_external_pt(const Vector3 &p_point);
		void end_arc_at_tangent_pt(const Vector3 &p_tangent_point);
		
		void start_line(const Vector3 &p_start);
		void end_line(const Vector3 &p_end_a);
		
		// reverse this shape, the start becomes the end, the end becomes the start
		void reverse();
		
		float compute_arc_angle(const Vector3 &p_tangent_point);
		Vector3 compute_best_arc_centre(float radius, const Vector3 &p_tangent_point, const Vector3 &p_tangent, const Vector3 &p_target_point);
		int compute_arc_tangent_points_from_external_point(const Vector3 &p_external_point, Vector3 *p_a, Vector3 *p_b);
		int compute_tangent_point_direction(const Vector3 &p_tangent_point, const Vector3 &p_tangent_dir);
		float compute_angle_between(const Vector3 &p_tangent_a, const Vector3 &p_tangent_b, int direction);
		
		void add_to_curve(const BArcLineCurve& curve);
		
		Transform get_transform(float p_percentage);
		bool closest_distance_to(const Vector3& p_pos, DistanceInfo* p_dist_info, float p_distance_max = -1.f);
		
		Vector3 get_closest_point_to_segment(const Vector3 &p_point, const Vector3 *p_segment, float* p_percent);
	};
	
	mutable Vector<GeometricPrimitive> curve;
    mutable AABB aabb;
	
	Vector<Point> points;
	float radius_min;
	//Transform global_transform;
	
	mutable bool baked_cache_dirty;
	
    void _bake();
    void _bake_segment(int i, Point* p_a, Point* p_b); //const Vector3 &p_a, const Vector3 &p_out, const Vector3 &p_b, const Vector3 &p_in) const;
	
    //Transform modify_transform(const Transform& t);
	
	void validate_points();

    void _dirty();
		
protected:

	static void _bind_methods();
	
public:
	
	void apply_transform_to_points(const Transform& p_transform);
	
	int get_point_count() const;

	// given a point 
	// we have an p_out tangent, which is the forward tangent of the line towards the next point
	// but we also have the reverse tangent p_in which should likely just be -p_out
	// it is the back tangent when a point connects to this point
    //
    // NOTE: add_point and set_point_pos can modify the point position, the prime case for this is if the point is inside the circle
    // it is advised to call get_point_pos once add_point has been called
	int add_point(const Vector3 &p_pos, const Vector3 &p_in = Vector3(), const Vector3 &p_out = Vector3(), int p_atpos = -1);
	void set_point_pos(int p_index, const Vector3 &p_pos);
	Vector3 get_point_pos(int p_index) const;
	
	void set_point_in(int p_index, const Vector3 &p_in);
	Vector3 get_point_in(int p_index) const;
	void set_point_out(int p_index, const Vector3 &p_out);
	Vector3 get_point_out(int p_index) const;
	
	void remove_point(int p_index);
	void clear_points();
	
    float get_baked_length();
	
	Transform get_transform_from_distance_along_curve(float p_offset);
	PoolVector3Array tesselate_along_terrain(float p_interval, float p_push_along_normal_distance);
	
	// given a point, and some distance tolerance we want to be in
	// return a distance along the spline
	// -v distance tolerance indicates we ignore distance checking
    Dictionary closest_distance_to(const Vector3& p_pos, float p_distance_max = -1.f);
	
	//void set_global_transform(const Transform& transform);
	//Transform get_global_transform() const;
			
	void copy(const Ref<BArcLineCurve>& other);
	
	void unit_test();
	
	// network replication
	Variant pack() const;
	void unpack(const Variant& variant);
	
	// given a list of distances along this curve, use them to chop this curve up into a series of smaller curves
    Array subdivide(const Array& distances_along_curve);
	
	// given a start and end distance, return a new curve between those
	// two points
    Ref<BArcLineCurve> get_sub_curve(float start, float end);
	
	Ref<BArcLineCurve> get_reversed();

    float get_radius() { return radius_min; }
    void set_radius(float radius);
	
    BArcLineCurve();
    ~BArcLineCurve();    
	
	bool local;
};

#endif // BARC_LINE_H
