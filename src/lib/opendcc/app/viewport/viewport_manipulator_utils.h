/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <pxr/base/gf/transform.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include "opendcc/app/viewport/viewport_view.h"
#include "opendcc/app/core/undo/inverse.h"

#include <tuple>

OPENDCC_NAMESPACE_OPEN

namespace manipulator_utils
{
    OPENDCC_API std::array<int, 3> get_basis_indices_from_rot_order(PXR_NS::UsdGeomXformCommonAPI::RotationOrder rotation_order);
    OPENDCC_API double compute_screen_factor(const ViewportViewPtr& viewport_view, const PXR_NS::GfVec3d& center);
    OPENDCC_API PXR_NS::UsdTimeCode get_non_varying_time(const PXR_NS::UsdAttribute& attr);
    OPENDCC_API PXR_NS::GfRay compute_pick_ray(const ViewportViewPtr& viewport_view, int x, int y);
    OPENDCC_API bool compute_axis_intersection(const ViewportViewPtr& viewport_view, const PXR_NS::GfVec3d& gizmo_world_pos,
                                               const PXR_NS::GfVec3d& direction, const PXR_NS::GfMatrix4d& view_proj, int x, int y,
                                               PXR_NS::GfVec3d& result);
    OPENDCC_API bool compute_plane_intersection(const ViewportViewPtr& viewport_view, const PXR_NS::GfVec3d& gizmo_world_pos,
                                                const PXR_NS::GfVec3d& plane_normal, const PXR_NS::GfMatrix4d& view_proj, int x, int y,
                                                PXR_NS::GfVec3d& result);
    OPENDCC_API PXR_NS::GfVec3d compute_sphere_intersection(const ViewportViewPtr& viewport_view, double screen_factor,
                                                            const PXR_NS::GfVec3d& gizmo_world_pos, int x, int y);
    OPENDCC_API PXR_NS::GfFrustum compute_view_frustum(const ViewportViewPtr& viewport_view);
    OPENDCC_API PXR_NS::GfVec2d compute_pick_point(const ViewportDimensions& viewport_dim, int x, int y);
    OPENDCC_API PXR_NS::GfRay compute_pick_ray(const PXR_NS::GfFrustum& frustum, const ViewportDimensions& viewport_dim, int x, int y);
    OPENDCC_API bool compute_screen_space_pos(const ViewportViewPtr& viewport_view, const PXR_NS::GfVec3d& gizmo_world_pos,
                                              const PXR_NS::GfVec3d& direction, const PXR_NS::GfMatrix4d& view_proj, int x, int y,
                                              PXR_NS::GfVec3d& result);
    OPENDCC_API PXR_NS::GfQuatd to_quaternion(const PXR_NS::GfVec3d& euler_angles);
    OPENDCC_API void decompose_to_common_api(const PXR_NS::UsdGeomXformable& xform, const PXR_NS::GfTransform& transform);
    OPENDCC_API void reset_pivot(const SelectionList& selection_list);
    OPENDCC_API PXR_NS::GfVec3d get_euler_angles(const PXR_NS::UsdGeomXformable& xform, PXR_NS::UsdTimeCode time);
    OPENDCC_API PXR_NS::GfVec3f decompose_to_euler(const PXR_NS::GfMatrix4d& matrix, PXR_NS::UsdGeomXformCommonAPI::RotationOrder rot_order,
                                                   const PXR_NS::GfVec3d& hint);
    OPENDCC_API void visit_all_selected_points(const SelectionData& selection_data, const PXR_NS::UsdPrim& prim,
                                               const std::function<void(int)>& visit);
    OPENDCC_API std::tuple<PXR_NS::GfVec3f, size_t> compute_centroid_data(const SelectionData& selection_data, const PXR_NS::UsdPrim& prim,
                                                                          const PXR_NS::VtArray<PXR_NS::GfVec3f>& points,
                                                                          const PXR_NS::GfMatrix4d& world_transform);

    class OPENDCC_API ViewportSelection : public commands::Edit
    {
    public:
        virtual bool operator()() override;
        virtual bool merge_with(const Edit* other) override;
        virtual size_t get_edit_type_id() const override;
    };
};
OPENDCC_NAMESPACE_CLOSE
