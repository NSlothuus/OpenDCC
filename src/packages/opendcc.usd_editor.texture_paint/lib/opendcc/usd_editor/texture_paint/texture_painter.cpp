// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/texture_paint/texture_painter.h"
#include "opendcc/app/viewport/viewport_manipulator_utils.h"
#include <igl/boundary_loop.h>
#include "tbb/task_group.h"
#include "math_utils.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/usd/usdGeom/primvarsAPI.h"

PXR_NAMESPACE_USING_DIRECTIVE
OPENDCC_NAMESPACE_OPEN
using namespace math_utils;

namespace
{
    static constexpr int UDIM_START = 1001;
    static constexpr int UDIM_END = 1100;
    uint32_t uv_to_udim_ind(const GfVec2f& uv1, const GfVec2f& uv2, const GfVec2f& uv3)
    {
        const auto uv_centroid = (uv1 + uv2 + uv3) / 3;
        return UDIM_START + std::floor(uv_centroid[0]) + (10 * std::floor(uv_centroid[1]));
    }

    void normalize_uv(GfVec2f& uv, uint32_t udim_ind)
    {
        const auto u_start = (udim_ind - UDIM_START) % 10;
        const auto v_start = (udim_ind - UDIM_START) / 10;
        uv -= GfVec2f(u_start, v_start);
    }

    static constexpr bool multithread = true;

};

void TexturePainter::outset_uv_tri(const GfVec2f& v1, const GfVec2f& v2, const GfVec2f& v3, const GfVec2f& puv1, const GfVec2f& puv2,
                                   const GfVec2f& puv3, const GfVec2i& image_size, TriangleParam& triangle_param)
{
    GfVec2f ibuf_inv(1.0f / image_size[0], 1.0f / image_size[1]);
    GfVec2f uvs[3] = { v1, v2, v3 };
    GfVec2f puvs[3] = { puv1, puv2, puv3 };
    const auto seam_bleed_px = 2.0f;
    int edge[2];
    for (edge[0] = 0; edge[0] < 3; edge[0]++)
    {
        edge[1] = (edge[0] + 1) % 3;

        if ((triangle_param.edge_flags & (1 << edge[0])) == 0)
            continue;

        auto& seam_data = triangle_param.seam_data[edge[0]];
        float ang[2];
        for (int i = 0; i < 2; i++)
        {
            GfVec2f no = seam_data.normal[i] * seam_bleed_px;
            seam_data.puv[i] = puvs[edge[i]] + no;
            seam_data.uv[i] = GfVec2f(seam_data.puv[i][0] * ibuf_inv[0], seam_data.puv[i][1] * ibuf_inv[1]);
        }

        if ((ang[0] + ang[1]) < M_PI)
        {
            if (intersect_segment_segment(uvs[edge[0]], seam_data.uv[0], uvs[edge[1]], seam_data.uv[1]))
            {
                GfVec2f isect_co;
                intersect_segment_segment(uvs[edge[0]], seam_data.uv[0], uvs[edge[1]], seam_data.uv[1], isect_co);
                seam_data.uv[0] = isect_co;
                seam_data.uv[1] = isect_co;
            }
        }
    }
}

TexturePainter::TexturePainter(const ViewportViewPtr& viewport_view, const PXR_NS::UsdGeomMesh& mesh, int mouse_x, int mouse_y,
                               const std::shared_ptr<BrushProperties>& brush_properties, const std::shared_ptr<TextureData>& texture_data,
                               bool occlude)
{
    init_mesh_data(mesh);

    const auto pick_frustum = manipulator_utils::compute_view_frustum(viewport_view);
    const auto world_transform = GfMatrix4f(mesh.ComputeLocalToWorldTransform(UsdTimeCode::Default()));
    const auto vp = GfMatrix4f(pick_frustum.ComputeViewMatrix() * pick_frustum.ComputeProjectionMatrix());

    m_brush_radius = brush_properties->get_radius();
    m_brush_color = GfVec4f(brush_properties->get_color()[0], brush_properties->get_color()[1], brush_properties->get_color()[2], 1);
    m_image_size = texture_data->get_dimensions();
    m_falloff_curve = brush_properties->get_falloff_curve();
    m_inv_view_proj = vp.GetInverse();
    m_cam_pos = GfVec3f(pick_frustum.GetPosition());
    m_occlude = occlude;
    m_texture_data = texture_data;
    init_points(viewport_view, world_transform, vp, pick_frustum.GetNearFar().GetMin(), mouse_x, mouse_y);
    init_paint_buckets(pick_frustum.GetProjectionType() == GfFrustum::ProjectionType::Perspective);
    init_vert_seam_data();
}

void TexturePainter::exec_paint()
{
    while (true)
    {
        int bucket_idx = -1;
        GfRange2f bucket_rect;
        for (int i = m_cur_bucket_id.fetch_add(1); i < m_max_bucket_id; i = m_cur_bucket_id.fetch_add(1))
        {
            const auto y = i / m_buckets_dims[0];
            const auto x = i - y * m_buckets_dims[0];
            if (m_brush_buckets_ids.GetMinX() <= x && x < m_brush_buckets_ids.GetMaxX())
            {
                bucket_rect = get_bucket_rect(m_mesh_bbox_ss, m_buckets_dims, GfVec2i(x, y));
                if (intersect_rect_circle(bucket_rect, m_brush_center_ss, 2 * m_brush_radius))
                {
                    bucket_idx = i;
                    break;
                }
            }
        }
        if (bucket_idx == -1)
            break;

        auto& cur_bucket = m_paint_buckets[bucket_idx];
        if (!cur_bucket.pixels_initialized)
            init_bucket_pixels(bucket_rect, cur_bucket);

        draw_pixels(cur_bucket);
    };
}

void TexturePainter::init_bucket_pixels(const PXR_NS::GfRange2f& bucket_rect, PaintBucket& bucket)
{
    if (m_texture_data->is_udim())
    {
        uint32_t cur_udim_tile = 0;
        auto img_iter = m_texture_data->get_image_data().end();
        for (auto tri_id : bucket.tri_ids)
        {
            const auto& v1_uv = m_uvs[m_st_tri_indices[tri_id][0]];
            const auto& v2_uv = m_uvs[m_st_tri_indices[tri_id][1]];
            const auto& v3_uv = m_uvs[m_st_tri_indices[tri_id][2]];
            const auto udim_tile = uv_to_udim_ind(v1_uv, v2_uv, v3_uv);

            if (udim_tile != cur_udim_tile)
            {
                img_iter = m_texture_data->get_image_data().find(udim_tile);
                cur_udim_tile = udim_tile;
            }
            if (img_iter != m_texture_data->get_image_data().end())
                fill_bucket_pixels(bucket_rect, tri_id, bucket, img_iter->second.get());
        }
    }
    else
    {
        const auto img_data = m_texture_data->get_image_data().at(UDIM_START).get();
        for (auto tri_id : bucket.tri_ids)
        {
            const auto& v1_uv = m_uvs[m_st_tri_indices[tri_id][0]];
            const auto& v2_uv = m_uvs[m_st_tri_indices[tri_id][1]];
            const auto& v3_uv = m_uvs[m_st_tri_indices[tri_id][2]];
            const auto udim_tile = uv_to_udim_ind(v1_uv, v2_uv, v3_uv);
            if (udim_tile == UDIM_START)
                fill_bucket_pixels(bucket_rect, tri_id, bucket, img_data);
        }
    }
    bucket.pixels_initialized = true;
}

bool TexturePainter::occlusion_test(const PXR_NS::GfVec3f& point_ss, const PaintBucket& bucket, int point_tri_id) const
{
    for (int i = 0; i < bucket.tri_ids.size(); i++)
    {
        const auto tri_id = bucket.tri_ids[i];
        if (tri_id == point_tri_id)
            continue;

        const auto& triangle_vert_ids = m_triangulated_vertex_indices[tri_id];
        const auto& v1_ss_vec4 = m_points_ss[triangle_vert_ids[0]];
        const auto& v2_ss_vec4 = m_points_ss[triangle_vert_ids[1]];
        const auto& v3_ss_vec4 = m_points_ss[triangle_vert_ids[2]];
        if (is_occluded(v1_ss_vec4, v2_ss_vec4, v3_ss_vec4, point_ss))
            return false;
    }
    return true;
}

void TexturePainter::init_vert_seam_data()
{
    Eigen::MatrixX3i igl_face_inds;
    igl_face_inds.resize(m_st_tri_indices.size(), 3);
    for (int i = 0; i < m_st_tri_indices.size(); i++)
    {
        igl_face_inds(i, 0) = m_st_tri_indices[i][0];
        igl_face_inds(i, 1) = m_st_tri_indices[i][1];
        igl_face_inds(i, 2) = m_st_tri_indices[i][2];
    };

    std::vector<std::vector<Eigen::Index>> uv_boundary_loops;
    igl::boundary_loop(igl_face_inds, uv_boundary_loops);

    m_vert_seam_data.clear();
    for (const auto& loop : uv_boundary_loops)
    {
        for (int i = 0; i < loop.size(); i++)
        {
            auto v0 = loop[(i - 1 + loop.size()) % loop.size()];
            auto v1 = loop[i];
            auto v2 = loop[(i + 1) % loop.size()];

            const auto prev_uv = m_uvs[v0];
            const auto cur_uv = m_uvs[v1];
            const auto next_uv = m_uvs[v2];
            const auto a = (prev_uv - cur_uv).GetNormalized();
            const auto b = (next_uv - cur_uv).GetNormalized();
            VertSeamData data;
            data.normal = (a + b).GetNormalized();
            data.angle = GfDot(a, data.normal);
            m_vert_seam_data[v1] = data;
        }
    }
}

void TexturePainter::init_paint_buckets(bool is_persp)
{
    const auto diameter = m_brush_radius * 2;
    // TODO: strong performance loss on small objects that wrap large texture area
    const auto buckets_per_brush = 4;
    m_buckets_dims[0] = m_mesh_bbox_ss.GetSize()[0] / ((float)diameter / buckets_per_brush);
    m_buckets_dims[1] = m_mesh_bbox_ss.GetSize()[1] / ((float)diameter / buckets_per_brush);

    m_buckets_dims[0] = GfMin(GfMax(m_buckets_dims[0], 4), 256);
    m_buckets_dims[1] = GfMin(GfMax(m_buckets_dims[1], 4), 256);
    m_paint_buckets.resize(m_buckets_dims[0] * m_buckets_dims[1]);
    for (int i = 0; i < m_triangulated_vertex_indices.size(); i++)
    {
        const auto& v1_ss = m_points_ss[m_triangulated_vertex_indices[i][0]];
        const auto& v2_ss = m_points_ss[m_triangulated_vertex_indices[i][1]];
        const auto& v3_ss = m_points_ss[m_triangulated_vertex_indices[i][2]];

        // Try to clip vertices that a less than clip_start and use this func
        // do clipping in world space so far.
        if (should_cull(v1_ss, v2_ss, v3_ss, m_mesh_bbox_ss, is_persp))
            continue;

        GfRange2f face_bbox_ss;
        face_bbox_ss.ExtendBy(GfVec2f(v1_ss[0], v1_ss[1]));
        face_bbox_ss.ExtendBy(GfVec2f(v2_ss[0], v2_ss[1]));
        face_bbox_ss.ExtendBy(GfVec2f(v3_ss[0], v3_ss[1]));

        const auto buckets_rect_ids = get_bucket_min_max_ids(m_mesh_bbox_ss, m_buckets_dims, face_bbox_ss);
        bool hit_smth = false;
        for (int y = buckets_rect_ids.GetMinY(); y < buckets_rect_ids.GetMaxY(); ++y)
        {
            bool hit_row = false;
            for (int x = buckets_rect_ids.GetMinX(); x < buckets_rect_ids.GetMaxX(); ++x)
            {
                const auto bucket_rect = get_bucket_rect(m_mesh_bbox_ss, m_buckets_dims, GfVec2i(x, y));
                if (intersect_triangle_rect(v1_ss, v2_ss, v3_ss, bucket_rect))
                {
                    const auto bucket_id = y * m_buckets_dims[0] + x;
                    m_paint_buckets[bucket_id].tri_ids.push_back(i);
                    hit_smth = hit_row = true;
                }
                else if (hit_row) // no way we can hit another pixel in this row
                {
                    break;
                }
            }
            if (!hit_row && hit_smth)
                break;
        }
    }
    GfRange2f draw_rect(GfVec2f(m_brush_center_ss[0] - m_brush_radius, m_brush_center_ss[1] - m_brush_radius),
                        GfVec2f(m_brush_center_ss[0] + m_brush_radius, m_brush_center_ss[1] + m_brush_radius));

    m_brush_buckets_ids = get_bucket_min_max_ids(m_mesh_bbox_ss, m_buckets_dims, draw_rect);
}

void TexturePainter::init_points(const ViewportViewPtr& viewport_view, const PXR_NS::GfMatrix4f& world_transform,
                                 const PXR_NS::GfMatrix4f& view_projection, float near_clip, int mouse_x, int mouse_y)
{
    const auto mvp = world_transform * view_projection;
    const auto view_dim = viewport_view->get_viewport_dimensions();
    const auto to_ss = [&mvp, &view_dim, near_clip](const GfVec3f& point) {
        auto screen_coords = GfVec4f(point[0], point[1], point[2], 1) * mvp;
        if (screen_coords[3] < near_clip)
        {
            screen_coords[0] = std::numeric_limits<float>::quiet_NaN();
            return screen_coords;
        }
        screen_coords[0] = (view_dim.width * 0.5) + (view_dim.width * 0.5) * screen_coords[0] / screen_coords[3];
        screen_coords[1] = (view_dim.height * 0.5) + (view_dim.height * 0.5) * screen_coords[1] / screen_coords[3];
        screen_coords[2] = screen_coords[2] / screen_coords[3];
        return screen_coords;
    };

    m_mesh_bbox_ss.SetEmpty();
    m_points_ss.resize(points_world.size());
    for (int i = 0; i < points_world.size(); i++)
    {
        m_points_ss[i] = to_ss(points_world[i]);
        points_world[i] = world_transform.Transform(points_world[i]);
        m_mesh_bbox_ss.ExtendBy(GfVec2f(m_points_ss[i][0], m_points_ss[i][1]));
    }
    // Offset to avoid artifacts when mesh face is parallel to bbox face
    const auto mesh_bbox_ss_offset = (m_mesh_bbox_ss.GetMax() - m_mesh_bbox_ss.GetMin()) * 0.00001f;
    m_mesh_bbox_ss.SetMin(m_mesh_bbox_ss.GetMin() - mesh_bbox_ss_offset);
    m_mesh_bbox_ss.SetMax(m_mesh_bbox_ss.GetMax() + mesh_bbox_ss_offset);

    const auto extended_screen_range =
        GfRange2f(GfVec2f(-m_brush_radius, -m_brush_radius), GfVec2f(view_dim.width + m_brush_radius, view_dim.height + m_brush_radius));
    m_mesh_bbox_ss.IntersectWith(extended_screen_range);
    GfVec2f center_ss(mouse_x, view_dim.height - mouse_y - 1);
    m_viewport_dims = GfVec2i(view_dim.width, view_dim.height);
    m_brush_center_ss = center_ss;
}

void TexturePainter::init_mesh_data(const PXR_NS::UsdGeomMesh& mesh)
{
    TfToken subdiv_scheme;
    TfToken orientation;
    VtIntArray face_vertex_indices;
    VtIntArray face_vertex_counts;
    mesh.GetSubdivisionSchemeAttr().Get(&subdiv_scheme);
    mesh.GetOrientationAttr().Get(&orientation);
    mesh.GetFaceVertexCountsAttr().Get(&face_vertex_counts);
    mesh.GetFaceVertexIndicesAttr().Get(&face_vertex_indices);
    HdMeshTopology topo(subdiv_scheme, orientation, face_vertex_counts, face_vertex_indices);

    const auto mesh_util = HdMeshUtil(&topo, mesh.GetPath());
    VtIntArray primitive_params;
    mesh_util.ComputeTriangleIndices(&m_triangulated_vertex_indices, &primitive_params);
    mesh.GetPointsAttr().Get(&points_world);

    // Assume FaceVarying interpolation
    // TODO: select UV primvar, for now, hard coded st
    VtIntArray indices;
    UsdGeomPrimvarsAPI primvars_api(mesh.GetPrim());
    auto st_primvar = primvars_api.GetPrimvar(TfToken("st"));
    st_primvar.Get(&m_uvs);
    st_primvar.GetIndices(&indices);
    HdMeshTopology st_topo(subdiv_scheme, orientation, face_vertex_counts, indices);
    const auto st_utils = HdMeshUtil(&st_topo, st_primvar.GetAttr().GetPath());
    st_utils.ComputeTriangleIndices(&m_st_tri_indices, &primitive_params);
}

void TexturePainter::paint_stroke(const PXR_NS::GfVec2f& center_ss)
{
    // Find buckets in brush radius:
    GfRange2f draw_rect(GfVec2f(center_ss[0] - m_brush_radius, center_ss[1] - m_brush_radius),
                        GfVec2f(center_ss[0] + m_brush_radius, center_ss[1] + m_brush_radius));
    m_brush_buckets_ids = get_bucket_min_max_ids(m_mesh_bbox_ss, m_buckets_dims, draw_rect);
    // mouse outside the mesh area
    if (m_brush_buckets_ids.GetMinX() == m_brush_buckets_ids.GetMaxX() || m_brush_buckets_ids.GetMinY() == m_brush_buckets_ids.GetMaxY())
        return;

    m_brush_center_ss = center_ss;
    std::atomic_int cur_bucket_id { m_brush_buckets_ids.GetMinY() * m_buckets_dims[0] + m_brush_buckets_ids.GetMinX() };
    m_max_bucket_id = (m_brush_buckets_ids.GetMaxY() - 1) * m_buckets_dims[0] + m_brush_buckets_ids.GetMaxX();
    m_cur_bucket_id.store(cur_bucket_id);

    if (multithread)
    {
        const auto thread_count = std::thread::hardware_concurrency();
        tbb::task_group task_group;
        for (int i = 0; i < thread_count; i++)
        {
            task_group.run([this]() { exec_paint(); });
        }
        task_group.wait();
    }
    else
    {
        exec_paint();
    }
}

void TexturePainter::paint_stroke_to(const PXR_NS::GfVec2i& to_ss)
{
    auto stroke_dir = GfVec2f(to_ss) - GfVec2f(m_brush_center_ss[0], m_brush_center_ss[1]);
    float length = stroke_dir.Normalize();

    while (length > 0)
    {
        const float spacing = m_brush_radius * 0.1;
        if (length < spacing)
            break;

        const GfVec2f center_ss = m_brush_center_ss + stroke_dir * spacing;
        paint_stroke(center_ss);
        length -= spacing;
    };
}

bool TexturePainter::is_valid() const
{
    return m_brush_buckets_ids.GetMinX() == m_brush_buckets_ids.GetMaxX() || m_brush_buckets_ids.GetMinY() == m_brush_buckets_ids.GetMaxY();
}

void TexturePainter::fill_bucket_pixels(const GfRange2f& bucket_rect, int tri_id, PaintBucket& cur_bucket, ImageData* img_data)
{
    std::vector<GfVec2f> clip_polyline_uv;
    clip_polyline_uv.reserve(8); // should be enough even in the worst case

    const auto& triangle_vert_ids = m_triangulated_vertex_indices[tri_id];
    const auto& v1_ss_vec4 = m_points_ss[triangle_vert_ids[0]];
    const auto& v2_ss_vec4 = m_points_ss[triangle_vert_ids[1]];
    const auto& v3_ss_vec4 = m_points_ss[triangle_vert_ids[2]];
    const auto v1_ss = GfVec2f(v1_ss_vec4[0], v1_ss_vec4[1]);
    const auto v2_ss = GfVec2f(v2_ss_vec4[0], v2_ss_vec4[1]);
    const auto v3_ss = GfVec2f(v3_ss_vec4[0], v3_ss_vec4[1]);
    const auto persp_weights = GfVec3f(v1_ss_vec4[3], v2_ss_vec4[3], v3_ss_vec4[3]);

    auto v1_uv = m_uvs[m_st_tri_indices[tri_id][0]];
    auto v2_uv = m_uvs[m_st_tri_indices[tri_id][1]];
    auto v3_uv = m_uvs[m_st_tri_indices[tri_id][2]];

    // normalize udim UVs to [0;1] range
    normalize_uv(v1_uv, img_data->udim_index);
    normalize_uv(v2_uv, img_data->udim_index);
    normalize_uv(v3_uv, img_data->udim_index);

    const auto half_px = GfVec2f((0.5f + 0.01 * (1.0 / 3)) / m_image_size[0], (0.5f + 0.01 * (1.0 / 4)) / m_image_size[1]);
    // some magic with UV offsets
    v1_uv -= half_px;
    v2_uv -= half_px;
    v3_uv -= half_px;

    GfRange2f extended_bucket_rect = bucket_rect;
    extended_bucket_rect.SetMin(bucket_rect.GetMin() - GfVec2f(0.01f));
    extended_bucket_rect.SetMax(bucket_rect.GetMax() + GfVec2f(0.01f));

    init_clipping_polyline(v1_ss, v2_ss, v3_ss, GfVec3f(v1_ss_vec4[3], v2_ss_vec4[3], v3_ss_vec4[3]), persp_weights, v1_uv, v2_uv, v3_uv,
                           extended_bucket_rect, bucket_rect, true, false, clip_polyline_uv);

    if (clip_polyline_uv.empty())
        return;

    // get image pixels from UV polyline
    GfRange2f polyline_bbox;
    for (auto& v : clip_polyline_uv)
        polyline_bbox.ExtendBy(v);

    const GfRect2i image_bounds(GfVec2i(m_image_size[0] * polyline_bbox.GetMin()[0], m_image_size[1] * polyline_bbox.GetMin()[1]),
                                GfVec2i(m_image_size[0] * polyline_bbox.GetMax()[0] + 1, m_image_size[1] * polyline_bbox.GetMax()[1] + 1));
    if (image_bounds.GetMinX() == image_bounds.GetMaxX() || image_bounds.GetMinY() == image_bounds.GetMaxY())
        return;

    const auto with_backface_culling = false;
    add_pixels_from_poly_bounds(image_bounds, tri_id, img_data, v1_uv, v2_uv, v3_uv, clip_polyline_uv, persp_weights,
                                GfVec3f(v1_ss[0], v1_ss[1], v1_ss_vec4[2]), GfVec3f(v2_ss[0], v2_ss[1], v2_ss_vec4[2]),
                                GfVec3f(v3_ss[0], v3_ss[1], v3_ss_vec4[2]), with_backface_culling, cur_bucket);

    // Handle seams
    add_pixels_from_seams(bucket_rect, extended_bucket_rect, tri_id, img_data, v1_uv, v2_uv, v3_uv, persp_weights,
                          GfVec3f(v1_ss[0], v1_ss[1], v1_ss_vec4[2]), GfVec3f(v2_ss[0], v2_ss[1], v2_ss_vec4[2]),
                          GfVec3f(v3_ss[0], v3_ss[1], v3_ss_vec4[2]), with_backface_culling, cur_bucket);
}

void TexturePainter::add_pixels_from_poly_bounds(const GfRect2i& bounds, int tri_id, ImageData* img_data, const GfVec2f& uv1, const GfVec2f& uv2,
                                                 const GfVec2f& uv3, const std::vector<GfVec2f>& clip_polyline_uv, const GfVec3f& persp_weights,
                                                 const GfVec3f& v1_ss, const GfVec3f& v2_ss, const GfVec3f& v3_ss, bool with_backface_culling,
                                                 PaintBucket& paint_bucket)
{
    for (int y = bounds.GetMinY(); y < bounds.GetMaxY(); y++)
    {
        GfVec2f uv;
        uv[1] = (float)y / m_image_size[1];
        bool hit_row = false;
        for (int x = bounds.GetMinX(); x < bounds.GetMaxX(); x++)
        {
            uv[0] = (float)x / m_image_size[0];
            if ((with_backface_culling && is_inside_polyline(clip_polyline_uv, uv)) ||
                (!with_backface_culling && is_inside_polyline_twoside(clip_polyline_uv, uv)))
            {
                hit_row = true;
                // calc SS pos with persp correction
                const auto bary_weights = to_bary_2d(uv1, uv2, uv3, uv);
                auto cor_weights =
                    GfVec3f(bary_weights[0] * persp_weights[0], bary_weights[1] * persp_weights[1], bary_weights[2] * persp_weights[2]);
                const float sum_weights = cor_weights[0] + cor_weights[1] + cor_weights[2];
                if (sum_weights > 0)
                    cor_weights /= sum_weights;
                else
                    cor_weights = GfVec3f(1.0 / 3);

                const auto screen_coord = bary_interp(GfVec3f(v1_ss[0], v1_ss[1], v1_ss[2]), GfVec3f(v2_ss[0], v2_ss[1], v2_ss[2]),
                                                      GfVec3f(v3_ss[0], v3_ss[1], v3_ss[2]), cor_weights);

                if (m_occlude && !occlusion_test(screen_coord, paint_bucket, tri_id))
                    continue;

                const auto wrapped_x = GfClamp(x, 0.0, m_image_size[0] - 1);
                const auto wrapped_y = GfClamp(y, 0.0, m_image_size[1] - 1);

                const auto px_ind = size_t(wrapped_y) * m_image_size[0] + wrapped_x;

                PixelInfo pix_info;
                pix_info.pixel_data = &img_data->shared_px_data[px_ind];
                pix_info.ss = GfVec2f(screen_coord[0], screen_coord[1]);
                pix_info.tri_id = tri_id;
                paint_bucket.pixels.push_back(std::move(pix_info));
            }
            else if (hit_row)
            {
                break;
            }
        }
    }
}

void TexturePainter::add_pixels_from_seam_bounds(const GfRect2i& bounds, int tri_id, ImageData* img_data, const TriangleParam::SeamData& seam_data,
                                                 const GfVec2f seam_subsection[4], const GfVec2f& uv1, const GfVec2f& uv2, const GfVec2f& uv3,
                                                 const GfVec3f& persp_weights, const GfVec3f& v1_ss, const GfVec3f& v2_ss, const GfVec3f& v3_ss,
                                                 bool with_backface_culling, PaintBucket& paint_bucket)
{
    for (int y = bounds.GetMinY(); y < bounds.GetMaxY(); y++)
    {
        GfVec2f uv;
        uv[1] = (float)y / m_image_size[1];
        bool hit_row = false;
        for (int x = bounds.GetMinX(); x < bounds.GetMaxX(); x++)
        {
            GfVec2f puv { (float)x, (float)y };
            uv[0] = (float)x / m_image_size[0];

            bool in_bounds = false;
            if (seam_data.uv[0] == seam_data.uv[1])
                in_bounds = intersect_point_triangle(uv, seam_subsection[0], seam_subsection[1], seam_subsection[2]);
            else
                in_bounds = intersect_point_quad(uv, seam_subsection[0], seam_subsection[1], seam_subsection[2], seam_subsection[3]);

            if (in_bounds)
            {
                const auto bary_weights = to_bary_2d(uv1, uv2, uv3, uv);
                auto cor_weights =
                    GfVec3f(bary_weights[0] * persp_weights[0], bary_weights[1] * persp_weights[1], bary_weights[2] * persp_weights[2]);
                const float sum_weights = cor_weights[0] + cor_weights[1] + cor_weights[2];
                if (sum_weights > 0)
                    cor_weights /= sum_weights;
                else
                    cor_weights = GfVec3f(1.0 / 3);

                const auto screen_coord = bary_interp(v1_ss, v2_ss, v3_ss, cor_weights);

                if (m_occlude && !occlusion_test(screen_coord, paint_bucket, tri_id))
                    continue;

                const auto wrapped_x = GfClamp(x, 0.0, m_image_size[0] - 1);
                const auto wrapped_y = GfClamp(y, 0.0, m_image_size[1] - 1);

                const auto px_ind = size_t(wrapped_y) * m_image_size[0] + wrapped_x;

                PixelInfo pix_info;
                pix_info.pixel_data = &img_data->shared_px_data[px_ind]; // shared_px_data[size_t(wrapped_y) * image_size[0] + wrapped_x];
                pix_info.ss = GfVec2f(screen_coord[0], screen_coord[1]);
                pix_info.tri_id = tri_id;
                paint_bucket.pixels.push_back(std::move(pix_info));
            }
        }
    }
}

void TexturePainter::add_pixels_from_seams(const GfRange2f& bucket_rect, const GfRange2f& extended_bucket_rect, int tri_id, ImageData* img_data,
                                           const GfVec2f& uv1, const GfVec2f& uv2, const GfVec2f& uv3, const GfVec3f& persp_weights,
                                           const GfVec3f& v1_ss, const GfVec3f& v2_ss, const GfVec3f& v3_ss, bool with_backface_culling,
                                           PaintBucket& paint_bucket)
{
    const auto uv1_ind = m_st_tri_indices[tri_id][0];
    const auto uv2_ind = m_st_tri_indices[tri_id][1];
    const auto uv3_ind = m_st_tri_indices[tri_id][2];
    TriangleParam tri_param;
    decltype(m_vert_seam_data.begin()) vert_it[3];
    vert_it[0] = m_vert_seam_data.find(uv1_ind);
    vert_it[1] = m_vert_seam_data.find(uv2_ind);
    // no seams
    if (vert_it[0] == m_vert_seam_data.end() && vert_it[1] == m_vert_seam_data.end())
        return;
    vert_it[2] = m_vert_seam_data.find(uv3_ind);

    const auto uv_tri_mid_point = (m_uvs[uv1_ind] + m_uvs[uv2_ind] + m_uvs[uv3_ind]) / 3;
    for (int e_i = 0; e_i < 3; ++e_i)
    {
        int edge_idx[2] = { e_i, (e_i + 1) % 3 };
        GfVec2i e;
        e[0] = m_st_tri_indices[tri_id][edge_idx[0]];
        e[1] = m_st_tri_indices[tri_id][edge_idx[1]];

        if (vert_it[edge_idx[0]] == m_vert_seam_data.end() || vert_it[edge_idx[1]] == m_vert_seam_data.end())
            continue;

        tri_param.edge_flags |= (TriangleParam::EdgeFlags::Seam1 << e_i);
        auto& seam_data = tri_param.seam_data[e_i];

        const auto swap_coef = left_of_line(uv_tri_mid_point, m_uvs[e[0]], m_uvs[e[1]]) ? -1 : 1;

        for (int j = 0; j < 2; j++)
        {
            const auto& vert_seam = vert_it[edge_idx[j]]->second;
            if (left_of_line(vert_seam.normal + m_uvs[e[j]], m_uvs[e[0]], m_uvs[e[1]]))
                seam_data.normal[j] = swap_coef * vert_seam.normal;
            else
                seam_data.normal[j] = -1 * swap_coef * vert_seam.normal;
        }
    }
    if (tri_param.edge_flags == TriangleParam::Invalid)
        return;

    // outset triangle uv
    GfVec2f uvs[3] = { uv1, uv2, uv3 };
    GfVec2f puvs[3];
    for (int i = 0; i < 3; i++)
        puvs[i] = GfVec2f(uvs[i][0] * m_image_size[0], uvs[i][1] * m_image_size[1]);
    outset_uv_tri(uv1, uv2, uv3, puvs[0], puvs[1], puvs[2], m_image_size, tri_param);
    GfVec2f seam_uvs[4];

    // for now only if persp!
    // TODO: for ortho use screen space
    const auto& triangle_vert_ids = m_triangulated_vertex_indices[tri_id];
    const auto& v1_wpos = points_world[triangle_vert_ids[0]];
    const auto& v2_wpos = points_world[triangle_vert_ids[1]];
    const auto& v3_wpos = points_world[triangle_vert_ids[2]];

    GfVec3f wpos[3] = { v1_wpos, v2_wpos, v3_wpos };

    GfVec2f bucket_clip_edge[2];
    GfVec3f edge_verts_inset_clip[2];
    std::array<GfVec2f, 3> ss_points = { GfVec2f(v1_ss[0], v1_ss[1]), GfVec2f(v2_ss[0], v2_ss[1]), GfVec2f(v3_ss[0], v3_ss[1]) };
    for (int e1 = 0; e1 < 3; e1++)
    {
        int e2 = (e1 + 1) % 3;
        if ((tri_param.edge_flags & (1 << e1)) && // if cur edge is a seam
            clip_line(extended_bucket_rect, bucket_rect, ss_points[e1], ss_points[e2], bucket_clip_edge[0], bucket_clip_edge[1]))
        {
            if ((ss_points[e1] - ss_points[e2]).GetLengthSq() > FLT_EPSILON)
            {
                const auto& seam_data = tri_param.seam_data[e1];
                const auto fac1 = get_uv_point_on_line(m_viewport_dims, m_inv_view_proj, m_cam_pos, bucket_clip_edge[0], wpos[e1], wpos[e2]);
                const auto fac2 = get_uv_point_on_line(m_viewport_dims, m_inv_view_proj, m_cam_pos, bucket_clip_edge[1], wpos[e1], wpos[e2]);

                GfVec2f seam_subsection[4];
                seam_subsection[0] = GfLerp(fac1, uvs[e1], uvs[e2]);
                seam_subsection[1] = GfLerp(fac2, uvs[e1], uvs[e2]);
                seam_subsection[2] = GfLerp(fac2, seam_data.uv[0], seam_data.uv[1]);
                seam_subsection[3] = GfLerp(fac1, seam_data.uv[0], seam_data.uv[1]);

                GfRange2f seam_bbox;
                for (int i = 0; i < 4; i++)
                    seam_bbox.ExtendBy(seam_subsection[i]);

                GfRect2i image_bounds(GfVec2i(m_image_size[0] * seam_bbox.GetMin()[0], m_image_size[1] * seam_bbox.GetMin()[1]),
                                      GfVec2i(m_image_size[0] * seam_bbox.GetMax()[0] + 1, m_image_size[1] * seam_bbox.GetMax()[1] + 1));
                if (image_bounds.GetMinX() == image_bounds.GetMaxX() || image_bounds.GetMinY() == image_bounds.GetMaxY())
                    return;

                add_pixels_from_seam_bounds(image_bounds, tri_id, img_data, seam_data, seam_subsection, uv1, uv2, uv3, persp_weights, v1_ss, v2_ss,
                                            v3_ss, with_backface_culling, paint_bucket);
            }
        }
    }
}

void TexturePainter::draw_pixels(PaintBucket& bucket)
{
    const auto radius_sq = m_brush_radius * m_brush_radius;

    for (auto& pi : bucket.pixels)
    {
        const auto length_sq = (pi.ss - m_brush_center_ss).GetLengthSq();
        if (length_sq > radius_sq)
            continue;

        const auto px_data = pi.pixel_data;
        // TODO brush strength, opacity

        const auto falloff = m_falloff_curve->value_at(sqrt(length_sq) / m_brush_radius);
        const float opacity = 1.0f;
        const float max_influence = opacity * falloff;
        const float influence = GfMin(px_data->influence + (max_influence - px_data->influence * falloff), 1.0f);
        if (influence == 0)
            continue;
        px_data->influence = influence;

        // TODO: other blend modes
        float pixel_color[4];
        pixel_color[0] = GfLerp(influence, px_data->orig_color[0], m_brush_color[0]);
        pixel_color[1] = GfLerp(influence, px_data->orig_color[1], m_brush_color[1]);
        pixel_color[2] = GfLerp(influence, px_data->orig_color[2], m_brush_color[2]);
        pixel_color[3] = GfLerp(influence, px_data->orig_color[3], m_brush_color[3]);

        px_data->img_data->texture_buffer->setpixel(px_data->x, px_data->y, pixel_color);
        px_data->touched = true;
        px_data->img_data->dirty = true;
    }
}

OPENDCC_NAMESPACE_CLOSE
