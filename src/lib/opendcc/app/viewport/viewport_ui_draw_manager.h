/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/plane.h>
#include <queue>
#include <memory>

OPENDCC_NAMESPACE_OPEN

class ViewportUiDrawManager;

namespace draw_utils
{
    OPENDCC_API void draw_axis(ViewportUiDrawManager* draw_manager, const PXR_NS::GfMatrix4f& mvp, const PXR_NS::GfMatrix4f& model,
                               const PXR_NS::GfVec4f& color, const PXR_NS::GfVec3f& orig, const PXR_NS::GfVec3f& axis, const PXR_NS::GfVec3f& vtx,
                               const PXR_NS::GfVec3f& vty, float fct, float fct2, uint32_t selection_id = 0);
    OPENDCC_API void draw_cube(ViewportUiDrawManager* draw_manager, const PXR_NS::GfMatrix4f& mvp, const PXR_NS::GfMatrix4f& model,
                               const PXR_NS::GfVec4f& color, float size = 1.0f, uint32_t depth_priority = 0, uint32_t selection_id = 0);
    OPENDCC_API void draw_circle(ViewportUiDrawManager* draw_manager, const PXR_NS::GfMatrix4f& mvp, const PXR_NS::GfVec4f& color,
                                 const PXR_NS::GfVec3f& orig, const PXR_NS::GfVec3f& vtx, const PXR_NS::GfVec3f& vty, float line_width = 1.0f,
                                 uint32_t depth_priority = 0, uint32_t selection_id = 0);
    OPENDCC_API void draw_circle_half(ViewportUiDrawManager* draw_manager, const PXR_NS::GfMatrix4f& mvp, const PXR_NS::GfVec4f& color,
                                      const PXR_NS::GfVec3f& orig, const PXR_NS::GfVec3f& vtx, const PXR_NS::GfVec3f& vty,
                                      PXR_NS::GfPlane& camera_plane, uint32_t depth_priority = 0, uint32_t selection_id = 0);
    OPENDCC_API void draw_outlined_quad(ViewportUiDrawManager* draw_manager, const PXR_NS::GfMatrix4f& mvp, const PXR_NS::GfVec4f& color,
                                        const PXR_NS::GfVec4f& outline_color, const std::vector<PXR_NS::GfVec3f> vertices,
                                        float outline_width /* = 1.0f */, int handle_priority /* = 0 */, uint32_t selection_id /* = 0 */);
    OPENDCC_API void draw_outlined_circle(ViewportUiDrawManager* draw_manager, const PXR_NS::GfMatrix4f& mvp, const PXR_NS::GfVec4f& color,
                                          const PXR_NS::GfVec4f& outline_color, const PXR_NS::GfVec3f& orig, const PXR_NS::GfVec3f& vtx,
                                          const PXR_NS::GfVec3f& vty, float outline_width /* = 1.0f */, uint32_t handle_priority /* = 0 */,
                                          uint32_t selection_id /* = 0 */);
};

class OPENDCC_API ViewportUiDrawManager
{
public:
    ViewportUiDrawManager() = default;

    ViewportUiDrawManager(unsigned width, unsigned height);
    virtual ~ViewportUiDrawManager();

    enum class Selectability
    {
        NON_SELECTABLE,
        SELECTABLE
    };
    enum class PaintStyle
    {
        FLAT,
        SHADED,
        STIPPLED
    };

    enum PrimitiveType
    {
        PrimitiveTypeLines,
        PrimitiveTypeLinesStrip,
        PrimitiveTypeLinesLoop,
        PrimitiveTypeTriangles,
        PrimitiveTypeTriangleFan,
        PrimitiveTypePoints
    };

    uint32_t create_selection_id();
    uint32_t get_current_selection() const { return m_selected_handle; };
    void begin_drawable(uint32_t selection_id = 0);
    void set_depth_priority(uint32_t depth_priority = 0);
    void set_color(const PXR_NS::GfVec3f& color);
    void set_color(const PXR_NS::GfVec4f& color);
    void set_mvp_matrix(const PXR_NS::GfMatrix4f& mvp);
    void set_paint_style(PaintStyle style);
    void set_prim_type(PrimitiveType mode);
    void line(const PXR_NS::GfVec3f& start, const PXR_NS::GfVec3f& end);
    void set_line_width(float width);
    void rect2d(const PXR_NS::GfVec2f& start, const PXR_NS::GfVec2f& end);
    void set_point_size(float point_size);
    void mesh(PrimitiveType prim_type, const std::vector<PXR_NS::GfVec3f>& vertex_buffer);
    void arc(const PXR_NS::GfVec3f& center, const PXR_NS::GfVec3f& start, const PXR_NS::GfVec3f& end, const PXR_NS::GfVec3f& normal, double radius,
             bool filled = true);
    void cone(const PXR_NS::GfVec3f& base, const PXR_NS::GfVec3f& dir, float radius, float height, bool filled = true);
    void end_drawable();

    void execute_draw_queue(unsigned width, unsigned height, uint32_t mouse_x, uint32_t mouse_y, const PXR_NS::GfMatrix4f& proj,
                            const PXR_NS::GfMatrix4f& view);

private:
    void read_selected_handle_id(int x, int y);
    void create_framebuffer(unsigned width, unsigned height);

    friend void draw_utils::draw_axis(ViewportUiDrawManager*, const PXR_NS::GfMatrix4f&, const PXR_NS::GfMatrix4f&, const PXR_NS::GfVec4f&,
                                      const PXR_NS::GfVec3f&, const PXR_NS::GfVec3f& axis, const PXR_NS::GfVec3f&, const PXR_NS::GfVec3f&, float,
                                      float, uint32_t);
    friend void draw_utils::draw_cube(ViewportUiDrawManager*, const PXR_NS::GfMatrix4f&, const PXR_NS::GfMatrix4f&, const PXR_NS::GfVec4f&, float,
                                      uint32_t, uint32_t);
    struct GpuDataPimpl* m_gpu_data;
    unsigned m_width, m_height;
    bool m_flushed;
    struct DrawCall
    {
        PaintStyle paint_style;
        PrimitiveType prim_type;
        PXR_NS::GfVec4f color;
        PXR_NS::GfMatrix4f mvp_matrix;
        PXR_NS::GfMatrix4f model_matrix;
        std::vector<PXR_NS::GfVec3f> vertex_buffer;
        float point_size = 1.0f;
        float line_width = 1.0f;
        uint32_t selection_id = 0;
        uint32_t depth_priority = 0;
        uint32_t vao = 0;
        uint32_t vbo = 0;

        bool operator<(const DrawCall& other) const { return depth_priority > other.depth_priority; }
    };
    DrawCall m_current_draw_call;
    std::priority_queue<DrawCall, std::vector<DrawCall>> m_draw_queue;
    std::priority_queue<DrawCall, std::vector<DrawCall>> m_transparent_queue;
    uint32_t m_selection_counter = 0;
    uint32_t m_selected_handle = 0;
    std::vector<uint32_t> m_selection_buffer;

    static const int m_pick_radius;
};

OPENDCC_NAMESPACE_CLOSE
