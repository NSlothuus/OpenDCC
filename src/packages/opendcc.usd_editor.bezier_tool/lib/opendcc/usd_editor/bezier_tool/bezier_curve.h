/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/usdGeom/basisCurves.h>

#include "opendcc/app/viewport/viewport_view.h"
#include "opendcc/app/viewport/viewport_ui_draw_manager.h"

#include <vector>
#include <memory>

OPENDCC_NAMESPACE_OPEN

/**
 * @brief This class stores information about the shape and points of a Bezier curve.
 * It also provides methods for editing and displaying it in a viewport.
 */
class BezierCurve
{
public:
    /**
     * @brief Represents a BezierCurve's point in three-dimensional space.
     */
    struct Point
    {
        Point() = default;
        Point(const PXR_NS::GfVec3f& p);
        Point(const PXR_NS::GfVec3f& l, const PXR_NS::GfVec3f& p, const PXR_NS::GfVec3f& r);

        PXR_NS::GfVec3f ltangent; /**< The 'left' tangent vector, which belongs to the curve's segment with number n*/
        PXR_NS::GfVec3f point; /**< The coordinates of the point in three-dimensional space. */
        PXR_NS::GfVec3f rtangent; /**< The 'right' tangent vector, which belongs to the curve's segment with number n+1*/
    };

    /**
     * @brief Determines the way of updating the tangents of a BezierCurve's point in the 'update_point' method.
     */
    enum class TangentMode
    {
        /**
         * @brief In this mode, after updating the tangents, they will be edited as a whole staying on the same line, thus
         * the distance from 'ltangent' and 'rtangent' to 'point' remains equal and the tangents are kept symmetrical.
         */
        Normal,
        /**
         * @brief In this mode, after updating the tangents, they will be edited as a whole staying on the same line,
         * yet the distance from 'ltangent' and 'rtangent' to 'point' may be different.
         */
        Weighted,
        /**
         * @brief In this mode, the tangents are edited independently of each other.
         */
        Tangent,
    };

    struct Tangent
    {
        enum class Type
        {
            Left,
            Right,
            Unknown
        };

        size_t point_index = s_invalid_index;
        Type type = Type::Unknown;

        friend bool operator==(const Tangent& l, const Tangent& r);
        friend bool operator!=(const Tangent& l, const Tangent& r);
    };

    constexpr static size_t s_invalid_index = -1;

    BezierCurve();
    BezierCurve(const PXR_NS::UsdGeomBasisCurves& curve);

    /**
     * @brief Removes the specified point from a BezierCurve.
     *
     * @note Removing the first point from a closed (periodic) curve
     * will result in transforming it to an open (nonperiodic) curve
     * by removing the segment constructed with the points 0 and 1.
     */
    void remove_point(const size_t point_index);

    /**
     * @brief Inserts a 'point' into the BezierСurve using the 'point_index' position.
     *
     * @note Using the `get_point` method after inserting a point with the 'point_index' argument returns the added point.
     */
    void insert_point(const size_t point_index, const BezierCurve::Point& point);

    /**
     * @brief Adds a point to the end of the Bezier curve.
     */
    void push_back(const BezierCurve::Point& point);

    /**
     * @brief Updates the `ltangent` and `rtangent` points with the specified `point_index`.
     */
    void update_tangents(const size_t point_index, const PXR_NS::GfVec3f& ltangent, const PXR_NS::GfVec3f& rtangent);

    /**
     * @brief Sets a value for the point with the specified `point_index`.
     */
    void set_point(const size_t point_index, const BezierCurve::Point& point);

    /**
     * @brief Updates the specified tangent's coordinates and re-computes the position of the opposite tangent
     * in the selected TangentMode.
     */
    void update_point(const BezierCurve::Tangent& tangent, const PXR_NS::GfVec3f& new_tangent, TangentMode mode);

    /**
     * @brief Returns the number BezierCurve's control points.
     */
    size_t size();

    /**
     * @brief Removes all the control points from the BezierCurve.
     */
    void clear();

    /**
     * @brief Makes a curve open (nonperiodic). Does nothing if it is already open (nonperiodic).
     *
     * @note Removes the curve segment formed by the last and first points.
     */
    void open();

    /**
     * @brief Makes a curve closed (periodic). Does nothing if it is already closed (periodic).
     *
     * @note Forms a new segment using the last and first points.
     */
    void close();

    /**
     * @brief Checks whether the curve is closed.
     * @return true if the curve is closed, false otherwise.
     */
    bool is_close();

    /**
     * @brief Checks whether the curve is empty.
     * @return True if the curve does not contain any points, false otherwise.
     */
    bool is_empty();

    /**
     * @brief Removes the last point of the curve.
     */
    void pop_back();

    /**
     * @brief Gets the point with the specified `point_index`.
     *
     * @return Point index.
     */
    BezierCurve::Point get_point(const size_t point_index);

    /**
     * @brief Gets the last point of the curve.
     *
     * @note If the curve is not closed (nonperiodic), the `ltangent` will be approximated in `TangentMode::Normal' mode.
     */
    BezierCurve::Point first();

    /**
     * @brief Gets the first point of the curve.
     *
     * @note If the curve is not closed (nonperiodic), the `rtangent` will be approximated in 'TangentMode::Normal' mode
     */
    BezierCurve::Point last();

    /**
     * @brief Checks if the specified `ray` intersects the BezierCurve's point.
     *
     * @return True if the ray intersects any point of the BezierCurve, false otherwise.
     *
     * @note If the 'point_index' argument is passed and the curve point intersects with the ray,
     * the 'point_index' is set to the intersection point index, 's_invalid_index' otherwise.
     */
    bool intersect_curve_point(const PXR_NS::GfRay& ray, const ViewportViewPtr& viewport_view, size_t* point_index = nullptr);

    /**
     * @brief Checks whether the specified `ray` intersects the BezierCurve's curve.
     *
     * @return True if the ray intersects any tangent of the BezierCurve, false otherwise.
     *
     * If the argument `tangent_info` is passed, information about the intersected tangent will be recorded in it.
     * Otherwise, `tangent_info->point_index = s_invalid_index` and `tangent_info.type = Tangent::Type::Unknown`.
     */
    bool intersect_curve_tangent(const PXR_NS::GfRay& ray, const ViewportViewPtr& viewport_view, Tangent* tangent_info = nullptr);

    /**
     * @brief Returns the path to the `BezierCurve` UsdPrim.
     */
    const PXR_NS::SdfPath& get_path();

    void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager);

    /**
     * @brief Sets the point with the specified `index` as selected.
     * It is used in the 'draw' method.
     */
    void set_select_point(size_t index);

    /**
     * @brief Sets the point with the specified `index` as intersected.
     * It is used in the 'draw' method.
     */
    void set_intersect_point(size_t index);

    /**
     * @brief Sets the tangent with the specified tangent information as intersected.
     * It is used in the 'draw' method.
     */
    void set_intersect_tangent(const Tangent& tangent);

    /**
     * @brief Sets the tangent with the specified tangent information as selected.
     * It is used in the 'draw' method.
     */
    void set_select_tangent(const Tangent& tangent);

    PXR_NS::GfMatrix4d compute_model_matrix() const;

private:
    void draw_tangents(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager, const int point_index,
                       const PXR_NS::GfVec3f& ltangent_color, const PXR_NS::GfVec3f& rtangent_color);

    void draw_screen_rect(const PXR_NS::GfVec3f& world_rect_center, const PXR_NS::GfMatrix4d view_projection, ViewportUiDrawManager* draw_manager,
                          const PXR_NS::GfVec2f& offset, const PXR_NS::GfVec3f& color);
    void draw_screen_line(const PXR_NS::GfVec3f& world_line_begin, const PXR_NS::GfVec3f& world_line_end, const PXR_NS::GfMatrix4d view_projection,
                          ViewportUiDrawManager* draw_manager, const PXR_NS::GfVec3f& color);

    bool is_intersect(const PXR_NS::GfVec3f& curve_point, const PXR_NS::GfVec3f& point, const ViewportViewPtr& viewport_view);

    void update_usd();

    void periodic_update(const size_t point_index, const BezierCurve::Point& point);
    void periodic_remove(const size_t point_index);

    PXR_NS::GfVec3f compute_first_tangent();
    PXR_NS::GfVec3f compute_last_tangent();

    // https://openusd.org/dev/api/class_usd_geom_basis_curves.html
    constexpr static int vstep = 3;
    constexpr static int segment_points_count = 4;

    /**
     * @brief It stores points the same way as it is written in the USD documentation.
     * https://openusd.org/dev/api/class_usd_geom_basis_curves.html
     */
    PXR_NS::VtVec3fArray m_points;

    PXR_NS::SdfPath m_usd_path;
    PXR_NS::UsdGeomBasisCurves m_usd_curve;
    bool m_periodic = false;

    size_t m_select_point = s_invalid_index;
    size_t m_intersect_point = s_invalid_index;
    Tangent m_intersect_tangent = Tangent();
    Tangent m_select_tangent = Tangent();
};

using BezierCurvePtr = std::shared_ptr<BezierCurve>;

OPENDCC_NAMESPACE_CLOSE
