// ======================================================================
//
// Copyright (c) 1997 The CGAL Consortium
//
// This software and related documentation is part of an INTERNAL release
// of the Computational Geometry Algorithms Library (CGAL). It is not
// intended for general use.
//
// ----------------------------------------------------------------------
//
// release       : $$
// release_date  : $$
//
// file          : include/CGAL/Pm_segment_traits_2.h
// package       : Planar_map (5.87)
// maintainer    : Eyal Flato        <flato@math.tau.ac.il>
// source        : 
// revision      : 
// revision_date : 
// author(s)     : Efi Fogel         <efif@post.tau.ac.il>
//                 Ron Wein          <wein@post.tau.ac.il>
//
// coordinator   : Tel-Aviv University (Dan Halperin <halperin@math.tau.ac.il>)
//
// Chapter       : 
// ======================================================================
#ifndef CGAL_ARR_SEGMENT_CACHED_TRAITS_2_H
#define CGAL_ARR_SEGMENT_CACHED_TRAITS_2_H

#include <CGAL/tags.h>
#include <CGAL/intersections.h>

#include <list>
#include <fstream>

CGAL_BEGIN_NAMESPACE

template <class Kernel_> class Segment_cached_2;

/*!
 * A traits class for maintaining an arrangement of segments, aoviding cascading
 * of computations as much as possible.
 */
template <class Kernel_>
class Arr_segment_cached_traits_2 : public Kernel_
{
public:
  typedef Kernel_                         Kernel;

  typedef Tag_true                        Has_left_category;

  /*!
   * Representation of a segement with cached data.
   */
  class My_segment_cached_2
  {
    typedef typename Kernel_::Segment_2             Segment_2;
    typedef typename Kernel_::Point_2               Point_2;

  protected:

    bool      is_orig;          // Is this an original segment.
    Segment_2 orig_seg;         // The original segment.
    Point_2   ps, pt;           // The source a target points.
    bool      is_vert;          // Is this a vertical segment.

    /*!
     * Default constructor.
     */
    My_segment_cached_2 () :
      is_orig(true),
      orig_seg(),
      is_vert(-1)
    {}

    /*!
     * Constructor from a segment.
     * \param seg The segment.
     */
    My_segment_cached_2 (const Segment_2& seg) :
      is_orig(true),
      orig_seg(seg)
    {
      Kernel_   kernel;
      
      is_vert = kernel.is_vertical_2_object()(seg);
      
      typename Kernel_::Construct_vertex_2 
	construct_vertex = kernel.construct_vertex_2_object();

      ps = construct_vertex(seg, 0);
      pt = construct_vertex(seg, 1);
    }

    /*!
     * Construct a segment from two end-points.
     * \param source The source point.
     * \param target The target point.
     */
    My_segment_cached_2 (const Point_2& source, const Point_2& target) :
      is_orig(true),
      orig_seg(source,target)
    {
      Kernel_   kernel;
      
      is_vert = kernel.is_vertical_2_object()(orig_seg);
      
      ps = source;
      pt = target;
    }

    friend class Arr_segment_cached_traits_2;
  };

  // Traits objects
  typedef typename Kernel::Point_2        Point_2;
  typedef Segment_cached_2<Kernel>        X_curve_2;
  typedef Segment_cached_2<Kernel>        Curve_2;

  // Backward compatability    
  typedef Point_2                         Point;
  typedef X_curve_2                       X_curve;
  typedef X_curve_2                       Curve;

  // Currently, I leave this in the traits
  // Maybe we can change the usage inside Planar_map_2
  typedef enum
  {
    UNDER_CURVE        = -1,
    CURVE_NOT_IN_RANGE =  0,
    ABOVE_CURVE        =  1,
    ON_CURVE           =  2

  } Curve_point_status;	

protected:

  // Functors:
  typedef typename Kernel::Is_vertical_2        Is_vertical_2;
  typedef typename Kernel::Construct_vertex_2   Construct_vertex_2;
  typedef typename Kernel::Less_x_2             Less_x_2;
  typedef typename Kernel::Equal_2              Equal_2;
  typedef typename Kernel::Compare_x_2          Compare_x_2;
  typedef typename Kernel::Compare_xy_2         Compare_xy_2;
  typedef typename Kernel::Compare_slope_2      Compare_slope_2;
  typedef typename Kernel::Orientation_2        Orientation_2;
    
public:

  /*!
   * Defalut constructor.
   */
  Arr_segment_cached_traits_2() {}

  // Operations
  // ----------
    
  /*!
   * Compare the x-coordinates of two given points.
   * \param p1 The first point.
   * \param p2 The second point.
   * \return LARGER if x(p1) > x(p2), SMALLER if x(p1) < x(p2), or else EQUAL.
   */
  Comparison_result compare_x(const Point_2 & p1, const Point_2 & p2) const
  {
    return compare_x_2_object()(p1, p2);
  }

  /*! 
   * Compares lexigoraphically the two points: by x, then by y.
   * \param p1 Te first point.
   * \param p2 The second point.
   * \return LARGER if x(p1) > x(p2), or if x(p1) = x(p2) and y(p1) > y(p2); 
   *         SMALLER if x(p1) < x(p2), or if x(p1) = x(p2) and y(p1) < y(p2);
   *         or else EQUAL.
   */
  Comparison_result compare_xy(const Point_2 & p1, const Point_2 & p2) const
  {
    return compare_xy_2_object()(p1, p2);
  }

  /*!
   * Check whether the given curve is a vertical segment.
   * \param cv The curve.
   * \return (true) if the curve is vertical.
   */
  bool curve_is_vertical(const X_curve_2 & cv) const 
  {
    return (cv.is_vert);
  } 

  /*!
   * Check whether the given point is in the x-range of the given curve.
   * In out case, the curve is a segment [s, t], check whether x(s)<=x(q)<=x(t)
   * or whether x(t)<=x(q)<=x(s).
   * \param cv The curve.
   * \param q The point.
   * \return (true) if q is in the x-range of cv.
   */
  bool curve_is_in_x_range(const X_curve_2 & cv, const Point_2 & q) const
  {
    Compare_x_2       compare_x = compare_x_2_object();
    Comparison_result res1 = compare_x (q, cv.ps);
    Comparison_result res2 = compare_x (q, cv.pt);

    // We check if x(p) equals the x value of one of the end-points.
    // If not, we check whether one end-point is to p's left and the other is
    // to its right.
    return ((res1 == EQUAL) || (res2 == EQUAL) ||
	    (res1 != res2));
  }

  /*!
   * Get the relative status of two curves at the x-coordinate of a given 
   * point.
   * \param cv1 The first curve.
   * \param cv2 The second curve.
   * \param q The point.
   * \pre The point q is in the x-range of the two curves.
   * \return LARGER if cv1(x(q)) > cv2(x(q)); SMALLER if cv1(x(q)) < cv2(x(q);
   *  or else EQUAL.
   */
  Comparison_result curve_compare_at_x(const X_curve_2 & cv1, 
				       const X_curve_2 & cv2, 
				       const Point_2 & q) const
  {
    CGAL_precondition(curve_is_in_x_range(cv1, q));
    CGAL_precondition(curve_is_in_x_range(cv2, q));

    // Compare using the original segments.
    return (compare_y_at_x_2_object()(q, cv1.orig_seg, cv2.orig_seg));
  }

  /*!
   * Compares the y value of two curves in an epsilon environment to the left
   * of the x-value of the input point.
   * \param cv1 The first curve.
   * \param cv2 The second curve.
   * \param q The point.
   * \pre The point q is in the x range of the two curves, and both of them 
   * must be also be defined to its left.
   * \return The relative position of cv1 with respect to cv2 to the left of
   * x(q): LARGER, SMALLER or EQUAL.
   */
  Comparison_result curve_compare_at_x_left(const X_curve_2 & cv1,
                                            const X_curve_2 & cv2, 
                                            const Point_2 & q) const 
  {
    // The two curves must not be vertical.
    CGAL_precondition(! cv1.is_vert);
    CGAL_precondition(! cv2.is_vert);

    // The two curve must be defined at q and also to its left.
    CGAL_precondition_code(
	Less_x_2 less_x = less_x_2_object();
	);

    CGAL_precondition (less_x(cv1.ps, q) || less_x(cv1.pt, q));
    CGAL_precondition (!(less_x(cv1.ps, q) && less_x(cv1.pt, q)));
    
    CGAL_precondition (less_x(cv2.ps, q) || less_x(cv2.pt, q));
    CGAL_precondition (!(less_x(cv2.ps, q) && less_x(cv2.pt, q)));
    
    // Since the curves are continuous, if they are not equal at q, the same
    // result also applies to q's left.
    Comparison_result res = 
      compare_y_at_x_2_object()(q, cv1.orig_seg, cv2.orig_seg);

    if (res != EQUAL)
      return (res);     
    
    // <cv2> and <cv1> meet at a point with the same x-coordinate as q
    // compare their derivatives.
    // Notice we use the original segments in order to compare the slopes.
    return (compare_slope_2_object()(cv2.orig_seg, cv1.orig_seg));
  }

  /*!
   * Compares the y value of two curves in an epsilon environment to the right
   * of the x-value of the input point.
   * \param cv1 The first curve.
   * \param cv2 The second curve.
   * \param q The point.
   * \pre The point q is in the x range of the two curves, and both of them 
   * must be also be defined to its right.
   * \return The relative position of cv1 with respect to cv2 to the right of
   * x(q): LARGER, SMALLER or EQUAL.
   */
  Comparison_result curve_compare_at_x_right(const X_curve_2 & cv1,
                                             const X_curve_2 & cv2, 
                                             const Point_2 & q) const 
  {
    // The two curves must not be vertical.
    CGAL_precondition(! cv1.is_vert);
    CGAL_precondition(! cv2.is_vert);

    // The two curve must be defined at q and also to its right.
    CGAL_precondition_code(Less_x_2 less_x = less_x_2_object(););

    CGAL_precondition (less_x(q, cv1.ps) || less_x(q, cv1.pt));
    CGAL_precondition (!(less_x(q, cv1.ps) && less_x(q, cv1.pt)));
    
    CGAL_precondition (less_x(q, cv2.ps) || less_x(q, cv2.pt));
    CGAL_precondition (!(less_x(q, cv2.ps) && less_x(q, cv2.pt)));
    
    // Since the curves are continuous, if they are not equal at q, the same
    // result also applies to q's right.
    Comparison_result res =
      compare_y_at_x_2_object()(q, cv1.orig_seg, cv2.orig_seg);

    if (res != EQUAL)
      return (res);     
    
    // <cv1> and <cv2> meet at a point with the same x-coordinate as q
    // compare their derivatives
    // Notice we use the original segments in order to compare the slopes.
    return (compare_slope_2_object()(cv1.orig_seg, cv2.orig_seg));
  }
    
  /*! 
   * Return the location of the given point with respect to the input curve.
   * \param cv The curve.
   * \param p The point.
   * \return CURVE_NOT_IN_RANGE if p is not in the x-range of cv;
   *         ABOVE_CURVE if cv(x(p)) < y(p);
   *         UNDER_CURVE if cv(x(p)) > y(p);
   *         or else ON_CURVE (if p is on the curve).
   */
  Curve_point_status curve_get_point_status (const X_curve_2& cv, 
					     const Point_2& p) const
  {
    if (! curve_is_in_x_range(cv, p))
      return (CURVE_NOT_IN_RANGE);

    if (! cv.is_vert)
    {
      // Compare with the original segment.
      Comparison_result res = compare_y_at_x_2_object()(p, cv.orig_seg);
      return ((res == LARGER) ? ABOVE_CURVE :
	      ((res == SMALLER) ? UNDER_CURVE : ON_CURVE));
    }
    else
    {
      // Compare with the vertical segment's end-points.
      Compare_xy_2      compare_xy = compare_xy_2_object();
      Comparison_result res1 = compare_xy (p, cv.ps);
      Comparison_result res2 = compare_xy (p, cv.pt);
      
      if (res1 == LARGER && res2 == LARGER)
	return (ABOVE_CURVE);
      else if (res1 == SMALLER && res2 == SMALLER)
	return (UNDER_CURVE);
      else
	return (ON_CURVE);
    }
  }

  /*! 
   * Check if the given query segment is encountered when rotating the first
   * segment in a clockwise direction around a given point until reaching the
   * segment curve.
   * \param cv The query segment.
   * \param cv1 The first segment.
   * \param cv2 The second segment.
   * \param p The point around which we rotate cv1.
   * \pre p is an end-point of all three segments.
   * \return (true) if cv is between cv1 and cv2. If cv overlaps cv1 or cv2
   * the result is always (false). If cv1 and cv2 overlap, the result is (true),
   * unless cv1 also overlaps them.
   */
  bool curve_is_between_cw(const X_curve_2& cv, 
                           const X_curve_2& cv1, 
                           const X_curve_2& cv2, 
                           const Point_2& p) const
  {
    // Find the direction of each segment.
    Segment_dir     dir = _curve_direction(cv, p);
    Segment_dir     dir1 = _curve_direction(cv1, p);
    Segment_dir     dir2 = _curve_direction(cv2, p);

    // Special treatment for the cases where cv1 or cv2 are vertical segments:
    if (cv1.is_vert)
    {
      if (cv2.is_vert)
      {
	// Both cv1 and cv2 are vertical:
	if (dir1 == DIR_UP && dir2 == DIR_DOWN)
	  return (dir == DIR_RIGHT);
	else if (dir1 == DIR_DOWN && dir2 == DIR_UP)
	  return (dir == DIR_LEFT);
	else
	  return (dir != dir1);
      }

      // Only cv1 is vertical:
      if (dir1 == DIR_UP)
      {
	if (dir2 == DIR_LEFT)
	  return (dir == DIR_RIGHT ||
		  dir == DIR_DOWN ||
                  (dir == DIR_LEFT &&
                   _curve_compare_slope_left (cv2, cv) == LARGER));
	else
	  return (dir == DIR_RIGHT &&
		  _curve_compare_slope_right (cv2, cv) == SMALLER);
      }
      else
      {
	if (dir2 == DIR_LEFT)
	  return (dir == DIR_LEFT &&
		  _curve_compare_slope_left (cv2, cv) == LARGER);
	else
	  return (dir == DIR_LEFT ||
		  dir == DIR_UP ||
                  (dir == DIR_RIGHT &&
                   _curve_compare_slope_right (cv2, cv) == SMALLER));
      }
    }

    if (cv2.is_vert)
    {
      // Only cv2 is vertical:
      if (dir2 == DIR_UP)
      {
	if (dir1 == DIR_LEFT)
	  return (dir == DIR_LEFT &&
		  _curve_compare_slope_left (cv1, cv) == SMALLER);
	else
	  return (dir == DIR_LEFT || 
		  dir == DIR_DOWN ||
                  (dir == DIR_RIGHT &&
                   _curve_compare_slope_right (cv1, cv) == LARGER));
      }
      else
      {
	if (dir1 == DIR_LEFT)
	  return (dir == DIR_RIGHT ||
		  dir == DIR_UP ||
                  (dir == DIR_LEFT &&
                   _curve_compare_slope_left (cv1, cv) == SMALLER));
	else
	  return (dir == DIR_RIGHT &&
		  _curve_compare_slope_left (cv1, cv) == LARGER);
      }
    }

    // Take care of the general 4 cases:
    if (dir1 == DIR_LEFT && dir2 == DIR_LEFT)
    {
      // Case 1: Both cv1 and cv2 are defined to the left of p.
      Comparison_result l_res = _curve_compare_slope_left (cv1, cv2);
      
      if (l_res == LARGER)
      {
	// Case 1(a) : cv1 is above cv2.
	return (dir != DIR_LEFT ||
		_curve_compare_slope_left (cv1, cv) == SMALLER ||
                _curve_compare_slope_left (cv2, cv) == LARGER);
      }
      else if (l_res == SMALLER)
      {
	// Case 1(b): cv1 is below cv2.
	return (dir == DIR_LEFT &&
		_curve_compare_slope_left (cv1, cv) == SMALLER &&
		_curve_compare_slope_left (cv2, cv) == LARGER);
      }
      else
      {
        // Overlapping segments.
        return (dir != DIR_LEFT ||
                _curve_compare_slope_left (cv1, cv) != EQUAL);
      }
    }
    else if (dir1 == DIR_RIGHT && dir2 == DIR_RIGHT)
    {
      // Case 2: Both cv1 and cv2 are defined to the right of p.
      Comparison_result r_res = _curve_compare_slope_right (cv1, cv2);

      if (r_res == LARGER)
      {
	// Case 2(a) : cv1 is above cv2.
	return (dir == DIR_RIGHT &&
		_curve_compare_slope_right (cv1, cv) == LARGER &&
		_curve_compare_slope_right (cv2, cv) == SMALLER);
      }
      else if (r_res == SMALLER)
      {
	// Case 2(b): cv1 is below cv2.
	return (dir != DIR_RIGHT ||
		_curve_compare_slope_right (cv1, cv) == LARGER ||
                _curve_compare_slope_right (cv2, cv) == SMALLER);
      }
      else
      {
        // Overlapping segments.
        return (dir != DIR_RIGHT ||
                _curve_compare_slope_right (cv1, cv) != EQUAL);
      }
    }
    else if (dir1 == DIR_LEFT && dir2 == DIR_RIGHT)
    {
      // Case 3: cv1 is defined to the left of p, and cv2 to its right.
      return ((dir == DIR_LEFT &&
	       _curve_compare_slope_left (cv1, cv) == SMALLER) ||
	      (dir == DIR_RIGHT &&
	       _curve_compare_slope_right (cv2, cv) == SMALLER) ||
	      dir == DIR_UP);
    }
    else
    {
      // Case 4: cv1 is defined to the right of p, and cv2 to its leftt.
      return ((dir == DIR_RIGHT &&
	       _curve_compare_slope_right (cv1, cv) == LARGER) ||
	      (dir == DIR_LEFT &&
	       _curve_compare_slope_left (cv2, cv) == LARGER) ||
	      dir == DIR_DOWN);
    }
  }

  /*! 
   * Check if the two curves are the same (have the same graph).
   * \param cv1 The first curve.
   * \param cv2 The second curve.
   * \return (true) if the two curves are the same.
   */
  bool curve_is_same(const X_curve_2 & cv1,const X_curve_2 & cv2) const
  {
    Equal_2 equal = equal_2_object();
    return ((equal(cv1.ps, cv2.ps) && equal(cv1.pt, cv2.pt)) ||
	    (equal(cv1.ps, cv2.pt) && equal(cv1.pt, cv2.ps)));
  }

  /*!
   * Check if the two points are the same.
   * \param p1 The first point.
   * \param p2 The second point.
   * \return (true) if p1 == p2.
   */
  bool point_is_same(const Point_2 & p1, const Point_2 & p2) const
  {
    return equal_2_object()(p1, p2);
  }
  
  /*!
   * Get the curve source.
   * \param cv The curve.
   * \return The source point.
   */
  const Point_2& curve_source(const X_curve_2 & cv) const 
  { 
    return (cv.ps);
  }

  /*!
   * Get the curve target.
   * \param cv The curve.
   * \return The target point.
   */
  const Point_2& curve_target(const X_curve_2 & cv) const 
  { 
    return (cv.pt);
  }

  /*!
   * Check whether the curve is x-monotone.
   * \param cv The curves.
   * \return (true) if the curve is x-monotone. In case of segments, the
   * function always returns (true), since all segments are x-monotone. 
   * Vertical segments are also considered as 'weakly' x-monotone.
   */
  bool is_x_monotone(const Curve_2 &) const
  {
    // Return true, since a sgement is always x-monotone.
    return (true);
  }
  
  /*!
   * Split the given curve, to x-monotone sub-curves.
   * In case of segments does nothing, since a sgement is always x-monotone.
   * \param cv The curve.
   * \param x_curves A list of the output x-monotone sub-curves.
   * \pre cv is not x-monotone (and not a vertical segment).
   */
  void make_x_monotone(const Curve_2&, std::list<Curve_2>& ) const
  {} 

  /*!
   * Flip a given curve.
   * \param cv The input curve.
   * \return The flipped curve. In case of segments, if the input is [s,t],
   * then the flipped curve is simply [t,s].
   */
  X_curve_2 curve_flip(const X_curve_2 & cv) const
  {
    X_curve_2 flip_cv(cv);

    flip_cv.ps = cv.pt;
    flip_cv.pt = cv.ps;
    return (flip_cv);
  }

  /*!
   * Split a given curve at a given split point into two sub-curves.
   * \param cv the curve to split
   * \param c1 the output first part of the split curve. Its source is the
   * source of the original curve.
   * \param c2 the output second part of the split curve. Its target is the
   * target of the original curve.
   * \param p the split point.
   * \pre p lies on cv but is not one of its end-points.
   */
  void curve_split(const X_curve_2& cv, 
		   X_curve_2& c1, X_curve_2& c2, 
                   const Point_2& p) const
  {
    // Check preconditions.
    CGAL_precondition(curve_get_point_status(cv, p) == ON_CURVE);
    CGAL_precondition_code(Compare_xy_2 compare_xy = compare_xy_2_object());
    CGAL_precondition(compare_xy(cv.ps, p) != EQUAL);
    CGAL_precondition(compare_xy(cv.pt, p) != EQUAL);
    
    // Do the split.
    c1.is_orig = false;
    c1.orig_seg = cv.orig_seg;
    c1.ps = cv.ps;
    c1.pt = p;
    c1.is_vert = cv.is_vert;

    c2.is_orig = false;
    c2.orig_seg = cv.orig_seg;
    c2.ps = p;
    c2.pt = cv.pt;
    c2.is_vert = cv.is_vert;

    return;
  }

  /*! 
   * Check whether the two given curves intersect at a point to the right of
   * a given reference point.
   * \param cv1 The first curve.
   * \param cv2 The second curve.
   * \param p The reference point.
   * \return (true) if cv1 and cv2 intersect at a point that is 
   * lexicographically larger than p, that is above or to the right of p 
   * (but not on p).    
   */
  bool do_intersect_to_right(const X_curve_2& cv1, const X_curve_2& cv2,
                             const Point_2& p) const 
  {
    bool     is_overlap;
    Point_2  ip1, ip2;

    if (! _find_intersection (cv1, cv2, is_overlap, ip1, ip2))
      return (false);

    if (! is_overlap) 
      return (compare_xy_2_object()(ip1, p) == LARGER);

    // Since always ip1 < ip2.
    return (compare_xy_2_object()(ip2, p) == LARGER);
  }

  /*! 
   * Check whether the two given curves intersect at a point to the left of
   * a given reference point.
   * \param cv1 The first curve.
   * \param cv2 The second curve.
   * \param p The reference point.
   * \return (true) if cv1 and cv2 intersect at a point that is 
   * lexicographically smaller than p, that is under or to the left of p 
   * (but not on p).    
   */
  bool do_intersect_to_left(const X_curve_2 & cv1, const X_curve_2 & cv2,
                            const Point_2 & p) const 
  {
    bool     is_overlap;
    Point_2  ip1, ip2;

    if (! _find_intersection (cv1, cv2, is_overlap, ip1, ip2))
      return (false);

    if (! is_overlap) 
      return (compare_xy_2_object()(ip1, p) == SMALLER);

    // Since always ip1 < ip2.
    return (compare_xy_2_object()(ip1, p) == SMALLER);
  }

  /*!
   * Find the nearest intersection point of two given curves to the right of 
   * a given point. Nearest is defined as the lexicographically nearest not 
   * including the point itself (with one exception explained below).
   * If the intersection of the two curves is an X_curve_2, that is,
   * there is an overlapping subcurve, then if the the source and target of the
   * subcurve are strickly to the right, they are returned through two
   * other point references p1 and p2. If p is between the source and target
   * of the overlapping subcurve, or p is its left endpoint, p and the target
   * of the right endpoint of the subcurve are returned through p1 and p2 
   * respectively.
   * If the intersection of the two curves is a point to the right of p, it is
   * returned through the p1 and p2.
   * \param cv1 The first curve.
   * \param cv2 The second curve.
   * \param p The refernece point.
   * \param p1 The first output point.
   * \param p2 The second output point.
   * \return (true) if c1 and c2 do intersect to the right of p, or (false)
   * if no such intersection exists.
   */
  bool nearest_intersection_to_right(const X_curve_2 & cv1,
                                     const X_curve_2 & cv2,
                                     const Point_2 & p,
                                     Point_2 & p1, Point_2 & p2) const
  {
    bool     is_overlap;

    if (! _find_intersection (cv1, cv2, is_overlap, p1, p2))
      return (false);

    if (! is_overlap) 
    {
      if (compare_xy_2_object()(p1, p) == LARGER)
      {
	p2 = p1;
	return (true);
      }
      return (false);
    }
    else
    {
      // Notice that p1 < p2.
      if (compare_xy_2_object()(p1, p) == LARGER)
      {
	return (true);
      }
      else if (compare_xy_2_object()(p2, p) == LARGER)
      {
	p1 = p;
	return (true);
      }
      return (false);
    }
  }

  /*!
   * Find the nearest intersection point of two given curves to the left of 
   * a given point. Nearest is defined as the lexicographically nearest not 
   * including the point itself (with one exception explained below).
   * If the intersection of the two curves is an X_curve_2, that is,
   * there is an overlapping subcurve, then if the the source and target of the
   * subcurve are strickly to the left, they are returned through two
   * other point references p1 and p2. If p is between the source and target
   * of the overlapping subcurve, or p is its right endpoint, p and the source
   * of the left endpoint of the subcurve are returned through p1 and p2 
   * respectively.
   * If the intersection of the two curves is a point to the left of p, it is
   * returned through the p1 and p2.
   * \param cv1 The first curve.
   * \param cv2 The second curve.
   * \param p The refernece point.
   * \param p1 The first output point.
   * \param p2 The second output point.
   * \return (true) if c1 and c2 do intersect to the left of p, or (false)
   * if no such intersection exists.
   */
  bool nearest_intersection_to_left(const X_curve_2 & cv1,
                                    const X_curve_2 & cv2,
                                    const Point_2 & p,
                                    Point_2 & p1, Point_2 & p2) const
  {
    bool     is_overlap;

    if (! _find_intersection (cv1, cv2, is_overlap, p1, p2))
      return (false);

    if (! is_overlap) 
    {
      if (compare_xy_2_object()(p1, p) == SMALLER)
      {
	p2 = p1;
	return (true);
      }
      return (false);
    }
    else
    {
      // Notice that ip1 < ip2.
      if (compare_xy_2_object()(p2, p) == SMALLER)
      {
	return (true);
      }
      else if (compare_xy_2_object()(p1, p) == SMALLER)
      {
	p2 = p;
	return (true);
      }
      return (false);
    }
  }

  /*!
   * Check whether the two given curves overlap.
   * \patam cv1 The first curve.
   * \patam cv2 The second curve.
   * \return (true) if the two curves overlap in a one-dimensional subcurve
   * (i.e., not in a finite number of points). Otherwise, if they have a finite
   * number of intersection points, or none at all, return (false).
   */
  bool curves_overlap(const X_curve_2 & cv1, const X_curve_2 & cv2) const
  {
    bool     is_overlap;
    Point_2  ip1, ip2;

    if (! _find_intersection (cv1, cv2, is_overlap, ip1, ip2))
      return (false);

    return (is_overlap); 
  }

private:

  /*!
   * Enum used only be the curve_is_between_clockwise() function.
   */
  enum Segment_dir
  {
    DIR_UP,           // Vertical segment, point at 12 o'clock.
    DIR_RIGHT,        // Non-vertical segment going towards the right.
    DIR_DOWN,         // Vertical segment, point at 6 o'clock.
    DIR_LEFT          // Non-vertical segment going towards the left.
  };

  /*!
   * Return the segment direction, with respect to a given refernece point.
   * \param cv The segment.
   * \param p The reference point.
   * \pre p must be an end-point of the segment.
   * \return DIR_UP if cv is a vertical segment pointing at 12 o'clock;
   *         DIR_RIGHT if cv is a non-vertical segment going to the right of p;
   *         DIR_DOWN if cv is a vertical segment pointing at 6 o'clock;
   *         DIR_LEFT if cv is a non-vertical segment going to the left of p;
   */
  Segment_dir _curve_direction (const X_curve_2& cv,
				const Point_2& p) const
  {
    // p is one of the end-point. Compare it with the other end-point.
    Comparison_result res;

    if (cv.is_vert)
    {
      // Special treatment for vertical segments:
      Compare_xy_2      comp_xy = compare_xy_2_object();
      res = comp_xy(p, cv.ps);

      if (res == EQUAL)
      {
	res = comp_xy(p, cv.pt);
      }
      else
      {
	// Make sure that p is indeed an end-point.
	CGAL_precondition(comp_xy(p, cv.pt) == EQUAL);
      }

      return ((res == SMALLER) ? DIR_UP : DIR_DOWN);
    }

    // In case cv is not vertical:
    Compare_x_2       comp_x = compare_x_2_object();
    res = compare_x(p, cv.ps);

    if (res == EQUAL)
    {
      res = comp_x(p, cv.pt);
    }
    else
    {
      // Make sure that p is indeed an end-point.
      CGAL_precondition(compare_xy_2_object()(p, cv.pt) == EQUAL);
    }

    return ((res == SMALLER) ? DIR_LEFT : DIR_RIGHT);
  }

  Comparison_result _curve_compare_slope_left (const X_curve_2& cv1,
					       const X_curve_2& cv2) const
  { 
    return (compare_slope_2_object()(cv2.orig_seg, cv1.orig_seg));
  }

  Comparison_result _curve_compare_slope_right (const X_curve_2 & cv1,
					        const X_curve_2 & cv2) const 
  {
    return (compare_slope_2_object()(cv1.orig_seg, cv2.orig_seg));
  }

  /*!
   * Find the intersection between teo segements.
   * \param cv1 The first segment.
   * \param cv2 The second segment.
   * \param is_ovelap Are the two segment overlapping.
   * \param p1 The intersection point (if there is no overlap),
   * otherwise the leftmost end-point of the intersection segment.
   * \param p2 If there is an overlap, the rightmost end-point of the 
   * intersection segment.
   * \return (true) if an intersectio has been found.
   */
  bool _find_intersection (const X_curve_2& cv1,
			   const X_curve_2& cv2,
                           bool& is_overlap,
			   Point_2& p1, Point_2& p2) const
  {
    // Intersect the two original segments.
    Object    res = intersect_2_object()(cv1.orig_seg, cv2.orig_seg);

    if (res.is_empty())
    {
      // Empty object is returned - no intersection.
      return (false);
    }
    
    Point_2   ip;
    if (assign(ip, res))
    {
      is_overlap = false;

      // Simple case of intersection at a single point.
      if ((cv1.is_orig || curve_get_point_status(cv1, ip) == ON_CURVE) &&
	  (cv2.is_orig || curve_get_point_status(cv2, ip) == ON_CURVE))
      {
	p1 = ip;
	return (true);
      }
      
      return (false);
    }

    typename Kernel::Segment_2 iseg;
    if (assign(iseg, res)) 
    {
      // Assign the end-points such that p1 < p2.
      Compare_xy_2       comp_xy = compare_xy_2_object();
      Construct_vertex_2 construct_vertex = construct_vertex_2_object();

      p1 = construct_vertex(iseg, 0);
      p2 = construct_vertex(iseg, 1);

      if (! comp_xy(p1, p2) == SMALLER)
      {
        p1 = construct_vertex(iseg, 1);
        p2 = construct_vertex(iseg, 0);
      }

      // Clip the intersection segment with respect to cv1.
      if (! cv1.is_orig)
      {
        Comparison_result res1 = comp_xy(cv1.ps, cv1.pt);
        const Point_2&    left1 = (res1 == SMALLER) ? cv1.ps : cv1.pt;
        const Point_2&    right1 = (res1 == LARGER) ? cv1.ps : cv1.pt;

        if (comp_xy(p2, left1) == SMALLER)
          return (false);
        else if (comp_xy(p1, left1) == SMALLER)
          p1 = left1;

        if (comp_xy(p1, right1) == LARGER)
          return (false);
        else if (comp_xy(p2, right1) == LARGER)
          p2 = right1;
      }

      // Clip the intersection segment with respect to cv2.
      if (! cv2.is_orig)
      {
        Comparison_result res2 = comp_xy(cv2.ps, cv2.pt);
        const Point_2&    left2 = (res2 == SMALLER) ? cv2.ps : cv2.pt;
        const Point_2&    right2 = (res2 == LARGER) ? cv2.ps : cv2.pt;

        if (comp_xy(p2, left2) == SMALLER)
          return (false);
        else if (comp_xy(p1, left2) == SMALLER)
          p1 = left2;

        if (comp_xy(p1, right2) == LARGER)
          return (false);
        else if (comp_xy(p2, right2) == LARGER)
          p2 = right2;
      }

      // Check if the intersection segment has not become a point.
      is_overlap = (comp_xy(p1,p2) != EQUAL);
      CGAL_assertion(comp_xy(p1,p2) != LARGER);
      return (true);
    }

    // No intersection at all:
    return (false);
  }

};

/*!
 * A representation of a segment, as used by the Arr_segment_cached_traits_2
 * traits class.
 */
template <class Kernel_>
class Segment_cached_2 :
    public Arr_segment_cached_traits_2<Kernel_>::My_segment_cached_2
{
  typedef typename Arr_segment_cached_traits_2<Kernel_>::My_segment_cached_2
                                                  Base;
  typedef typename Kernel_::Segment_2             Segment_2;
  typedef typename Kernel_::Point_2               Point_2;

  public:

  /*!
   * Default constructor.
   */
  Segment_cached_2 () :
    Base()
  {}
    
  /*!
   * Constructor from a segment.
   * \param seg The segment.
   */
  Segment_cached_2 (const Segment_2& seg) :
    Base(seg)
  {}

  /*!
   * Construct a segment from two end-points.
   * \param source The source point.
   * \param target The target point.
   */
  Segment_cached_2 (const Point_2& source, const Point_2& target) :
    Base(source,target)
  {}

  /*!
   * Cast to a segment.
   */
  operator Segment_2 () const
  {
    return (Segment_2(ps, pt));
  }

  /*!
   * Create a bounding box for the segment.
   */
  Bbox_2 bbox()
  {
    Segment_2 seg(ps, pt);
    return (seg.bbox());
  }

  /*!
   * Get the segment source.
   */
  const Point_2& source() const 
  { 
    return ps; 
  }

  /*!
   * Get the segment target.
   */
  const Point_2& target() const
  { 
    return pt;
  }
};

template <class Kernel_>
::std::ostream& operator<< (::std::ostream& os,
                            const Segment_cached_2<Kernel_>& seg)
{
  os << static_cast<typename Kernel_::Segment_2>(seg);
  return (os);
}

CGAL_END_NAMESPACE

#endif
