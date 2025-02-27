/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPath_DEFINED
#define SkPath_DEFINED

#include <string>
#include "include/core/SkMatrix.h"
#include "include/private/SkPathRef.h"
#include "include/private/SkTo.h"

#include <initializer_list>

class SkAutoPathBoundsUpdate;
class SkData;
class SkRRect;
class SkWStream;

/** \class SkPath
    SkPath contain geometry. SkPath may be empty, or contain one or more verbs that
    outline a figure. SkPath always starts with a move verb to a Cartesian coordinate,
    and may be followed by additional verbs that add lines or curves.
    Adding a close verb makes the geometry into a continuous loop, a closed contour.
    SkPath may contain any number of contours, each beginning with a move verb.

    SkPath contours may contain only a move verb, or may also contain lines,
    quadratic beziers, conics, and cubic beziers. SkPath contours may be open or
    closed.

    When used to draw a filled area, SkPath describes whether the fill is inside or
    outside the geometry. SkPath also describes the winding rule used to fill
    overlapping contours.

    Internally, SkPath lazily computes metrics likes bounds and convexity. Call
    SkPath::updateBoundsCache to make SkPath thread safe.
*/
class SK_API SkPath {
public:

    /** \enum SkPath::Direction
        Direction describes whether contour is clockwise or counterclockwise.
        When SkPath contains multiple overlapping contours, Direction together with
        FillType determines whether overlaps are filled or form holes.

        Direction also determines how contour is measured. For instance, dashing
        measures along SkPath to determine where to start and stop stroke; Direction
        will change dashed results as it steps clockwise or counterclockwise.

        Closed contours like SkRect, SkRRect, circle, and oval added with
        kCW_Direction travel clockwise; the same added with kCCW_Direction
        travel counterclockwise.
    */
    enum Direction : int {
        kCW_Direction,  //!< contour travels clockwise
        kCCW_Direction, //!< contour travels counterclockwise
    };

    /** Constructs an empty SkPath. By default, SkPath has no verbs, no SkPoint, and no weights.
        SkPath::FillType is set to kWinding_FillType.

        @return  empty SkPath
    */
    SkPath();

    /** Constructs a copy of an existing path.
        Copy constructor makes two paths identical by value. Internally, path and
        the returned result share pointer values. The underlying verb array, SkPoint array
        and weights are copied when modified.

        Creating a SkPath copy is very efficient and never allocates memory.
        SkPath are always copied by value from the interface; the underlying shared
        pointers are not exposed.

        @param path  SkPath to copy by value
        @return      copy of SkPath
    */
    SkPath(const SkPath& path);

    /** Releases ownership of any shared data and deletes data if SkPath is sole owner.
    */
    ~SkPath();

    /** Constructs a copy of an existing path.
        SkPath assignment makes two paths identical by value. Internally, assignment
        shares pointer values. The underlying verb array, SkPoint array and weights
        are copied when modified.

        Copying SkPath by assignment is very efficient and never allocates memory.
        SkPath are always copied by value from the interface; the underlying shared
        pointers are not exposed.

        @param path  verb array, SkPoint array, weights, and SkPath::FillType to copy
        @return      SkPath copied by value
    */
    SkPath& operator=(const SkPath& path);

    /** Compares a and b; returns true if SkPath::FillType, verb array, SkPoint array, and weights
        are equivalent.

        @param a  SkPath to compare
        @param b  SkPath to compare
        @return   true if SkPath pair are equivalent
    */
    friend SK_API bool operator==(const SkPath& a, const SkPath& b);

    /** Compares a and b; returns true if SkPath::FillType, verb array, SkPoint array, and weights
        are not equivalent.

        @param a  SkPath to compare
        @param b  SkPath to compare
        @return   true if SkPath pair are not equivalent
    */
    friend bool operator!=(const SkPath& a, const SkPath& b) {
        return !(a == b);
    }

    /** Returns true if SkPath contain equal verbs and equal weights.
        If SkPath contain one or more conics, the weights must match.

        conicTo() may add different verbs depending on conic weight, so it is not
        trivial to interpolate a pair of SkPath containing conics with different
        conic weight values.

        @param compare  SkPath to compare
        @return         true if SkPath verb array and weights are equivalent
    */
    bool isInterpolatable(const SkPath& compare) const;

    /** Interpolates between SkPath with SkPoint array of equal size.
        Copy verb array and weights to out, and set out SkPoint array to a weighted
        average of this SkPoint array and ending SkPoint array, using the formula:
        (Path Point * weight) + ending Point * (1 - weight).

        weight is most useful when between zero (ending SkPoint array) and
        one (this Point_Array); will work with values outside of this
        range.

        interpolate() returns false and leaves out unchanged if SkPoint array is not
        the same size as ending SkPoint array. Call isInterpolatable() to check SkPath
        compatibility prior to calling interpolate().

        @param ending  SkPoint array averaged with this SkPoint array
        @param weight  contribution of this SkPoint array, and
                       one minus contribution of ending SkPoint array
        @param out     SkPath replaced by interpolated averages
        @return        true if SkPath contain same number of SkPoint
    */
    bool interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const;

    /** \enum SkPath::FillType
        FillType selects the rule used to fill SkPath. SkPath set to kWinding_FillType
        fills if the sum of contour edges is not zero, where clockwise edges add one, and
        counterclockwise edges subtract one. SkPath set to kEvenOdd_FillType fills if the
        number of contour edges is odd. Each FillType has an inverse variant that
        reverses the rule:
        kInverseWinding_FillType fills where the sum of contour edges is zero;
        kInverseEvenOdd_FillType fills where the number of contour edges is even.
    */
    enum FillType {
        kWinding_FillType,        //!< is enclosed by a non-zero sum of contour directions
        kEvenOdd_FillType,        //!< is enclosed by an odd number of contours
        kInverseWinding_FillType, //!< is enclosed by a zero sum of contour directions
        kInverseEvenOdd_FillType, //!< is enclosed by an even number of contours
    };

    /** Returns FillType, the rule used to fill SkPath. FillType of a new SkPath is
        kWinding_FillType.

        @return  one of: kWinding_FillType, kEvenOdd_FillType,  kInverseWinding_FillType,
                 kInverseEvenOdd_FillType
    */
    FillType getFillType() const { return (FillType)fFillType; }

    /** Sets FillType, the rule used to fill SkPath. While there is no check
        that ft is legal, values outside of FillType are not supported.

        @param ft  one of: kWinding_FillType, kEvenOdd_FillType,  kInverseWinding_FillType,
                   kInverseEvenOdd_FillType
    */
    void setFillType(FillType ft) {
        fFillType = SkToU8(ft);
    }

    /** Returns if FillType describes area outside SkPath geometry. The inverse fill area
        extends indefinitely.

        @return  true if FillType is kInverseWinding_FillType or kInverseEvenOdd_FillType
    */
    bool isInverseFillType() const { return IsInverseFillType((FillType)fFillType); }

    /** Replaces FillType with its inverse. The inverse of FillType describes the area
        unmodified by the original FillType.
    */
    void toggleInverseFillType() {
        fFillType ^= 2;
    }

    /** \enum SkPath::Convexity
        SkPath is convex if it contains one contour and contour loops no more than
        360 degrees, and contour angles all have same Direction. Convex SkPath
        may have better performance and require fewer resources on GPU surface.

        SkPath is concave when either at least one Direction change is clockwise and
        another is counterclockwise, or the sum of the changes in Direction is not 360
        degrees.

        Initially SkPath Convexity is kUnknown_Convexity. SkPath Convexity is computed
        if needed by destination SkSurface.
    */
    enum Convexity : uint8_t {
        kUnknown_Convexity, //!< indicates Convexity has not been determined
        kConvex_Convexity,  //!< one contour made of a simple geometry without indentations
        kConcave_Convexity, //!< more than one contour, or a geometry with indentations
    };

    /** Computes SkPath::Convexity if required, and returns stored value.
        SkPath::Convexity is computed if stored value is kUnknown_Convexity,
        or if SkPath has been altered since SkPath::Convexity was computed or set.

        @return  computed or stored SkPath::Convexity
    */
    Convexity getConvexity() const {
        Convexity convexity = this->getConvexityOrUnknown();
        if (convexity != kUnknown_Convexity) {
            return convexity;
        }
        return this->internalGetConvexity();
    }

    /** Returns last computed SkPath::Convexity, or kUnknown_Convexity if
        SkPath has been altered since SkPath::Convexity was computed or set.

        @return  stored SkPath::Convexity
    */
    Convexity getConvexityOrUnknown() const { return fConvexity.load(std::memory_order_relaxed); }

    /** Stores convexity so that it is later returned by getConvexity() or getConvexityOrUnknown().
        convexity may differ from getConvexity(), although setting an incorrect value may
        cause incorrect or inefficient drawing.

        If convexity is kUnknown_Convexity: getConvexity() will
        compute SkPath::Convexity, and getConvexityOrUnknown() will return kUnknown_Convexity.

        If convexity is kConvex_Convexity or kConcave_Convexity, getConvexity()
        and getConvexityOrUnknown() will return convexity until the path is
        altered.

        @param convexity  one of: kUnknown_Convexity, kConvex_Convexity, or kConcave_Convexity
    */
    void setConvexity(Convexity convexity);

    /** Computes SkPath::Convexity if required, and returns true if value is kConvex_Convexity.
        If setConvexity() was called with kConvex_Convexity or kConcave_Convexity, and
        the path has not been altered, SkPath::Convexity is not recomputed.

        @return  true if SkPath::Convexity stored or computed is kConvex_Convexity
    */
    bool isConvex() const {
        return kConvex_Convexity == this->getConvexity();
    }

    /** Returns true if this path is recognized as an oval or circle.

        bounds receives bounds of oval.

        bounds is unmodified if oval is not found.

        @param bounds  storage for bounding SkRect of oval; may be nullptr
        @return        true if SkPath is recognized as an oval or circle
    */
    bool isOval(SkRect* bounds) const;

    /** Returns true if path is representable as SkRRect.
        Returns false if path is representable as oval, circle, or SkRect.

        rrect receives bounds of SkRRect.

        rrect is unmodified if SkRRect is not found.

        @param rrect  storage for bounding SkRect of SkRRect; may be nullptr
        @return       true if SkPath contains only SkRRect
    */
    bool isRRect(SkRRect* rrect) const;

    /** Sets SkPath to its initial state.
        Removes verb array, SkPoint array, and weights, and sets FillType to kWinding_FillType.
        Internal storage associated with SkPath is released.

        @return  reference to SkPath
    */
    SkPath& reset();

    /** Sets SkPath to its initial state, preserving internal storage.
        Removes verb array, SkPoint array, and weights, and sets FillType to kWinding_FillType.
        Internal storage associated with SkPath is retained.

        Use rewind() instead of reset() if SkPath storage will be reused and performance
        is critical.

        @return  reference to SkPath
    */
    SkPath& rewind();

    /** Returns if SkPath is empty.
        Empty SkPath may have FillType but has no SkPoint, SkPath::Verb, or conic weight.
        SkPath() constructs empty SkPath; reset() and rewind() make SkPath empty.

        @return  true if the path contains no SkPath::Verb array
    */
    bool isEmpty() const {
        SkDEBUGCODE(this->validate();)
        return 0 == fPathRef->countVerbs();
    }

    /** Returns if contour is closed.
        Contour is closed if SkPath SkPath::Verb array was last modified by close(). When stroked,
        closed contour draws SkPaint::Join instead of SkPaint::Cap at first and last SkPoint.

        @return  true if the last contour ends with a kClose_Verb
    */
    bool isLastContourClosed() const;

    /** Returns true for finite SkPoint array values between negative SK_ScalarMax and
        positive SK_ScalarMax. Returns false for any SkPoint array value of
        SK_ScalarInfinity, SK_ScalarNegativeInfinity, or SK_ScalarNaN.

        @return  true if all SkPoint values are finite
    */
    bool isFinite() const {
        SkDEBUGCODE(this->validate();)
        return fPathRef->isFinite();
    }

    /** Returns true if the path is volatile; it will not be altered or discarded
        by the caller after it is drawn. SkPath by default have volatile set false, allowing
        SkSurface to attach a cache of data which speeds repeated drawing. If true, SkSurface
        may not speed repeated drawing.

        @return  true if caller will alter SkPath after drawing
    */
    bool isVolatile() const {
        return SkToBool(fIsVolatile);
    }

    /** Specifies whether SkPath is volatile; whether it will be altered or discarded
        by the caller after it is drawn. SkPath by default have volatile set false, allowing
        SkBaseDevice to attach a cache of data which speeds repeated drawing.

        Mark temporary paths, discarded or modified after use, as volatile
        to inform SkBaseDevice that the path need not be cached.

        Mark animating SkPath volatile to improve performance.
        Mark unchanging SkPath non-volatile to improve repeated rendering.

        raster surface SkPath draws are affected by volatile for some shadows.
        GPU surface SkPath draws are affected by volatile for some shadows and concave geometries.

        @param isVolatile  true if caller will alter SkPath after drawing
    */
    void setIsVolatile(bool isVolatile) {
        fIsVolatile = isVolatile;
    }

    /** Tests if line between SkPoint pair is degenerate.
        Line with no length or that moves a very short distance is degenerate; it is
        treated as a point.

        exact changes the equality test. If true, returns true only if p1 equals p2.
        If false, returns true if p1 equals or nearly equals p2.

        @param p1     line start point
        @param p2     line end point
        @param exact  if false, allow nearly equals
        @return       true if line is degenerate; its length is effectively zero
    */
    static bool IsLineDegenerate(const SkPoint& p1, const SkPoint& p2, bool exact);

    /** Tests if quad is degenerate.
        Quad with no length or that moves a very short distance is degenerate; it is
        treated as a point.

        @param p1     quad start point
        @param p2     quad control point
        @param p3     quad end point
        @param exact  if true, returns true only if p1, p2, and p3 are equal;
                      if false, returns true if p1, p2, and p3 are equal or nearly equal
        @return       true if quad is degenerate; its length is effectively zero
    */
    static bool IsQuadDegenerate(const SkPoint& p1, const SkPoint& p2,
                                 const SkPoint& p3, bool exact);

    /** Tests if cubic is degenerate.
        Cubic with no length or that moves a very short distance is degenerate; it is
        treated as a point.

        @param p1     cubic start point
        @param p2     cubic control point 1
        @param p3     cubic control point 2
        @param p4     cubic end point
        @param exact  if true, returns true only if p1, p2, p3, and p4 are equal;
                      if false, returns true if p1, p2, p3, and p4 are equal or nearly equal
        @return       true if cubic is degenerate; its length is effectively zero
    */
    static bool IsCubicDegenerate(const SkPoint& p1, const SkPoint& p2,
                                  const SkPoint& p3, const SkPoint& p4, bool exact);

    /** Returns true if SkPath contains only one line;
        SkPath::Verb array has two entries: kMove_Verb, kLine_Verb.
        If SkPath contains one line and line is not nullptr, line is set to
        line start point and line end point.
        Returns false if SkPath is not one line; line is unaltered.

        @param line  storage for line. May be nullptr
        @return      true if SkPath contains exactly one line
    */
    bool isLine(SkPoint line[2]) const;

    /** Returns the number of points in SkPath.
        SkPoint count is initially zero.

        @return  SkPath SkPoint array length
    */
    int countPoints() const;

    /** Returns SkPoint at index in SkPoint array. Valid range for index is
        0 to countPoints() - 1.
        Returns (0, 0) if index is out of range.

        @param index  SkPoint array element selector
        @return       SkPoint array value or (0, 0)
    */
    SkPoint getPoint(int index) const;

    /** Returns number of points in SkPath. Up to max points are copied.
        points may be nullptr; then, max must be zero.
        If max is greater than number of points, excess points storage is unaltered.

        @param points  storage for SkPath SkPoint array. May be nullptr
        @param max     maximum to copy; must be greater than or equal to zero
        @return        SkPath SkPoint array length
    */
    int getPoints(SkPoint points[], int max) const;

    /** Returns the number of verbs: kMove_Verb, kLine_Verb, kQuad_Verb, kConic_Verb,
        kCubic_Verb, and kClose_Verb; added to SkPath.

        @return  length of verb array
    */
    int countVerbs() const;

    /** Returns the number of verbs in the path. Up to max verbs are copied. The
        verbs are copied as one byte per verb.

        @param verbs  storage for verbs, may be nullptr
        @param max    maximum number to copy into verbs
        @return       the actual number of verbs in the path
    */
    int getVerbs(uint8_t verbs[], int max) const;

    /** Returns the approximate byte size of the SkPath in memory.

        @return  approximate size
    */
    size_t approximateBytesUsed() const;

    /** Exchanges the verb array, SkPoint array, weights, and SkPath::FillType with other.
        Cached state is also exchanged. swap() internally exchanges pointers, so
        it is lightweight and does not allocate memory.

        swap() usage has largely been replaced by operator=(const SkPath& path).
        SkPath do not copy their content on assignment until they are written to,
        making assignment as efficient as swap().

        @param other  SkPath exchanged by value
    */
    void swap(SkPath& other);

    /** Returns minimum and maximum axes values of SkPoint array.
        Returns (0, 0, 0, 0) if SkPath contains no points. Returned bounds width and height may
        be larger or smaller than area affected when SkPath is drawn.

        SkRect returned includes all SkPoint added to SkPath, including SkPoint associated with
        kMove_Verb that define empty contours.

        @return  bounds of all SkPoint in SkPoint array
    */
    const SkRect& getBounds() const {
        return fPathRef->getBounds();
    }

    /** Updates internal bounds so that subsequent calls to getBounds() are instantaneous.
        Unaltered copies of SkPath may also access cached bounds through getBounds().

        For now, identical to calling getBounds() and ignoring the returned value.

        Call to prepare SkPath subsequently drawn from multiple threads,
        to avoid a race condition where each draw separately computes the bounds.
    */
    void updateBoundsCache() const {
        // for now, just calling getBounds() is sufficient
        this->getBounds();
    }

    /** Returns minimum and maximum axes values of the lines and curves in SkPath.
        Returns (0, 0, 0, 0) if SkPath contains no points.
        Returned bounds width and height may be larger or smaller than area affected
        when SkPath is drawn.

        Includes SkPoint associated with kMove_Verb that define empty
        contours.

        Behaves identically to getBounds() when SkPath contains
        only lines. If SkPath contains curves, computed bounds includes
        the maximum extent of the quad, conic, or cubic; is slower than getBounds();
        and unlike getBounds(), does not cache the result.

        @return  tight bounds of curves in SkPath
    */
    SkRect computeTightBounds() const;

    /** Returns true if rect is contained by SkPath.
        May return false when rect is contained by SkPath.

        For now, only returns true if SkPath has one contour and is convex.
        rect may share points and edges with SkPath and be contained.
        Returns true if rect is empty, that is, it has zero width or height; and
        the SkPoint or line described by rect is contained by SkPath.

        @param rect  SkRect, line, or SkPoint checked for containment
        @return      true if rect is contained
    */
    bool conservativelyContainsRect(const SkRect& rect) const;

    /** Grows SkPath verb array and SkPoint array to contain extraPtCount additional SkPoint.
        May improve performance and use less memory by
        reducing the number and size of allocations when creating SkPath.

        @param extraPtCount  number of additional SkPoint to allocate
    */
    void incReserve(int extraPtCount);

    /** Shrinks SkPath verb array and SkPoint array storage to discard unused capacity.
        May reduce the heap overhead for SkPath known to be fully constructed.
    */
    void shrinkToFit();

    /** Adds beginning of contour at SkPoint (x, y).

        @param x  x-axis value of contour start
        @param y  y-axis value of contour start
        @return   reference to SkPath
    */
    SkPath& moveTo(SkScalar x, SkScalar y);

    /** Adds beginning of contour at SkPoint p.

        @param p  contour start
        @return   reference to SkPath
    */
    SkPath& moveTo(const SkPoint& p) {
        return this->moveTo(p.fX, p.fY);
    }

    /** Adds beginning of contour relative to last point.
        If SkPath is empty, starts contour at (dx, dy).
        Otherwise, start contour at last point offset by (dx, dy).
        Function name stands for "relative move to".

        @param dx  offset from last point to contour start on x-axis
        @param dy  offset from last point to contour start on y-axis
        @return    reference to SkPath
    */
    SkPath& rMoveTo(SkScalar dx, SkScalar dy);

    /** Adds line from last point to (x, y). If SkPath is empty, or last SkPath::Verb is
        kClose_Verb, last point is set to (0, 0) before adding line.

        lineTo() appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed.
        lineTo() then appends kLine_Verb to verb array and (x, y) to SkPoint array.

        @param x  end of added line on x-axis
        @param y  end of added line on y-axis
        @return   reference to SkPath
    */
    SkPath& lineTo(SkScalar x, SkScalar y);

    /** Adds line from last point to SkPoint p. If SkPath is empty, or last SkPath::Verb is
        kClose_Verb, last point is set to (0, 0) before adding line.

        lineTo() first appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed.
        lineTo() then appends kLine_Verb to verb array and SkPoint p to SkPoint array.

        @param p  end SkPoint of added line
        @return   reference to SkPath
    */
    SkPath& lineTo(const SkPoint& p) {
        return this->lineTo(p.fX, p.fY);
    }

    /** Adds line from last point to vector (dx, dy). If SkPath is empty, or last SkPath::Verb is
        kClose_Verb, last point is set to (0, 0) before adding line.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kLine_Verb to verb array and line end to SkPoint array.
        Line end is last point plus vector (dx, dy).
        Function name stands for "relative line to".

        @param dx  offset from last point to line end on x-axis
        @param dy  offset from last point to line end on y-axis
        @return    reference to SkPath
    */
    SkPath& rLineTo(SkScalar dx, SkScalar dy);

    /** Adds quad from last point towards (x1, y1), to (x2, y2).
        If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding quad.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kQuad_Verb to verb array; and (x1, y1), (x2, y2)
        to SkPoint array.

        @param x1  control SkPoint of quad on x-axis
        @param y1  control SkPoint of quad on y-axis
        @param x2  end SkPoint of quad on x-axis
        @param y2  end SkPoint of quad on y-axis
        @return    reference to SkPath
    */
    SkPath& quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2);

    /** Adds quad from last point towards SkPoint p1, to SkPoint p2.
        If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding quad.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kQuad_Verb to verb array; and SkPoint p1, p2
        to SkPoint array.

        @param p1  control SkPoint of added quad
        @param p2  end SkPoint of added quad
        @return    reference to SkPath
    */
    SkPath& quadTo(const SkPoint& p1, const SkPoint& p2) {
        return this->quadTo(p1.fX, p1.fY, p2.fX, p2.fY);
    }

    /** Adds quad from last point towards vector (dx1, dy1), to vector (dx2, dy2).
        If SkPath is empty, or last SkPath::Verb
        is kClose_Verb, last point is set to (0, 0) before adding quad.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array,
        if needed; then appends kQuad_Verb to verb array; and appends quad
        control and quad end to SkPoint array.
        Quad control is last point plus vector (dx1, dy1).
        Quad end is last point plus vector (dx2, dy2).
        Function name stands for "relative quad to".

        @param dx1  offset from last point to quad control on x-axis
        @param dy1  offset from last point to quad control on y-axis
        @param dx2  offset from last point to quad end on x-axis
        @param dy2  offset from last point to quad end on y-axis
        @return     reference to SkPath
    */
    SkPath& rQuadTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2);

    /** Adds conic from last point towards (x1, y1), to (x2, y2), weighted by w.
        If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding conic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed.

        If w is finite and not one, appends kConic_Verb to verb array;
        and (x1, y1), (x2, y2) to SkPoint array; and w to conic weights.

        If w is one, appends kQuad_Verb to verb array, and
        (x1, y1), (x2, y2) to SkPoint array.

        If w is not finite, appends kLine_Verb twice to verb array, and
        (x1, y1), (x2, y2) to SkPoint array.

        @param x1  control SkPoint of conic on x-axis
        @param y1  control SkPoint of conic on y-axis
        @param x2  end SkPoint of conic on x-axis
        @param y2  end SkPoint of conic on y-axis
        @param w   weight of added conic
        @return    reference to SkPath
    */
    SkPath& conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                    SkScalar w);

    /** Adds conic from last point towards SkPoint p1, to SkPoint p2, weighted by w.
        If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to (0, 0)
        before adding conic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed.

        If w is finite and not one, appends kConic_Verb to verb array;
        and SkPoint p1, p2 to SkPoint array; and w to conic weights.

        If w is one, appends kQuad_Verb to verb array, and SkPoint p1, p2
        to SkPoint array.

        If w is not finite, appends kLine_Verb twice to verb array, and
        SkPoint p1, p2 to SkPoint array.

        @param p1  control SkPoint of added conic
        @param p2  end SkPoint of added conic
        @param w   weight of added conic
        @return    reference to SkPath
    */
    SkPath& conicTo(const SkPoint& p1, const SkPoint& p2, SkScalar w) {
        return this->conicTo(p1.fX, p1.fY, p2.fX, p2.fY, w);
    }

    /** Adds conic from last point towards vector (dx1, dy1), to vector (dx2, dy2),
        weighted by w. If SkPath is empty, or last SkPath::Verb
        is kClose_Verb, last point is set to (0, 0) before adding conic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array,
        if needed.

        If w is finite and not one, next appends kConic_Verb to verb array,
        and w is recorded as conic weight; otherwise, if w is one, appends
        kQuad_Verb to verb array; or if w is not finite, appends kLine_Verb
        twice to verb array.

        In all cases appends SkPoint control and end to SkPoint array.
        control is last point plus vector (dx1, dy1).
        end is last point plus vector (dx2, dy2).

        Function name stands for "relative conic to".

        @param dx1  offset from last point to conic control on x-axis
        @param dy1  offset from last point to conic control on y-axis
        @param dx2  offset from last point to conic end on x-axis
        @param dy2  offset from last point to conic end on y-axis
        @param w    weight of added conic
        @return     reference to SkPath
    */
    SkPath& rConicTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2,
                     SkScalar w);

    /** Adds cubic from last point towards (x1, y1), then towards (x2, y2), ending at
        (x3, y3). If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to
        (0, 0) before adding cubic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kCubic_Verb to verb array; and (x1, y1), (x2, y2), (x3, y3)
        to SkPoint array.

        @param x1  first control SkPoint of cubic on x-axis
        @param y1  first control SkPoint of cubic on y-axis
        @param x2  second control SkPoint of cubic on x-axis
        @param y2  second control SkPoint of cubic on y-axis
        @param x3  end SkPoint of cubic on x-axis
        @param y3  end SkPoint of cubic on y-axis
        @return    reference to SkPath
    */
    SkPath& cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                    SkScalar x3, SkScalar y3);

    /** Adds cubic from last point towards SkPoint p1, then towards SkPoint p2, ending at
        SkPoint p3. If SkPath is empty, or last SkPath::Verb is kClose_Verb, last point is set to
        (0, 0) before adding cubic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array, if needed;
        then appends kCubic_Verb to verb array; and SkPoint p1, p2, p3
        to SkPoint array.

        @param p1  first control SkPoint of cubic
        @param p2  second control SkPoint of cubic
        @param p3  end SkPoint of cubic
        @return    reference to SkPath
    */
    SkPath& cubicTo(const SkPoint& p1, const SkPoint& p2, const SkPoint& p3) {
        return this->cubicTo(p1.fX, p1.fY, p2.fX, p2.fY, p3.fX, p3.fY);
    }

    /** Adds cubic from last point towards vector (dx1, dy1), then towards
        vector (dx2, dy2), to vector (dx3, dy3).
        If SkPath is empty, or last SkPath::Verb
        is kClose_Verb, last point is set to (0, 0) before adding cubic.

        Appends kMove_Verb to verb array and (0, 0) to SkPoint array,
        if needed; then appends kCubic_Verb to verb array; and appends cubic
        control and cubic end to SkPoint array.
        Cubic control is last point plus vector (dx1, dy1).
        Cubic end is last point plus vector (dx2, dy2).
        Function name stands for "relative cubic to".

        @param dx1  offset from last point to first cubic control on x-axis
        @param dy1  offset from last point to first cubic control on y-axis
        @param dx2  offset from last point to second cubic control on x-axis
        @param dy2  offset from last point to second cubic control on y-axis
        @param dx3  offset from last point to cubic end on x-axis
        @param dy3  offset from last point to cubic end on y-axis
        @return    reference to SkPath
    */
    SkPath& rCubicTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2,
                     SkScalar dx3, SkScalar dy3);

    /** Appends arc to SkPath. Arc added is part of ellipse
        bounded by oval, from startAngle through sweepAngle. Both startAngle and
        sweepAngle are measured in degrees, where zero degrees is aligned with the
        positive x-axis, and positive sweeps extends arc clockwise.

        arcTo() adds line connecting SkPath last SkPoint to initial arc SkPoint if forceMoveTo
        is false and SkPath is not empty. Otherwise, added contour begins with first point
        of arc. Angles greater than -360 and less than 360 are treated modulo 360.

        @param oval         bounds of ellipse containing arc
        @param startAngle   starting angle of arc in degrees
        @param sweepAngle   sweep, in degrees. Positive is clockwise; treated modulo 360
        @param forceMoveTo  true to start a new contour with arc
        @return             reference to SkPath
    */
    SkPath& arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo);

    /** Appends arc to SkPath, after appending line if needed. Arc is implemented by conic
        weighted to describe part of circle. Arc is contained by tangent from
        last SkPath point to (x1, y1), and tangent from (x1, y1) to (x2, y2). Arc
        is part of circle sized to radius, positioned so it touches both tangent lines.

        If last Path Point does not start Arc, arcTo appends connecting Line to Path.
        The length of Vector from (x1, y1) to (x2, y2) does not affect Arc.

        Arc sweep is always less than 180 degrees. If radius is zero, or if
        tangents are nearly parallel, arcTo appends Line from last Path Point to (x1, y1).

        arcTo appends at most one Line and one conic.
        arcTo implements the functionality of PostScript arct and HTML Canvas arcTo.

        @param x1      x-axis value common to pair of tangents
        @param y1      y-axis value common to pair of tangents
        @param x2      x-axis value end of second tangent
        @param y2      y-axis value end of second tangent
        @param radius  distance from arc to circle center
        @return        reference to SkPath
    */
    SkPath& arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius);

    /** Appends arc to SkPath, after appending line if needed. Arc is implemented by conic
        weighted to describe part of circle. Arc is contained by tangent from
        last SkPath point to p1, and tangent from p1 to p2. Arc
        is part of circle sized to radius, positioned so it touches both tangent lines.

        If last SkPath SkPoint does not start arc, arcTo() appends connecting line to SkPath.
        The length of vector from p1 to p2 does not affect arc.

        Arc sweep is always less than 180 degrees. If radius is zero, or if
        tangents are nearly parallel, arcTo() appends line from last SkPath SkPoint to p1.

        arcTo() appends at most one line and one conic.
        arcTo() implements the functionality of PostScript arct and HTML Canvas arcTo.

        @param p1      SkPoint common to pair of tangents
        @param p2      end of second tangent
        @param radius  distance from arc to circle center
        @return        reference to SkPath
    */
    SkPath& arcTo(const SkPoint p1, const SkPoint p2, SkScalar radius) {
        return this->arcTo(p1.fX, p1.fY, p2.fX, p2.fY, radius);
    }

    /** \enum SkPath::ArcSize
        Four oval parts with radii (rx, ry) start at last SkPath SkPoint and ends at (x, y).
        ArcSize and Direction select one of the four oval parts.
    */
    enum ArcSize {
        kSmall_ArcSize, //!< smaller of arc pair
        kLarge_ArcSize, //!< larger of arc pair
    };

    /** Appends arc to SkPath. Arc is implemented by one or more conics weighted to
        describe part of oval with radii (rx, ry) rotated by xAxisRotate degrees. Arc
        curves from last SkPath SkPoint to (x, y), choosing one of four possible routes:
        clockwise or counterclockwise, and smaller or larger.

        Arc sweep is always less than 360 degrees. arcTo() appends line to (x, y) if
        either radii are zero, or if last SkPath SkPoint equals (x, y). arcTo() scales radii
        (rx, ry) to fit last SkPath SkPoint and (x, y) if both are greater than zero but
        too small.

        arcTo() appends up to four conic curves.
        arcTo() implements the functionality of SVG arc, although SVG sweep-flag value
        is opposite the integer value of sweep; SVG sweep-flag uses 1 for clockwise,
        while kCW_Direction cast to int is zero.

        @param rx           radius on x-axis before x-axis rotation
        @param ry           radius on y-axis before x-axis rotation
        @param xAxisRotate  x-axis rotation in degrees; positive values are clockwise
        @param largeArc     chooses smaller or larger arc
        @param sweep        chooses clockwise or counterclockwise arc
        @param x            end of arc
        @param y            end of arc
        @return             reference to SkPath
    */
    SkPath& arcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc,
                  Direction sweep, SkScalar x, SkScalar y);

    /** Appends arc to SkPath. Arc is implemented by one or more conic weighted to describe
        part of oval with radii (r.fX, r.fY) rotated by xAxisRotate degrees. Arc curves
        from last SkPath SkPoint to (xy.fX, xy.fY), choosing one of four possible routes:
        clockwise or counterclockwise,
        and smaller or larger.

        Arc sweep is always less than 360 degrees. arcTo() appends line to xy if either
        radii are zero, or if last SkPath SkPoint equals (xy.fX, xy.fY). arcTo() scales radii r to
        fit last SkPath SkPoint and xy if both are greater than zero but too small to describe
        an arc.

        arcTo() appends up to four conic curves.
        arcTo() implements the functionality of SVG arc, although SVG sweep-flag value is
        opposite the integer value of sweep; SVG sweep-flag uses 1 for clockwise, while
        kCW_Direction cast to int is zero.

        @param r            radii on axes before x-axis rotation
        @param xAxisRotate  x-axis rotation in degrees; positive values are clockwise
        @param largeArc     chooses smaller or larger arc
        @param sweep        chooses clockwise or counterclockwise arc
        @param xy           end of arc
        @return             reference to SkPath
    */
    SkPath& arcTo(const SkPoint r, SkScalar xAxisRotate, ArcSize largeArc, Direction sweep,
               const SkPoint xy) {
        return this->arcTo(r.fX, r.fY, xAxisRotate, largeArc, sweep, xy.fX, xy.fY);
    }

    /** Appends arc to SkPath, relative to last SkPath SkPoint. Arc is implemented by one or
        more conic, weighted to describe part of oval with radii (rx, ry) rotated by
        xAxisRotate degrees. Arc curves from last SkPath SkPoint to relative end SkPoint:
        (dx, dy), choosing one of four possible routes: clockwise or
        counterclockwise, and smaller or larger. If SkPath is empty, the start arc SkPoint
        is (0, 0).

        Arc sweep is always less than 360 degrees. arcTo() appends line to end SkPoint
        if either radii are zero, or if last SkPath SkPoint equals end SkPoint.
        arcTo() scales radii (rx, ry) to fit last SkPath SkPoint and end SkPoint if both are
        greater than zero but too small to describe an arc.

        arcTo() appends up to four conic curves.
        arcTo() implements the functionality of svg arc, although SVG "sweep-flag" value is
        opposite the integer value of sweep; SVG "sweep-flag" uses 1 for clockwise, while
        kCW_Direction cast to int is zero.

        @param rx           radius before x-axis rotation
        @param ry           radius before x-axis rotation
        @param xAxisRotate  x-axis rotation in degrees; positive values are clockwise
        @param largeArc     chooses smaller or larger arc
        @param sweep        chooses clockwise or counterclockwise arc
        @param dx           x-axis offset end of arc from last SkPath SkPoint
        @param dy           y-axis offset end of arc from last SkPath SkPoint
        @return             reference to SkPath
    */
    SkPath& rArcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc,
                   Direction sweep, SkScalar dx, SkScalar dy);

    /** Appends kClose_Verb to SkPath. A closed contour connects the first and last SkPoint
        with line, forming a continuous loop. Open and closed contour draw the same
        with SkPaint::kFill_Style. With SkPaint::kStroke_Style, open contour draws
        SkPaint::Cap at contour start and end; closed contour draws
        SkPaint::Join at contour start and end.

        close() has no effect if SkPath is empty or last SkPath SkPath::Verb is kClose_Verb.

        @return  reference to SkPath
    */
    SkPath& close();

    /** Returns true if fill is inverted and SkPath with fill represents area outside
        of its geometric bounds.

        @param fill  one of: kWinding_FillType, kEvenOdd_FillType,
                     kInverseWinding_FillType, kInverseEvenOdd_FillType
        @return      true if SkPath fills outside its bounds
    */
    static bool IsInverseFillType(FillType fill) {
        static_assert(0 == kWinding_FillType, "fill_type_mismatch");
        static_assert(1 == kEvenOdd_FillType, "fill_type_mismatch");
        static_assert(2 == kInverseWinding_FillType, "fill_type_mismatch");
        static_assert(3 == kInverseEvenOdd_FillType, "fill_type_mismatch");
        return (fill & 2) != 0;
    }

    /** Returns equivalent SkPath::FillType representing SkPath fill inside its bounds.
        .

        @param fill  one of: kWinding_FillType, kEvenOdd_FillType,
                     kInverseWinding_FillType, kInverseEvenOdd_FillType
        @return      fill, or kWinding_FillType or kEvenOdd_FillType if fill is inverted
    */
    static FillType ConvertToNonInverseFillType(FillType fill) {
        static_assert(0 == kWinding_FillType, "fill_type_mismatch");
        static_assert(1 == kEvenOdd_FillType, "fill_type_mismatch");
        static_assert(2 == kInverseWinding_FillType, "fill_type_mismatch");
        static_assert(3 == kInverseEvenOdd_FillType, "fill_type_mismatch");
        return (FillType)(fill & 1);
    }

    /** Approximates conic with quad array. Conic is constructed from start SkPoint p0,
        control SkPoint p1, end SkPoint p2, and weight w.
        Quad array is stored in pts; this storage is supplied by caller.
        Maximum quad count is 2 to the pow2.
        Every third point in array shares last SkPoint of previous quad and first SkPoint of
        next quad. Maximum pts storage size is given by:
        (1 + 2 * (1 << pow2)) * sizeof(SkPoint).

        Returns quad count used the approximation, which may be smaller
        than the number requested.

        conic weight determines the amount of influence conic control point has on the curve.
        w less than one represents an elliptical section. w greater than one represents
        a hyperbolic section. w equal to one represents a parabolic section.

        Two quad curves are sufficient to approximate an elliptical conic with a sweep
        of up to 90 degrees; in this case, set pow2 to one.

        @param p0    conic start SkPoint
        @param p1    conic control SkPoint
        @param p2    conic end SkPoint
        @param w     conic weight
        @param pts   storage for quad array
        @param pow2  quad count, as power of two, normally 0 to 5 (1 to 32 quad curves)
        @return      number of quad curves written to pts
    */
    static int ConvertConicToQuads(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                                   SkScalar w, SkPoint pts[], int pow2);

    /** Returns true if SkPath is equivalent to SkRect when filled.
        If false: rect, isClosed, and direction are unchanged.
        If true: rect, isClosed, and direction are written to if not nullptr.

        rect may be smaller than the SkPath bounds. SkPath bounds may include kMove_Verb points
        that do not alter the area drawn by the returned rect.

        @param rect       storage for bounds of SkRect; may be nullptr
        @param isClosed   storage set to true if SkPath is closed; may be nullptr
        @param direction  storage set to SkRect direction; may be nullptr
        @return           true if SkPath contains SkRect
    */
    bool isRect(SkRect* rect, bool* isClosed = nullptr, Direction* direction = nullptr) const;

    /** Returns true if SkPath is equivalent to nested SkRect pair when filled.
        If false, rect and dirs are unchanged.
        If true, rect and dirs are written to if not nullptr:
        setting rect[0] to outer SkRect, and rect[1] to inner SkRect;
        setting dirs[0] to SkPath::Direction of outer SkRect, and dirs[1] to SkPath::Direction of
        inner SkRect.

        @param rect  storage for SkRect pair; may be nullptr
        @param dirs  storage for SkPath::Direction pair; may be nullptr
        @return      true if SkPath contains nested SkRect pair
    */
    bool isNestedFillRects(SkRect rect[2], Direction dirs[2] = nullptr) const;

    /** Adds SkRect to SkPath, appending kMove_Verb, three kLine_Verb, and kClose_Verb,
        starting with top-left corner of SkRect; followed by top-right, bottom-right,
        and bottom-left if dir is kCW_Direction; or followed by bottom-left,
        bottom-right, and top-right if dir is kCCW_Direction.

        @param rect  SkRect to add as a closed contour
        @param dir   SkPath::Direction to wind added contour
        @return      reference to SkPath
    */
    SkPath& addRect(const SkRect& rect, Direction dir = kCW_Direction);

    /** Adds SkRect to SkPath, appending kMove_Verb, three kLine_Verb, and kClose_Verb.
        If dir is kCW_Direction, SkRect corners are added clockwise; if dir is
        kCCW_Direction, SkRect corners are added counterclockwise.
        start determines the first corner added.

        @param rect   SkRect to add as a closed contour
        @param dir    SkPath::Direction to wind added contour
        @param start  initial corner of SkRect to add
        @return       reference to SkPath
    */
    SkPath& addRect(const SkRect& rect, Direction dir, unsigned start);

    /** Adds SkRect (left, top, right, bottom) to SkPath,
        appending kMove_Verb, three kLine_Verb, and kClose_Verb,
        starting with top-left corner of SkRect; followed by top-right, bottom-right,
        and bottom-left if dir is kCW_Direction; or followed by bottom-left,
        bottom-right, and top-right if dir is kCCW_Direction.

        @param left    smaller x-axis value of SkRect
        @param top     smaller y-axis value of SkRect
        @param right   larger x-axis value of SkRect
        @param bottom  larger y-axis value of SkRect
        @param dir     SkPath::Direction to wind added contour
        @return        reference to SkPath
    */
    SkPath& addRect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom,
                    Direction dir = kCW_Direction);

    /** Adds oval to path, appending kMove_Verb, four kConic_Verb, and kClose_Verb.
        Oval is upright ellipse bounded by SkRect oval with radii equal to half oval width
        and half oval height. Oval begins at (oval.fRight, oval.centerY()) and continues
        clockwise if dir is kCW_Direction, counterclockwise if dir is kCCW_Direction.

        @param oval  bounds of ellipse added
        @param dir   SkPath::Direction to wind ellipse
        @return      reference to SkPath
    */
    SkPath& addOval(const SkRect& oval, Direction dir = kCW_Direction);

    /** Adds oval to SkPath, appending kMove_Verb, four kConic_Verb, and kClose_Verb.
        Oval is upright ellipse bounded by SkRect oval with radii equal to half oval width
        and half oval height. Oval begins at start and continues
        clockwise if dir is kCW_Direction, counterclockwise if dir is kCCW_Direction.

        @param oval   bounds of ellipse added
        @param dir    SkPath::Direction to wind ellipse
        @param start  index of initial point of ellipse
        @return       reference to SkPath
    */
    SkPath& addOval(const SkRect& oval, Direction dir, unsigned start);

    /** Adds circle centered at (x, y) of size radius to SkPath, appending kMove_Verb,
        four kConic_Verb, and kClose_Verb. Circle begins at: (x + radius, y), continuing
        clockwise if dir is kCW_Direction, and counterclockwise if dir is kCCW_Direction.

        Has no effect if radius is zero or negative.

        @param x       center of circle
        @param y       center of circle
        @param radius  distance from center to edge
        @param dir     SkPath::Direction to wind circle
        @return        reference to SkPath
    */
    SkPath& addCircle(SkScalar x, SkScalar y, SkScalar radius,
                      Direction dir = kCW_Direction);

    /** Appends arc to SkPath, as the start of new contour. Arc added is part of ellipse
        bounded by oval, from startAngle through sweepAngle. Both startAngle and
        sweepAngle are measured in degrees, where zero degrees is aligned with the
        positive x-axis, and positive sweeps extends arc clockwise.

        If sweepAngle <= -360, or sweepAngle >= 360; and startAngle modulo 90 is nearly
        zero, append oval instead of arc. Otherwise, sweepAngle values are treated
        modulo 360, and arc may or may not draw depending on numeric rounding.

        @param oval        bounds of ellipse containing arc
        @param startAngle  starting angle of arc in degrees
        @param sweepAngle  sweep, in degrees. Positive is clockwise; treated modulo 360
        @return            reference to SkPath
    */
    SkPath& addArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle);

    /** Appends SkRRect to SkPath, creating a new closed contour. SkRRect has bounds
        equal to rect; each corner is 90 degrees of an ellipse with radii (rx, ry). If
        dir is kCW_Direction, SkRRect starts at top-left of the lower-left corner and
        winds clockwise. If dir is kCCW_Direction, SkRRect starts at the bottom-left
        of the upper-left corner and winds counterclockwise.

        If either rx or ry is too large, rx and ry are scaled uniformly until the
        corners fit. If rx or ry is less than or equal to zero, addRoundRect() appends
        SkRect rect to SkPath.

        After appending, SkPath may be empty, or may contain: SkRect, oval, or SkRRect.

        @param rect  bounds of SkRRect
        @param rx    x-axis radius of rounded corners on the SkRRect
        @param ry    y-axis radius of rounded corners on the SkRRect
        @param dir   SkPath::Direction to wind SkRRect
        @return      reference to SkPath
    */
    SkPath& addRoundRect(const SkRect& rect, SkScalar rx, SkScalar ry,
                         Direction dir = kCW_Direction);

    /** Appends SkRRect to SkPath, creating a new closed contour. SkRRect has bounds
        equal to rect; each corner is 90 degrees of an ellipse with radii from the
        array.

        @param rect   bounds of SkRRect
        @param radii  array of 8 SkScalar values, a radius pair for each corner
        @param dir    SkPath::Direction to wind SkRRect
        @return       reference to SkPath
    */
    SkPath& addRoundRect(const SkRect& rect, const SkScalar radii[],
                         Direction dir = kCW_Direction);

    /** Adds rrect to SkPath, creating a new closed contour. If
        dir is kCW_Direction, rrect starts at top-left of the lower-left corner and
        winds clockwise. If dir is kCCW_Direction, rrect starts at the bottom-left
        of the upper-left corner and winds counterclockwise.

        After appending, SkPath may be empty, or may contain: SkRect, oval, or SkRRect.

        @param rrect  bounds and radii of rounded rectangle
        @param dir    SkPath::Direction to wind SkRRect
        @return       reference to SkPath
    */
    SkPath& addRRect(const SkRRect& rrect, Direction dir = kCW_Direction);

    /** Adds rrect to SkPath, creating a new closed contour. If dir is kCW_Direction, rrect
        winds clockwise; if dir is kCCW_Direction, rrect winds counterclockwise.
        start determines the first point of rrect to add.

        @param rrect  bounds and radii of rounded rectangle
        @param dir    SkPath::Direction to wind SkRRect
        @param start  index of initial point of SkRRect
        @return       reference to SkPath
    */
    SkPath& addRRect(const SkRRect& rrect, Direction dir, unsigned start);

    /** Adds contour created from line array, adding (count - 1) line segments.
        Contour added starts at pts[0], then adds a line for every additional SkPoint
        in pts array. If close is true, appends kClose_Verb to SkPath, connecting
        pts[count - 1] and pts[0].

        If count is zero, append kMove_Verb to path.
        Has no effect if count is less than one.

        @param pts    array of line sharing end and start SkPoint
        @param count  length of SkPoint array
        @param close  true to add line connecting contour end and start
        @return       reference to SkPath
    */
    SkPath& addPoly(const SkPoint pts[], int count, bool close);

    /** Adds contour created from list. Contour added starts at list[0], then adds a line
        for every additional SkPoint in list. If close is true, appends kClose_Verb to SkPath,
        connecting last and first SkPoint in list.

        If list is empty, append kMove_Verb to path.

        @param list   array of SkPoint
        @param close  true to add line connecting contour end and start
        @return       reference to SkPath
    */
    SkPath& addPoly(const std::initializer_list<SkPoint>& list, bool close) {
        return this->addPoly(list.begin(), SkToInt(list.size()), close);
    }

    /** \enum SkPath::AddPathMode
        AddPathMode chooses how addPath() appends. Adding one SkPath to another can extend
        the last contour or start a new contour.
    */
    enum AddPathMode {
        kAppend_AddPathMode, //!< appended to destination unaltered
        kExtend_AddPathMode, //!< add line if prior contour is not closed
    };

    /** Appends src to SkPath, offset by (dx, dy).

        If mode is kAppend_AddPathMode, src verb array, SkPoint array, and conic weights are
        added unaltered. If mode is kExtend_AddPathMode, add line before appending
        verbs, SkPoint, and conic weights.

        @param src   SkPath verbs, SkPoint, and conic weights to add
        @param dx    offset added to src SkPoint array x-axis coordinates
        @param dy    offset added to src SkPoint array y-axis coordinates
        @param mode  kAppend_AddPathMode or kExtend_AddPathMode
        @return      reference to SkPath
    */
    SkPath& addPath(const SkPath& src, SkScalar dx, SkScalar dy,
                    AddPathMode mode = kAppend_AddPathMode);

    /** Appends src to SkPath.

        If mode is kAppend_AddPathMode, src verb array, SkPoint array, and conic weights are
        added unaltered. If mode is kExtend_AddPathMode, add line before appending
        verbs, SkPoint, and conic weights.

        @param src   SkPath verbs, SkPoint, and conic weights to add
        @param mode  kAppend_AddPathMode or kExtend_AddPathMode
        @return      reference to SkPath
    */
    SkPath& addPath(const SkPath& src, AddPathMode mode = kAppend_AddPathMode) {
        SkMatrix m;
        m.reset();
        return this->addPath(src, m, mode);
    }

    /** Appends src to SkPath, transformed by matrix. Transformed curves may have different
        verbs, SkPoint, and conic weights.

        If mode is kAppend_AddPathMode, src verb array, SkPoint array, and conic weights are
        added unaltered. If mode is kExtend_AddPathMode, add line before appending
        verbs, SkPoint, and conic weights.

        @param src     SkPath verbs, SkPoint, and conic weights to add
        @param matrix  transform applied to src
        @param mode    kAppend_AddPathMode or kExtend_AddPathMode
        @return        reference to SkPath
    */
    SkPath& addPath(const SkPath& src, const SkMatrix& matrix,
                    AddPathMode mode = kAppend_AddPathMode);

    /** Appends src to SkPath, from back to front.
        Reversed src always appends a new contour to SkPath.

        @param src  SkPath verbs, SkPoint, and conic weights to add
        @return     reference to SkPath
    */
    SkPath& reverseAddPath(const SkPath& src);

    /** Offsets SkPoint array by (dx, dy). Offset SkPath replaces dst.
        If dst is nullptr, SkPath is replaced by offset data.

        @param dx   offset added to SkPoint array x-axis coordinates
        @param dy   offset added to SkPoint array y-axis coordinates
        @param dst  overwritten, translated copy of SkPath; may be nullptr
    */
    void offset(SkScalar dx, SkScalar dy, SkPath* dst) const;

    /** Offsets SkPoint array by (dx, dy). SkPath is replaced by offset data.

        @param dx  offset added to SkPoint array x-axis coordinates
        @param dy  offset added to SkPoint array y-axis coordinates
    */
    void offset(SkScalar dx, SkScalar dy) {
        this->offset(dx, dy, this);
    }

    /** Transforms verb array, SkPoint array, and weight by matrix.
        transform may change verbs and increase their number.
        Transformed SkPath replaces dst; if dst is nullptr, original data
        is replaced.

        @param matrix  SkMatrix to apply to SkPath
        @param dst     overwritten, transformed copy of SkPath; may be nullptr
    */
    void transform(const SkMatrix& matrix, SkPath* dst) const;

    /** Transforms verb array, SkPoint array, and weight by matrix.
        transform may change verbs and increase their number.
        SkPath is replaced by transformed data.

        @param matrix  SkMatrix to apply to SkPath
    */
    void transform(const SkMatrix& matrix) {
        this->transform(matrix, this);
    }

    /** Returns last point on SkPath in lastPt. Returns false if SkPoint array is empty,
        storing (0, 0) if lastPt is not nullptr.

        @param lastPt  storage for final SkPoint in SkPoint array; may be nullptr
        @return        true if SkPoint array contains one or more SkPoint
    */
    bool getLastPt(SkPoint* lastPt) const;

    /** Sets last point to (x, y). If SkPoint array is empty, append kMove_Verb to
        verb array and append (x, y) to SkPoint array.

        @param x  set x-axis value of last point
        @param y  set y-axis value of last point
    */
    void setLastPt(SkScalar x, SkScalar y);

    /** Sets the last point on the path. If SkPoint array is empty, append kMove_Verb to
        verb array and append p to SkPoint array.

        @param p  set value of last point
    */
    void setLastPt(const SkPoint& p) {
        this->setLastPt(p.fX, p.fY);
    }

    /** \enum SkPath::SegmentMask
        SegmentMask constants correspond to each drawing Verb type in SkPath; for
        instance, if SkPath only contains lines, only the kLine_SegmentMask bit is set.
    */
    enum SegmentMask {
        kLine_SegmentMask  = 1 << 0, //!< contains one or more lines
        kQuad_SegmentMask  = 1 << 1, //!< contains one or more quads
        kConic_SegmentMask = 1 << 2, //!< contains one or more conics
        kCubic_SegmentMask = 1 << 3, //!< contains one or more cubics
    };

    /** Returns a mask, where each set bit corresponds to a SegmentMask constant
        if SkPath contains one or more verbs of that type.
        Returns zero if SkPath contains no lines, or curves: quads, conics, or cubics.

        getSegmentMasks() returns a cached result; it is very fast.

        @return  SegmentMask bits or zero
    */
    uint32_t getSegmentMasks() const { return fPathRef->getSegmentMasks(); }

    /** \enum SkPath::Verb
        Verb instructs SkPath how to interpret one or more SkPoint and optional conic weight;
        manage contour, and terminate SkPath.
    */
    enum Verb {
        kMove_Verb,  //!< starts new contour at next SkPoint
        kLine_Verb,  //!< adds line from last point to next SkPoint
        kQuad_Verb,  //!< adds quad from last point
        kConic_Verb, //!< adds conic from last point
        kCubic_Verb, //!< adds cubic from last point
        kClose_Verb, //!< closes contour
        kDone_Verb,  //!< terminates SkPath
    };

    /** \class SkPath::Iter
        Iterates through verb array, and associated SkPoint array and conic weight.
        Provides options to treat open contours as closed, and to ignore
        degenerate data.
    */
    class SK_API Iter {
    public:

        /** Initializes SkPath::Iter with an empty SkPath. next() on SkPath::Iter returns
            kDone_Verb.
            Call setPath to initialize SkPath::Iter at a later time.

            @return  SkPath::Iter of empty SkPath
        */
        Iter();

        /** Sets SkPath::Iter to return elements of verb array, SkPoint array, and conic weight in
            path. If forceClose is true, SkPath::Iter will add kLine_Verb and kClose_Verb after each
            open contour. path is not altered.

            @param path        SkPath to iterate
            @param forceClose  true if open contours generate kClose_Verb
            @return            SkPath::Iter of path
        */
        Iter(const SkPath& path, bool forceClose);

        /** Sets SkPath::Iter to return elements of verb array, SkPoint array, and conic weight in
            path. If forceClose is true, SkPath::Iter will add kLine_Verb and kClose_Verb after each
            open contour. path is not altered.

            @param path        SkPath to iterate
            @param forceClose  true if open contours generate kClose_Verb
        */
        void setPath(const SkPath& path, bool forceClose);

        /** Returns next SkPath::Verb in verb array, and advances SkPath::Iter.
            When verb array is exhausted, returns kDone_Verb.

            Zero to four SkPoint are stored in pts, depending on the returned SkPath::Verb.

            @param pts  storage for SkPoint data describing returned SkPath::Verb
            @return     next SkPath::Verb from verb array
        */
        Verb next(SkPoint pts[4]);

        // DEPRECATED
        Verb next(SkPoint pts[4], bool /*doConsumeDegenerates*/, bool /*exact*/ = false) {
            return this->next(pts);
        }

        /** Returns conic weight if next() returned kConic_Verb.

            If next() has not been called, or next() did not return kConic_Verb,
            result is undefined.

            @return  conic weight for conic SkPoint returned by next()
        */
        SkScalar conicWeight() const { return *fConicWeights; }

        /** Returns true if last kLine_Verb returned by next() was generated
            by kClose_Verb. When true, the end point returned by next() is
            also the start point of contour.

            If next() has not been called, or next() did not return kLine_Verb,
            result is undefined.

            @return  true if last kLine_Verb was generated by kClose_Verb
        */
        bool isCloseLine() const { return SkToBool(fCloseLine); }

        /** Returns true if subsequent calls to next() return kClose_Verb before returning
            kMove_Verb. if true, contour SkPath::Iter is processing may end with kClose_Verb, or
            SkPath::Iter may have been initialized with force close set to true.

            @return  true if contour is closed
        */
        bool isClosedContour() const;

    private:
        const SkPoint*  fPts;
        const uint8_t*  fVerbs;
        const uint8_t*  fVerbStop;
        const SkScalar* fConicWeights;
        SkPoint         fMoveTo;
        SkPoint         fLastPt;
        bool            fForceClose;
        bool            fNeedClose;
        bool            fCloseLine;
        enum SegmentState : uint8_t {
            /** The current contour is empty. Starting processing or have just closed a contour. */
            kEmptyContour_SegmentState,
            /** Have seen a move, but nothing else. */
            kAfterMove_SegmentState,
            /** Have seen a primitive but not yet closed the path. Also the initial state. */
            kAfterPrimitive_SegmentState
        };
        SegmentState    fSegmentState;

        inline const SkPoint& cons_moveTo();
        Verb autoClose(SkPoint pts[2]);
    };

    /** \class SkPath::RawIter
        Iterates through verb array, and associated SkPoint array and conic weight.
        verb array, SkPoint array, and conic weight are returned unaltered.
    */
    class SK_API RawIter {
    public:

        /** Initializes RawIter with an empty SkPath. next() on RawIter returns kDone_Verb.
            Call setPath to initialize SkPath::Iter at a later time.

            @return  RawIter of empty SkPath
        */
        RawIter() {}

        /** Sets RawIter to return elements of verb array, SkPoint array, and conic weight in path.

            @param path  SkPath to iterate
            @return      RawIter of path
        */
        RawIter(const SkPath& path) {
            setPath(path);
        }

        /** Sets SkPath::Iter to return elements of verb array, SkPoint array, and conic weight in
            path.

            @param path  SkPath to iterate
        */
        void setPath(const SkPath& path) {
            fRawIter.setPathRef(*path.fPathRef.get());
        }

        /** Returns next SkPath::Verb in verb array, and advances RawIter.
            When verb array is exhausted, returns kDone_Verb.
            Zero to four SkPoint are stored in pts, depending on the returned SkPath::Verb.

            @param pts  storage for SkPoint data describing returned SkPath::Verb
            @return     next SkPath::Verb from verb array
        */
        Verb next(SkPoint pts[4]) {
            return (Verb) fRawIter.next(pts);
        }

        /** Returns next SkPath::Verb, but does not advance RawIter.

            @return  next SkPath::Verb from verb array
        */
        Verb peek() const {
            return (Verb) fRawIter.peek();
        }

        /** Returns conic weight if next() returned kConic_Verb.

            If next() has not been called, or next() did not return kConic_Verb,
            result is undefined.

            @return  conic weight for conic SkPoint returned by next()
        */
        SkScalar conicWeight() const {
            return fRawIter.conicWeight();
        }

    private:
        SkPathRef::Iter fRawIter;
        friend class SkPath;

    };

    /** Returns true if the point (x, y) is contained by SkPath, taking into
        account FillType.

        @param x  x-axis value of containment test
        @param y  y-axis value of containment test
        @return   true if SkPoint is in SkPath
    */
    bool contains(SkScalar x, SkScalar y) const;

    /** Writes text representation of SkPath to stream. If stream is nullptr, writes to
        standard output. Set forceClose to true to get edges used to fill SkPath.
        Set dumpAsHex true to generate exact binary representations
        of floating point numbers used in SkPoint array and conic weights.

        @param stream      writable SkWStream receiving SkPath text representation; may be nullptr
        @param forceClose  true if missing kClose_Verb is output
        @param dumpAsHex   true if SkScalar values are written as hexadecimal
    */
    void dump(SkWStream* stream, bool forceClose, bool dumpAsHex) const;
    void dump(std::string &desc, int depth) const;
    /** Writes text representation of SkPath to standard output. The representation may be
        directly compiled as C++ code. Floating point values are written
        with limited precision; it may not be possible to reconstruct original SkPath
        from output.
    */
    void dump() const;

    /** Writes text representation of SkPath to standard output. The representation may be
        directly compiled as C++ code. Floating point values are written
        in hexadecimal to preserve their exact bit pattern. The output reconstructs the
        original SkPath.

        Use instead of dump() when submitting
    */
    void dumpHex() const;

    /** Writes SkPath to buffer, returning the number of bytes written.
        Pass nullptr to obtain the storage size.

        Writes SkPath::FillType, verb array, SkPoint array, conic weight, and
        additionally writes computed information like SkPath::Convexity and bounds.

        Use only be used in concert with readFromMemory();
        the format used for SkPath in memory is not guaranteed.

        @param buffer  storage for SkPath; may be nullptr
        @return        size of storage required for SkPath; always a multiple of 4
    */
    size_t writeToMemory(void* buffer) const;

    /** Writes SkPath to buffer, returning the buffer written to, wrapped in SkData.

        serialize() writes SkPath::FillType, verb array, SkPoint array, conic weight, and
        additionally writes computed information like SkPath::Convexity and bounds.

        serialize() should only be used in concert with readFromMemory().
        The format used for SkPath in memory is not guaranteed.

        @return  SkPath data wrapped in SkData buffer
    */
    sk_sp<SkData> serialize() const;

    /** Initializes SkPath from buffer of size length. Returns zero if the buffer is
        data is inconsistent, or the length is too small.

        Reads SkPath::FillType, verb array, SkPoint array, conic weight, and
        additionally reads computed information like SkPath::Convexity and bounds.

        Used only in concert with writeToMemory();
        the format used for SkPath in memory is not guaranteed.

        @param buffer  storage for SkPath
        @param length  buffer size in bytes; must be multiple of 4
        @return        number of bytes read, or zero on failure
    */
    size_t readFromMemory(const void* buffer, size_t length);

    /** (See Skia bug 1762.)
        Returns a non-zero, globally unique value. A different value is returned
        if verb array, SkPoint array, or conic weight changes.

        Setting SkPath::FillType does not change generation identifier.

        Each time the path is modified, a different generation identifier will be returned.
        SkPath::FillType does affect generation identifier on Android framework.

        @return  non-zero, globally unique value
    */
    uint32_t getGenerationID() const;

    /** Returns if SkPath data is consistent. Corrupt SkPath data is detected if
        internal values are out of range or internal storage does not match
        array dimensions.

        @return  true if SkPath data is consistent
    */
    bool isValid() const { return this->isValidImpl() && fPathRef->isValid(); }

private:
    sk_sp<SkPathRef>               fPathRef;
    int                            fLastMoveToIndex;
    mutable std::atomic<Convexity> fConvexity;
    mutable std::atomic<uint8_t>   fFirstDirection; // really an SkPathPriv::FirstDirection
    uint8_t                        fFillType    : 2;
    uint8_t                        fIsVolatile  : 1;

    /** Resets all fields other than fPathRef to their initial 'empty' values.
     *  Assumes the caller has already emptied fPathRef.
     *  On Android increments fGenerationID without reseting it.
     */
    void resetFields();

    /** Sets all fields other than fPathRef to the values in 'that'.
     *  Assumes the caller has already set fPathRef.
     *  Doesn't change fGenerationID or fSourcePath on Android.
     */
    void copyFields(const SkPath& that);

    size_t writeToMemoryAsRRect(void* buffer) const;
    size_t readAsRRect(const void*, size_t);
    size_t readFromMemory_LE3(const void*, size_t);
    size_t readFromMemory_EQ4(const void*, size_t);

    friend class Iter;
    friend class SkPathPriv;
    friend class SkPathStroker;

    /*  Append, in reverse order, the first contour of path, ignoring path's
        last point. If no moveTo() call has been made for this contour, the
        first point is automatically set to (0,0).
    */
    SkPath& reversePathTo(const SkPath&);

    // called before we add points for lineTo, quadTo, cubicTo, checking to see
    // if we need to inject a leading moveTo first
    //
    //  SkPath path; path.lineTo(...);   <--- need a leading moveTo(0, 0)
    // SkPath path; ... path.close(); path.lineTo(...) <-- need a moveTo(previous moveTo)
    //
    inline void injectMoveToIfNeeded();

    inline bool hasOnlyMoveTos() const;

    Convexity internalGetConvexity() const;

    /** Asserts if SkPath data is inconsistent.
        Debugging check intended for internal use only.
     */
    SkDEBUGCODE(void validate() const { SkASSERT(this->isValidImpl()); } )
    bool isValidImpl() const;
    SkDEBUGCODE(void validateRef() const { fPathRef->validate(); } )

    bool isRectContour(bool allowPartial, int* currVerb, const SkPoint** pts,
                       bool* isClosed, Direction* direction, SkRect* rect) const;

    // called by stroker to see if all points (in the last contour) are equal and worthy of a cap
    bool isZeroLengthSincePoint(int startPtIndex) const;

    /** Returns if the path can return a bound at no cost (true) or will have to
        perform some computation (false).
     */
    bool hasComputedBounds() const {
        SkDEBUGCODE(this->validate();)
        return fPathRef->hasComputedBounds();
    }


    // 'rect' needs to be sorted
    void setBounds(const SkRect& rect) {
        SkPathRef::Editor ed(&fPathRef);

        ed.setBounds(rect);
    }

    void setPt(int index, SkScalar x, SkScalar y);

    // Bottlenecks for working with fConvexity and fFirstDirection.
    // Notice the setters are const... these are mutable atomic fields.
    void    setConvexity(Convexity) const;
    void    setFirstDirection(uint8_t) const;
    uint8_t getFirstDirection() const;

    friend class SkAutoPathBoundsUpdate;
    friend class SkAutoDisableOvalCheck;
    friend class SkAutoDisableDirectionCheck;
    friend class SkPathWriter;
    friend class SkOpBuilder;
    friend class SkBench_AddPathTest; // perf test reversePathTo
    friend class PathTest_Private; // unit test reversePathTo
    friend class ForceIsRRect_Private; // unit test isRRect
    friend class FuzzPath; // for legacy access to validateRef
};

#endif
