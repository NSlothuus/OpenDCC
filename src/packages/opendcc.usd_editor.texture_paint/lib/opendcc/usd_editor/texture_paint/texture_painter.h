/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/viewport/viewport_view.h"
#include "opendcc/usd_editor/texture_paint/brush_properties.h"
#include <pxr/usd/usdGeom/mesh.h>
#include "opendcc/usd_editor/texture_paint/texture_data.h"
#include "opendcc/ui/common_widgets/ramp.h"
#include <pxr/base/gf/matrix4f.h>
#include <atomic>

OPENDCC_NAMESPACE_OPEN

class TexturePainter
{
public:
    TexturePainter(const ViewportViewPtr& viewport_view, const PXR_NS::UsdGeomMesh& mesh, int mouse_x, int mouse_y,
                   const std::shared_ptr<BrushProperties>& brush_properties, const std::shared_ptr<TextureData>& texture_data, bool occlude);

    void paint_stroke(const PXR_NS::GfVec2f& center_ss);
    void paint_stroke_to(const PXR_NS::GfVec2i& to_ss);
    bool is_valid() const;

private:
    friend class TexturePaintStrokeCommand;
    struct PixelInfo
    {
        SharedPixelData* pixel_data = nullptr;
        PXR_NS::GfVec2f ss;
        int tri_id = -1;
    };
    struct PaintBucket
    {
        std::vector<int> tri_ids;
        std::vector<PixelInfo> pixels;
        bool pixels_initialized = false;
    };
    struct TriangleParam
    {
        enum EdgeFlags
        {
            Invalid = 0,
            Seam1 = 1,
            Seam2 = 2,
            Seam3 = 4
        };
        struct SeamData
        {
            PXR_NS::GfVec2f uv[2];
            PXR_NS::GfVec2f puv[2];
            PXR_NS::GfVec2f normal[2];
        };
        SeamData seam_data[3];
        uint8_t edge_flags = 0;
    };
    struct VertSeamData
    {
        float angle;
        PXR_NS::GfVec2f normal;
    };

    // initialization
    void init_mesh_data(const PXR_NS::UsdGeomMesh& mesh);
    void init_points(const ViewportViewPtr& viewport_view, const PXR_NS::GfMatrix4f& world_transform, const PXR_NS::GfMatrix4f& view_projection,
                     float near_clip, int mouse_x, int mouse_y);
    void init_paint_buckets(bool is_persp);
    void init_vert_seam_data();

    // painting
    void exec_paint();
    void init_bucket_pixels(const PXR_NS::GfRange2f& bucket_rect, PaintBucket& bucket);
    void fill_bucket_pixels(const PXR_NS::GfRange2f& bucket_rect, int tri_id, PaintBucket& cur_bucket, ImageData* img_data);
    void add_pixels_from_poly_bounds(const PXR_NS::GfRect2i& bounds, int tri_id, ImageData* img_data, const PXR_NS::GfVec2f& uv1,
                                     const PXR_NS::GfVec2f& uv2, const PXR_NS::GfVec2f& uv3, const std::vector<PXR_NS::GfVec2f>& clip_polyline_uv,
                                     const PXR_NS::GfVec3f& persp_weights, const PXR_NS::GfVec3f& v1_ss, const PXR_NS::GfVec3f& v2_ss,
                                     const PXR_NS::GfVec3f& v3_ss, bool with_backface_culling, PaintBucket& paint_bucket);
    void add_pixels_from_seam_bounds(const PXR_NS::GfRect2i& bounds, int tri_id, ImageData* img_data, const TriangleParam::SeamData& seam_data,
                                     const PXR_NS::GfVec2f seam_subsection[4], const PXR_NS::GfVec2f& uv1, const PXR_NS::GfVec2f& uv2,
                                     const PXR_NS::GfVec2f& uv3, const PXR_NS::GfVec3f& persp_weights, const PXR_NS::GfVec3f& v1_ss,
                                     const PXR_NS::GfVec3f& v2_ss, const PXR_NS::GfVec3f& v3_ss, bool with_backface_culling,
                                     PaintBucket& paint_bucket);
    void add_pixels_from_seams(const PXR_NS::GfRange2f& bucket_rect, const PXR_NS::GfRange2f& extended_bucket_rect, int tri_id, ImageData* img_data,
                               const PXR_NS::GfVec2f& uv1, const PXR_NS::GfVec2f& uv2, const PXR_NS::GfVec2f& uv3,
                               const PXR_NS::GfVec3f& persp_weights, const PXR_NS::GfVec3f& v1_ss, const PXR_NS::GfVec3f& v2_ss,
                               const PXR_NS::GfVec3f& v3_ss, bool with_backface_culling, PaintBucket& paint_bucket);
    void draw_pixels(PaintBucket& bucket);
    void outset_uv_tri(const PXR_NS::GfVec2f& v1, const PXR_NS::GfVec2f& v2, const PXR_NS::GfVec2f& v3, const PXR_NS::GfVec2f& puv1,
                       const PXR_NS::GfVec2f& puv2, const PXR_NS::GfVec2f& puv3, const PXR_NS::GfVec2i& image_size, TriangleParam& triangle_param);
    bool occlusion_test(const PXR_NS::GfVec3f& point_ss, const PaintBucket& bucket, int point_tri_id) const;

    std::atomic_int m_cur_bucket_id { -1 };
    int m_max_bucket_id = -1;
    PXR_NS::GfVec2i m_buckets_dims = PXR_NS::GfVec2i(0);
    PXR_NS::GfRect2i m_brush_buckets_ids = PXR_NS::GfRect2i(PXR_NS::GfVec2i(0), 0, 0);
    PXR_NS::GfRange2f m_mesh_bbox_ss = PXR_NS::GfRange2f(PXR_NS::GfVec2f(0), PXR_NS::GfVec2f(0));
    PXR_NS::GfVec3f m_cam_pos;
    PXR_NS::GfMatrix4f m_inv_view_proj;
    PXR_NS::GfVec2i m_viewport_dims;

    PXR_NS::GfVec2f m_brush_center_ss = PXR_NS::GfVec2f(0);
    PXR_NS::GfVec4f m_brush_color;
    float m_brush_radius = 0;

    std::vector<PaintBucket> m_paint_buckets;
    PXR_NS::VtVec3iArray m_triangulated_vertex_indices;
    std::vector<PXR_NS::GfVec4f> m_points_ss;
    PXR_NS::VtVec3fArray points_world;
    PXR_NS::VtVec2fArray m_uvs;
    PXR_NS::VtVec3iArray m_st_tri_indices;
    std::unordered_map<int, VertSeamData> m_vert_seam_data;

    PXR_NS::GfVec2i m_image_size = PXR_NS::GfVec2i(0);
    bool m_occlude = true;

    std::shared_ptr<Ramp<float>> m_falloff_curve;
    std::shared_ptr<TextureData> m_texture_data;
};

OPENDCC_NAMESPACE_CLOSE
