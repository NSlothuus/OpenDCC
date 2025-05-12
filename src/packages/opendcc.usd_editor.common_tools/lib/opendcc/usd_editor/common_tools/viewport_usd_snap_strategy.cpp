// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/common_tools/viewport_usd_snap_strategy.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"
#include <pxr/base/gf/lineSeg2d.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>
#include <pxr/base/gf/frustum.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE;

namespace
{
    HdRprimCollection create_pick_collection(const SelectionList& selection, const HdReprSelector& selector)
    {
        SdfPathVector exclude_paths;
        for (const auto& entry : selection)
            exclude_paths.push_back(entry.first);
        auto result = HdRprimCollection(HdTokens->geometry, selector, SdfPath::AbsoluteRootPath());
        result.SetExcludePaths(exclude_paths);
        return result;
    }

    GfVec2d to_screen_coord(const GfMatrix4d& view_proj, const ViewportDimensions& dim, const GfVec3f& world)
    {
        const auto projected_point = view_proj.Transform(world);
        return GfVec2d((1 + projected_point[0]) * 0.5 * dim.width, ((1 - projected_point[1]) * 0.5) * dim.height);
    }

    GfVec3i triangulate_prim(const VtIntArray& face_vertices, int face_ind, int index, int face_size, bool flip)
    {
        if (index + 2 >= face_size)
            return GfVec3i(-1);

        return flip ? GfVec3i { face_vertices[face_ind], face_vertices[face_ind + index + 2], face_vertices[face_ind + index + 1] }
                    : GfVec3i { face_vertices[face_ind], face_vertices[face_ind + index + 1], face_vertices[face_ind + index + 2] };
    }
    // point, barycentric uvw
    std::tuple<GfVec2d, double, double, double> closest_point_on_2d_triangle(const GfVec2d& p, const GfVec2d& a, const GfVec2d& b, const GfVec2d& c)
    {
        const auto ab = b - a;
        const auto ac = c - a;
        const auto ap = p - a;
        const auto d1 = GfDot(ab, ap);
        const auto d2 = GfDot(ac, ap);
        if (d1 <= 0 && d2 <= 0)
            return { a, 1, 0, 0 };

        const auto bp = p - b;
        const auto d3 = GfDot(ab, bp);
        const auto d4 = GfDot(ac, bp);
        if (d3 >= 0 && d4 <= d3)
            return { b, 0, 1, 0 };

        const auto cp = p - c;
        const auto d5 = GfDot(ab, cp);
        const auto d6 = GfDot(ac, cp);
        if (d6 >= 0 && d5 <= d6)
            return { c, 0, 0, 1 };

        const auto vc = d1 * d4 - d3 * d2;
        if (vc <= 0 && d1 >= 0 && d3 <= 0)
        {
            const auto v = d1 / (d1 - d3);
            return { a + v * ab, 1 - v, v, 0 };
        }

        const auto vb = d5 * d2 - d1 * d6;
        if (vb <= 0 && d2 >= 0 && d6 <= 0)
        {
            const auto v = d2 / (d2 - d6);
            return { a + v * ac, 1 - v, 0, v };
        }

        const auto va = d3 * d6 - d5 * d4;
        if (va <= 0 && (d4 - d3) >= 0 && (d5 - d6) >= 0)
        {
            const auto v = (d4 - d3) / ((d4 - d3) + (d5 - d6));
            return { b + v * (c - b), 0, 1 - v, v };
        }

        const auto denom = 1.f / (va + vb + vc);
        const auto v = vb * denom;
        const auto w = vc * denom;
        return { a + v * ab + w * ac, 1 - v - w, v, w };
    }
};

ViewportUsdMeshScreenSnapStrategy::ViewportUsdMeshScreenSnapStrategy(const SelectionList& selection)
    : m_selection_list(selection)
{
}

void ViewportUsdMeshScreenSnapStrategy::set_viewport_data(const ViewportViewPtr& viewport_view, const GfVec2f& screen_point, UsdTimeCode time)
{
    const auto camera = viewport_view->get_camera();
    auto frustum = camera.GetFrustum();

    const auto viewport_dim = viewport_view->get_viewport_dimensions();
    CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit,
                            viewport_dim.height != 0 ? (double)viewport_dim.width / viewport_dim.height : 1.0);

    m_view_proj = frustum.ComputeViewMatrix() * frustum.ComputeProjectionMatrix();
    m_viewport_view = viewport_view;
    m_screen_point = screen_point;
    m_time = time;
}

bool ViewportUsdMeshScreenSnapStrategy::is_valid_snap_state() const
{
    return m_viewport_view && !m_selection_list.empty();
}

GfVec3d ViewportUsdMeshScreenSnapStrategy::get_fallback_snap_value(const GfVec3d& start_pos, const GfVec3d& start_drag, const GfVec3d& cur_drag) const
{
    return start_pos + cur_drag - start_drag;
}

ViewportUsdVertexScreenSnapStrategy::ViewportUsdVertexScreenSnapStrategy(const SelectionList& selection)
    : ViewportUsdMeshScreenSnapStrategy(selection)
{
}

GfVec3d ViewportUsdVertexScreenSnapStrategy::get_snap_point(const GfVec3d& start_pos, const GfVec3d& start_drag, const GfVec3d& cur_drag)
{
    const auto stage = Application::instance().get_session()->get_current_stage();
    if (!is_valid_snap_state() || !stage)
        return get_fallback_snap_value(start_pos, start_drag, cur_drag);

    const auto pick_collection =
        create_pick_collection(m_selection_list, HdReprSelector(HdReprTokens->refined, HdReprTokens->smoothHull, HdReprTokens->points));

    const auto view_dim = m_viewport_view->get_viewport_dimensions();
    const int radius = 30;
    const GfVec2f snap_area_start = { GfMax<float>(m_screen_point[0] - radius, 0), GfMax<float>(m_screen_point[1] - radius, 0) };
    const GfVec2f snap_area_end = { GfMin<float>(m_screen_point[0] + radius, view_dim.width - 1),
                                    GfMin<float>(m_screen_point[1] + radius, view_dim.height - 1) };

    const auto pick_result = m_viewport_view->intersect(snap_area_start, snap_area_end, SelectionList::SelectionFlags::POINTS, true, &pick_collection,
                                                        { HdTokens->geometry });
    if (!pick_result.second)
        return get_fallback_snap_value(start_pos, start_drag, cur_drag);

    auto min_dist = std::numeric_limits<double>::max();
    UsdGeomXformCache cache(m_time);
    GfVec3f snap_point;
    for (const auto& hit : pick_result.first)
    {
        if (hit.pointIndex == -1 || m_selection_list.contains(hit.objectId))
            continue;

        if (auto mesh = UsdGeomPointBased(stage->GetPrimAtPath(hit.objectId)))
        {
            VtVec3fArray points;
            mesh.GetPointsAttr().Get(&points, m_time);
            const auto world_space_point = GfVec3f(cache.GetLocalToWorldTransform(mesh.GetPrim()).Transform(points[hit.pointIndex]));
            const auto screen_mesh_point = to_screen_coord(m_view_proj, view_dim, world_space_point);
            const auto screen_sq_dist_to_point = (screen_mesh_point - m_screen_point).GetLengthSq();
            if (screen_sq_dist_to_point < min_dist)
            {
                min_dist = screen_sq_dist_to_point;
                snap_point = world_space_point;
            }
        }
    }

    return min_dist == std::numeric_limits<double>::max() ? get_fallback_snap_value(start_pos, start_drag, cur_drag) : snap_point;
}

ViewportUsdEdgeScreenSnapStrategy::ViewportUsdEdgeScreenSnapStrategy(const SelectionList& selection, bool to_center)
    : ViewportUsdMeshScreenSnapStrategy(selection)
    , m_to_center(to_center)
{
}

GfVec3d ViewportUsdEdgeScreenSnapStrategy::get_snap_point(const GfVec3d& start_pos, const GfVec3d& start_drag, const GfVec3d& cur_drag)
{
    auto session = Application::instance().get_session();
    const auto stage = session->get_current_stage();
    if (!is_valid_snap_state())
        return get_fallback_snap_value(start_pos, start_drag, cur_drag);

    const auto pick_collection =
        create_pick_collection(m_selection_list, HdReprSelector(HdReprTokens->refinedWireOnSurf, HdReprTokens->refinedWireOnSurf));
    const auto view_dim = m_viewport_view->get_viewport_dimensions();
    const int radius = 30;
    const GfVec2f snap_area_start = { GfMax<float>(m_screen_point[0] - radius, 0), GfMax<float>(m_screen_point[1] - radius, 0) };
    const GfVec2f snap_area_end = { GfMin<float>(m_screen_point[0] + radius, view_dim.width - 1),
                                    GfMin<float>(m_screen_point[1] + radius, view_dim.height - 1) };
    const auto pick_result = m_viewport_view->intersect(snap_area_start, snap_area_end, SelectionList::SelectionFlags::EDGES, true, &pick_collection,
                                                        { HdTokens->geometry });
    if (!pick_result.second)
        return get_fallback_snap_value(start_pos, start_drag, cur_drag);

    auto min_dist = std::numeric_limits<double>::max();
    UsdGeomXformCache cache(m_time);
    auto topo_cache = session->get_stage_topology_cache(session->get_current_stage_id());

    GfVec3f snap_point;
    for (const auto& hit : pick_result.first)
    {
        if (hit.edgeIndex == -1 || m_selection_list.contains(hit.objectId))
            continue;

        if (auto mesh = UsdGeomMesh(stage->GetPrimAtPath(hit.objectId)))
        {
            auto topology = topo_cache.get_topology(mesh.GetPrim(), Application::instance().get_current_time());
            VtVec3fArray points;
            mesh.GetPointsAttr().Get(&points, Application::instance().get_current_time());

            auto verts = topology->edge_map.get_vertices_by_edge_id(hit.edgeIndex);
            if (!std::get<1>(verts))
                continue;
            const auto p0 = GfVec3f(cache.GetLocalToWorldTransform(mesh.GetPrim()).Transform(points[std::get<0>(verts)[0]]));
            const auto p1 = GfVec3f(cache.GetLocalToWorldTransform(mesh.GetPrim()).Transform(points[std::get<0>(verts)[1]]));

            const auto screen_p0 = to_screen_coord(m_view_proj, view_dim, p0);
            const auto screen_p1 = to_screen_coord(m_view_proj, view_dim, p1);
            const auto line_seg = GfLineSeg2d({ screen_p0[0], screen_p0[1] }, { screen_p1[0], screen_p1[1] });
            double t;
            GfVec2d closest_point;
            if (m_to_center)
            {
                closest_point = line_seg.FindClosestPoint(m_screen_point);
                t = 0.5;
            }
            else
            {
                closest_point = line_seg.FindClosestPoint(m_screen_point, &t);
            }

            const auto screen_sq_dist_to_point = (closest_point - m_screen_point).GetLengthSq();
            if (screen_sq_dist_to_point < min_dist)
            {
                min_dist = screen_sq_dist_to_point;
                snap_point = p0 + (p1 - p0) * t;
            }
        }
    }

    return min_dist == std::numeric_limits<double>::max() ? get_fallback_snap_value(start_pos, start_drag, cur_drag) : snap_point;
}

ViewportUsdFaceScreenSnapStrategy::ViewportUsdFaceScreenSnapStrategy(const SelectionList& selection, bool to_center)
    : ViewportUsdMeshScreenSnapStrategy(selection)
    , m_to_center(to_center)
{
}

GfVec3d ViewportUsdFaceScreenSnapStrategy::get_snap_point(const GfVec3d& start_pos, const GfVec3d& start_drag, const GfVec3d& cur_drag)
{
    auto session = Application::instance().get_session();
    const auto stage = session->get_current_stage();
    if (!is_valid_snap_state() || !stage)
        return get_fallback_snap_value(start_pos, start_drag, cur_drag);

    const auto pick_collection = create_pick_collection(m_selection_list, HdReprSelector(HdReprTokens->refined, HdReprTokens->hull));

    const auto view_dim = m_viewport_view->get_viewport_dimensions();
    const int radius = 30;
    const GfVec2f snap_area_start = { GfMax<float>(m_screen_point[0] - radius, 0), GfMax<float>(m_screen_point[1] - radius, 0) };
    const GfVec2f snap_area_end = { GfMin<float>(m_screen_point[0] + radius, view_dim.width - 1),
                                    GfMin<float>(m_screen_point[1] + radius, view_dim.height - 1) };
    const auto pick_result = m_viewport_view->intersect(snap_area_start, snap_area_end, SelectionList::SelectionFlags::ELEMENTS, true,
                                                        &pick_collection, { HdTokens->geometry });
    if (!pick_result.second)
        return get_fallback_snap_value(start_pos, start_drag, cur_drag);

    auto min_dist = std::numeric_limits<double>::max();
    GfVec3f snap_point;
    UsdGeomXformCache cache(m_time);
    auto topo_cache = session->get_stage_topology_cache(session->get_current_stage_id());
    for (const auto& hit : pick_result.first)
    {
        if (hit.elementIndex == -1 || m_selection_list.contains(hit.objectId))
            continue;

        auto mesh = UsdGeomMesh(stage->GetPrimAtPath(hit.objectId));
        if (!mesh)
            continue;

        auto topology = topo_cache.get_topology(mesh.GetPrim(), m_time);
        VtVec3fArray points;
        mesh.GetPointsAttr().Get(&points, m_time);

        const auto face_ind = hit.elementIndex;
        const auto& face_counts = topology->mesh_topology.GetFaceVertexCounts();
        const auto& face_indices = topology->mesh_topology.GetFaceVertexIndices();
        const auto face_start = topology->face_starts[face_ind];
        const auto point_count = face_counts[face_ind];
        // skip degenerated faces
        if (point_count < 3)
            continue;

        const auto flip = topology->mesh_topology.GetOrientation() != HdTokens->rightHanded;
        for (int j = 0; j < point_count - 2; j++)
        {
            const auto triangle = triangulate_prim(face_indices, face_start, j, point_count, flip);
            if (triangle[0] == -1)
                continue;

            const auto triangle_points = VtVec3fArray { GfVec3f(cache.GetLocalToWorldTransform(mesh.GetPrim()).Transform(points[triangle[0]])),
                                                        GfVec3f(cache.GetLocalToWorldTransform(mesh.GetPrim()).Transform(points[triangle[1]])),
                                                        GfVec3f(cache.GetLocalToWorldTransform(mesh.GetPrim()).Transform(points[triangle[2]])) };

            const auto closest_point_query_result = closest_point_on_2d_triangle(
                m_screen_point, to_screen_coord(m_view_proj, view_dim, triangle_points[0]),
                to_screen_coord(m_view_proj, view_dim, triangle_points[1]), to_screen_coord(m_view_proj, view_dim, triangle_points[2]));

            const auto& screen_point = std::get<0>(closest_point_query_result);
            if (m_to_center)
            {
                const auto screen_sq_dist_to_point = (screen_point - m_screen_point).GetLengthSq();
                if (screen_sq_dist_to_point < min_dist)
                {
                    min_dist = screen_sq_dist_to_point;
                    snap_point.Set(0, 0, 0);
                    for (int i = 0; i < point_count; i++)
                        snap_point += GfVec3f(cache.GetLocalToWorldTransform(mesh.GetPrim()).Transform(points[face_indices[face_start + i]]));
                    snap_point /= point_count;
                }
            }
            else
            {
                const auto barycentric = GfVec3d(std::get<1>(closest_point_query_result), std::get<2>(closest_point_query_result),
                                                 std::get<3>(closest_point_query_result));
                const auto screen_sq_dist_to_point = (screen_point - m_screen_point).GetLengthSq();
                if (screen_sq_dist_to_point < min_dist)
                {
                    min_dist = screen_sq_dist_to_point;
                    snap_point = triangle_points[0] * barycentric[0] + triangle_points[1] * barycentric[1] + triangle_points[2] * barycentric[2];
                }
            }

            if (GfIsClose(min_dist, 0.0, 0.000001))
                break;
        }
    }

    return min_dist == std::numeric_limits<double>::max() ? get_fallback_snap_value(start_pos, start_drag, cur_drag) : snap_point;
}

OPENDCC_NAMESPACE_CLOSE
