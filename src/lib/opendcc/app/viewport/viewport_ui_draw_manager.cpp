// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_ui_draw_manager.h"
#if PXR_VERSION < 2108
#include <GL/glew.h>
#else
#include <pxr/imaging/garch/gl.h>
#endif
#include <QImage>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <pxr/base/gf/matrix3f.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

const int ViewportUiDrawManager::m_pick_radius = 6;

namespace
{
    static const GLuint stipple_pattern[] = {
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
        0x55555555, // 0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#0#
        0x00000000, // 00000000000000000000000000000000
    };
}

struct GpuDataPimpl
{
    std::unique_ptr<QOpenGLShaderProgram> m_render_program;
    std::unique_ptr<QOpenGLShaderProgram> m_picking_program;
    std::unique_ptr<QOpenGLFramebufferObject> m_framebuffer;
    uint32_t m_pixel_pack_buffer[2] = { 0, 0 };
    uint32_t m_current_id = 0;

    int m_mvp_picking = -1;
    int m_handle_index = -1;
    int m_handle_priority = -1;

    int m_mvp_rendering = -1;
    int m_model = -1;
    int m_normal_mat = -1;
    int m_view = -1;

    int m_color = -1;
    int m_paint_style = -1;
};

ViewportUiDrawManager::ViewportUiDrawManager(unsigned width, unsigned height)
{
    static const char* picking_vert_src = R"#(#version 330
layout(location = 0) in vec3 position;

uniform mat4 mvpMatrix;
void main()
{
    gl_Position = vec4(position, 1) * mvpMatrix;
}
)#";
    static const char* picking_frag_src = R"#(#version 330

layout(location = 0) out vec4 encoded_index;

uniform int handle_index;
uniform int handle_priority;

vec4 int_to_vec4()
{
    return vec4(
        (handle_index >> 16 & 0xFF) / 255.0,
        (handle_index >> 8 & 0xFF) / 255.0,
        (handle_index >> 0 & 0xFF) / 255.0,
        handle_priority / 255.0
    );
}


void main() {
    encoded_index = int_to_vec4();
}
)#";
    static const char* render_vert_src = R"#(#version 330
uniform mat4 mvpMatrix;
uniform mat4 model;
uniform mat4 view;
uniform mat3 normal_mat;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 Pv;
out vec3 Nv;
void main()
{
    gl_Position = vec4(position, 1) * mvpMatrix;
    Pv = (view * model * vec4(position, 1)).xyz;
    Nv = (normal_mat) * normal;
}
)#";

    static const char* render_frag_src = R"#(#version 330

layout(location = 0) out vec4 outColor;

uniform vec4 color;
uniform int paint_style;

#define FLAT 0
#define SHADED 1
#define STIPPLED 2

in vec3 Pv;
in vec3 Nv;

void main() {
    if (paint_style == FLAT || paint_style == STIPPLED)
    {
        outColor = color;
    }
    else if (paint_style == SHADED)
    {
        vec3 n = normalize(Nv);
        vec3 l = normalize(vec3(0, 0, 1));

        vec3 ambient = color.rgb * 0.3;
        vec3 diffuse = color.rgb * 0.85;
        float diffuse_c = max(dot(n, l), 0.0);
        outColor = vec4(ambient + diffuse * diffuse_c, color.a);
    }
}
)#";
    m_gpu_data = new GpuDataPimpl;
    m_gpu_data->m_render_program = std::make_unique<QOpenGLShaderProgram>();
    m_gpu_data->m_render_program->addShaderFromSourceCode(QOpenGLShader::Vertex, render_vert_src);
    m_gpu_data->m_render_program->addShaderFromSourceCode(QOpenGLShader::Fragment, render_frag_src);
    m_gpu_data->m_render_program->link();

    m_gpu_data->m_picking_program = std::make_unique<QOpenGLShaderProgram>();
    m_gpu_data->m_picking_program->addShaderFromSourceCode(QOpenGLShader::Vertex, picking_vert_src);
    m_gpu_data->m_picking_program->addShaderFromSourceCode(QOpenGLShader::Fragment, picking_frag_src);
    m_gpu_data->m_picking_program->link();

    m_gpu_data->m_mvp_picking = m_gpu_data->m_picking_program->uniformLocation("mvpMatrix");
    m_gpu_data->m_mvp_rendering = m_gpu_data->m_render_program->uniformLocation("mvpMatrix");
    m_gpu_data->m_handle_index = m_gpu_data->m_picking_program->uniformLocation("handle_index");
    m_gpu_data->m_handle_priority = m_gpu_data->m_picking_program->uniformLocation("handle_priority");
    m_gpu_data->m_model = m_gpu_data->m_render_program->uniformLocation("model");
    m_gpu_data->m_view = m_gpu_data->m_render_program->uniformLocation("view");
    m_gpu_data->m_normal_mat = m_gpu_data->m_render_program->uniformLocation("normal_mat");
    m_gpu_data->m_color = m_gpu_data->m_render_program->uniformLocation("color");
    m_gpu_data->m_paint_style = m_gpu_data->m_render_program->uniformLocation("paint_style");

    create_framebuffer(width, height);
    m_selection_buffer.resize((m_pick_radius * 2 + 1) * (m_pick_radius * 2 + 1));
    glGenBuffers(2, m_gpu_data->m_pixel_pack_buffer);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_gpu_data->m_pixel_pack_buffer[0]);
    glBufferData(GL_PIXEL_PACK_BUFFER, m_selection_buffer.size() * sizeof(uint32_t), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_gpu_data->m_pixel_pack_buffer[1]);
    glBufferData(GL_PIXEL_PACK_BUFFER, m_selection_buffer.size() * sizeof(uint32_t), 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    m_flushed = true;
}

ViewportUiDrawManager::~ViewportUiDrawManager()
{
    if (m_gpu_data)
    {
        glDeleteBuffers(2, m_gpu_data->m_pixel_pack_buffer);
        delete m_gpu_data;
    }
}

uint32_t ViewportUiDrawManager::create_selection_id()
{
    const auto new_selection_id = ++m_selection_counter & 0x00FFFFFF;
    m_selection_counter = new_selection_id;
    return new_selection_id;
}

void ViewportUiDrawManager::read_selected_handle_id(int x, int y)
{
    if (x < 0 || x > (int)m_width - 1 || y < 0 || y > (int)m_height - 1)
        return;

    m_gpu_data->m_current_id = (m_gpu_data->m_current_id + 1) % 2;
    const auto next_id = (m_gpu_data->m_current_id + 1) % 2;
    m_gpu_data->m_framebuffer->bind();
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_gpu_data->m_pixel_pack_buffer[m_gpu_data->m_current_id]);
    glReadPixels(x - m_pick_radius, m_height - y - m_pick_radius - 1, 13, 13, GL_BGRA, GL_UNSIGNED_BYTE, 0);
    m_gpu_data->m_framebuffer->bindDefault();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_gpu_data->m_pixel_pack_buffer[next_id]);
    auto data = (uint32_t*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    memcpy(m_selection_buffer.data(), data, m_selection_buffer.size() * sizeof(uint32_t));
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    uint32_t selected_handle = 0;
    int selected_handle_priority = -1;

    for (const uint32_t encoded_handle_value : m_selection_buffer)
    {
        const uint32_t handle_priority = encoded_handle_value >> 24;
        if ((int)handle_priority > selected_handle_priority && encoded_handle_value != 0)
        {
            selected_handle = encoded_handle_value & 0x00FFFFFF;
            selected_handle_priority = handle_priority;
        }
    }

    m_selected_handle = selected_handle;
}

void ViewportUiDrawManager::create_framebuffer(unsigned width, unsigned height)
{
    QOpenGLFramebufferObjectFormat fmt;
    fmt.setAttachment(QOpenGLFramebufferObject::Attachment::Depth);
    m_gpu_data->m_framebuffer = std::make_unique<QOpenGLFramebufferObject>(width, height, fmt);
    m_gpu_data->m_framebuffer->addColorAttachment(width, height, GL_RGBA);
    m_width = width;
    m_height = height;
}

void ViewportUiDrawManager::begin_drawable(uint32_t selection_id /*= 0*/)
{
    if (!m_flushed)
        end_drawable();
    m_current_draw_call.vertex_buffer.clear();
    m_current_draw_call.color = GfVec4f(1, 1, 1, 1);
    m_current_draw_call.mvp_matrix.SetIdentity();
    m_current_draw_call.model_matrix.SetIdentity();
    m_current_draw_call.paint_style = PaintStyle::FLAT;
    m_current_draw_call.prim_type = PrimitiveTypeTriangleFan;
    m_current_draw_call.selection_id = selection_id;
    m_current_draw_call.depth_priority = 0;
    m_current_draw_call.point_size = 1.0f;
    m_current_draw_call.line_width = 1.0f;
}

void ViewportUiDrawManager::set_depth_priority(uint32_t depth_priority /*= 0*/)
{
    m_current_draw_call.depth_priority = depth_priority;
}

void ViewportUiDrawManager::set_color(const GfVec3f& color)
{
    m_current_draw_call.color = GfVec4f(color[0], color[1], color[2], 1);
}

void ViewportUiDrawManager::set_color(const GfVec4f& color)
{
    m_current_draw_call.color = color;
}

void ViewportUiDrawManager::end_drawable()
{
    if (m_current_draw_call.color[3] == 1)
        m_draw_queue.push(m_current_draw_call);
    else
        m_transparent_queue.push(m_current_draw_call);
    m_flushed = true;
}

void ViewportUiDrawManager::set_mvp_matrix(const GfMatrix4f& mvp)
{
    m_current_draw_call.mvp_matrix = mvp;
}

void ViewportUiDrawManager::set_paint_style(PaintStyle style)
{
    m_current_draw_call.paint_style = style;
}

void ViewportUiDrawManager::set_prim_type(PrimitiveType mode)
{
    m_current_draw_call.prim_type = mode;
}

void ViewportUiDrawManager::line(const GfVec3f& start, const GfVec3f& end)
{
    m_current_draw_call.vertex_buffer.push_back(start);
    m_current_draw_call.vertex_buffer.push_back(end);
}

void ViewportUiDrawManager::set_line_width(float width)
{
    m_current_draw_call.line_width = width;
}

void ViewportUiDrawManager::rect2d(const GfVec2f& start, const GfVec2f& end)
{
    m_current_draw_call.vertex_buffer.emplace_back(start[0], start[1], 0);
    m_current_draw_call.vertex_buffer.emplace_back(end[0], start[1], 0);
    m_current_draw_call.vertex_buffer.emplace_back(end[0], end[1], 0);
    m_current_draw_call.vertex_buffer.emplace_back(start[0], end[1], 0);
    m_current_draw_call.vertex_buffer.emplace_back(start[0], start[1], 0);
}

void ViewportUiDrawManager::set_point_size(float point_size)
{
    m_current_draw_call.point_size = point_size;
}

void ViewportUiDrawManager::execute_draw_queue(unsigned width, unsigned height, uint32_t mouse_x, uint32_t mouse_y, const GfMatrix4f& proj,
                                               const GfMatrix4f& view)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY, 0, -1, "ViewportUiDrawManager");

    if (width != m_width || height != m_height)
        create_framebuffer(width, height);

    std::vector<DrawCall> opaque;
    opaque.reserve(m_draw_queue.size());
    std::vector<DrawCall> transparent;
    transparent.reserve(transparent.size());
    while (!m_draw_queue.empty())
    {
        opaque.emplace_back(m_draw_queue.top());
        m_draw_queue.pop();
    }
    while (!m_transparent_queue.empty())
    {
        transparent.emplace_back(m_transparent_queue.top());
        m_transparent_queue.pop();
    }

    m_gpu_data->m_framebuffer->bind();
    m_gpu_data->m_picking_program->bind();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::vector<std::vector<DrawCall>*> queues = { &opaque, &transparent };

    GLint line_width_range[2];
    glGetIntegerv(GL_SMOOTH_LINE_WIDTH_RANGE, line_width_range);
    for (auto& queue : queues)
    {
        for (auto& draw_call : *queue)
        {
            glUniformMatrix4fv(m_gpu_data->m_mvp_picking, 1, GL_TRUE, &draw_call.mvp_matrix[0][0]);
            glUniform1i(m_gpu_data->m_handle_index, draw_call.selection_id);
            glUniform1i(m_gpu_data->m_handle_priority, draw_call.depth_priority);

            glGenVertexArrays(1, &draw_call.vao);
            glGenBuffers(1, &draw_call.vbo);

            glBindVertexArray(draw_call.vao);
            glBindBuffer(GL_ARRAY_BUFFER, draw_call.vbo);
            glBufferData(GL_ARRAY_BUFFER, draw_call.vertex_buffer.size() * 3 * sizeof(float), draw_call.vertex_buffer.data(), GL_STATIC_DRAW);
            const auto stride = draw_call.paint_style == PaintStyle::SHADED ? 6 * sizeof(float) : 3 * sizeof(float);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);

            GLuint draw_mode;
            draw_call.line_width = GfClamp(draw_call.line_width, line_width_range[0], line_width_range[1]);

            if (draw_call.paint_style == PaintStyle::STIPPLED)
            {
                glEnable(GL_LINE_STIPPLE);
                glEnable(GL_POLYGON_STIPPLE);
                glLineStipple(2, 0xAAAA);
                glPolygonStipple(reinterpret_cast<const GLubyte*>(stipple_pattern));
            }
            else if (draw_call.paint_style == PaintStyle::SHADED)
            {
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
            }
            switch (draw_call.prim_type)
            {
            case PrimitiveTypeLines:
                draw_mode = GL_LINES;
                glLineWidth(draw_call.line_width);
                break;
            case PrimitiveTypeLinesStrip:
                draw_mode = GL_LINE_STRIP;
                glLineWidth(draw_call.line_width);
                break;
            case PrimitiveTypeLinesLoop:
                draw_mode = GL_LINE_LOOP;
                glLineWidth(draw_call.line_width);
                break;
            case PrimitiveTypePoints:
                draw_mode = GL_POINTS;
                glPointSize(draw_call.point_size);
                break;
            case PrimitiveTypeTriangleFan:
                draw_mode = GL_TRIANGLE_FAN;
                break;
            case PrimitiveTypeTriangles:
            default:
                draw_mode = GL_TRIANGLES;
                break;
            }
            const auto vertex_buffer_size = draw_call.vertex_buffer.size() / (draw_call.paint_style == PaintStyle::SHADED ? 2 : 1);
            glDrawArrays(draw_mode, 0, vertex_buffer_size);
            if (draw_call.paint_style == PaintStyle::STIPPLED)
            {
                glDisable(GL_LINE_STIPPLE);
                glDisable(GL_POLYGON_STIPPLE);
            }
        }
    }

    read_selected_handle_id(mouse_x, mouse_y);

    QOpenGLFramebufferObject::bindDefault();
    m_gpu_data->m_render_program->bind();

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_MULTISAMPLE);

    for (int i = 0; i < queues.size(); i++)
    {
        if (i == 0) // opaque
        {
            glDisable(GL_BLEND);
        }
        else // transparent
        {
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        for (auto& draw_call : *queues[i])
        {
            glUniformMatrix4fv(m_gpu_data->m_mvp_rendering, 1, GL_TRUE, &draw_call.mvp_matrix[0][0]);
            glUniform4f(m_gpu_data->m_color, draw_call.color[0], draw_call.color[1], draw_call.color[2], draw_call.color[3]);
            glUniform1i(m_gpu_data->m_paint_style, static_cast<GLint>(draw_call.paint_style));
            glBindVertexArray(draw_call.vao);

            if (draw_call.paint_style == PaintStyle::STIPPLED)
            {
                glEnable(GL_LINE_STIPPLE);
                glEnable(GL_POLYGON_STIPPLE);
                glLineStipple(2, 0xAAAA);
                glPolygonStipple(reinterpret_cast<const GLubyte*>(stipple_pattern));
            }
            else if (draw_call.paint_style == PaintStyle::SHADED)
            {
                glUniformMatrix4fv(m_gpu_data->m_model, 1, GL_FALSE, draw_call.model_matrix.data());
                glUniformMatrix4fv(m_gpu_data->m_view, 1, GL_FALSE, view.data());
                const auto normal_mat = (draw_call.model_matrix * view).GetInverse().GetTranspose().ExtractRotationMatrix();
                glUniformMatrix3fv(m_gpu_data->m_normal_mat, 1, GL_FALSE, normal_mat.data());
            }
            GLenum draw_mode;
            switch (draw_call.prim_type)
            {
            case PrimitiveTypeLines:
                draw_mode = GL_LINES;
                glLineWidth(draw_call.line_width);
                break;
            case PrimitiveTypeLinesStrip:
                draw_mode = GL_LINE_STRIP;
                glLineWidth(draw_call.line_width);
                break;
            case PrimitiveTypeLinesLoop:
                draw_mode = GL_LINE_LOOP;
                glLineWidth(draw_call.line_width);
                break;
            case PrimitiveTypePoints:
                draw_mode = GL_POINTS;
                glPointSize(draw_call.point_size);
                break;
            case PrimitiveTypeTriangleFan:
                draw_mode = GL_TRIANGLE_FAN;
                break;
            case PrimitiveTypeTriangles:
            default:
                draw_mode = GL_TRIANGLES;
                break;
            }
            glDrawArrays(draw_mode, 0, draw_call.vertex_buffer.size() / (draw_call.paint_style == PaintStyle::SHADED ? 2 : 1));

            if (draw_call.paint_style == PaintStyle::STIPPLED)
            {
                glDisable(GL_LINE_STIPPLE);
                glDisable(GL_POLYGON_STIPPLE);
            }
            glBindVertexArray(0);
            glDeleteVertexArrays(1, &draw_call.vao);
            glDeleteBuffers(1, &draw_call.vbo);
        }
    }
    m_gpu_data->m_render_program->release();

    glPopDebugGroup();
}

void ViewportUiDrawManager::mesh(PrimitiveType prim_type, const std::vector<GfVec3f>& vertex_buffer)
{
    set_prim_type(prim_type);
    m_current_draw_call.vertex_buffer = vertex_buffer;
}

void ViewportUiDrawManager::arc(const GfVec3f& center, const GfVec3f& start, const GfVec3f& end, const GfVec3f& normal, double radius,
                                bool filled /*= true*/)
{
    static const int MAX_SEGMENTS = 30;
    static const double step = 1.0 / MAX_SEGMENTS;
    auto start_normalized = start.GetNormalized();
    auto end_normalized = end.GetNormalized();
    int sign;
    if (GfAbs(GfDot(start_normalized, end_normalized)) > 0.999)
        sign = 1;
    else
        sign = GfDot(GfCross(end_normalized, start_normalized), normal) > 0 ? 1 : -1;

    const auto draw_arc = [this, &center, radius](const GfVec3f& start, const GfVec3f& end, size_t segment_count) {
        for (size_t i = 0; i <= segment_count; i++)
        {
            const auto alpha = step * i;
            auto vt = GfSlerp(alpha, start, end) * radius;
            vt += center;
            m_current_draw_call.vertex_buffer.push_back(std::move(vt));
        }
    };
    m_current_draw_call.vertex_buffer.push_back(center);
    if (sign < 0)
    {
        end_normalized = -start_normalized;
        draw_arc(start_normalized, end_normalized, MAX_SEGMENTS - 1);
        start_normalized = end_normalized;
        end_normalized = end.GetNormalized();
    }
    draw_arc(start_normalized, end_normalized, MAX_SEGMENTS);
}

void draw_utils::draw_axis(ViewportUiDrawManager* draw_manager, const GfMatrix4f& mvp, const PXR_NS::GfMatrix4f& model, const GfVec4f& color,
                           const GfVec3f& orig, const GfVec3f& axis, const GfVec3f& vtx, const GfVec3f& vty, float fct, float fct2,
                           uint32_t selection_id)
{
    draw_manager->begin_drawable(selection_id);
    draw_manager->set_color(color);
    draw_manager->set_mvp_matrix(mvp);
    draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::FLAT);
    draw_manager->set_line_width(2.0f);
    draw_manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeLines);
    draw_manager->line(orig, orig + axis * fct2);
    draw_manager->end_drawable();

    draw_manager->begin_drawable(selection_id);
    draw_manager->set_color(color);
    draw_manager->set_mvp_matrix(mvp);
    draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::SHADED);
    std::vector<GfVec3f> points;
    for (int i = 0; i <= 30; i++)
    {
        GfVec3f p0, p1, p2;
        GfVec3f n0, n1, n2;

        p0 = vtx * cos(((2 * M_PI) / 30.0f) * i) * fct;
        p0 += vty * sin(((2 * M_PI) / 30.0f) * i) * fct;
        n0 = p0.GetNormalized();
        p0 += axis * fct2;
        p0 += orig;

        p1 = vtx * cos(((2 * M_PI) / 30.0f) * (i + 1)) * fct;
        p1 += vty * sin(((2 * M_PI) / 30.0f) * (i + 1)) * fct;
        n1 = p1.GetNormalized();
        p1 += axis * fct2;
        p1 += orig;

        p2 = orig + axis;
        const auto p2_p0 = (p0 - p2).GetNormalized();
        auto tang = GfCross(p2_p0, n0).GetNormalized();
        n0 = GfCross(tang, p2_p0).GetNormalized();

        const auto p1_p0 = (p1 - p2).GetNormalized();
        tang = GfCross(p1_p0, n1).GetNormalized();
        n1 = GfCross(tang, p1_p0).GetNormalized();

        n2 = (n0 + n1).GetNormalized();
        points.push_back(p0);
        points.push_back(n0);
        points.push_back(p1);
        points.push_back(n1);
        points.push_back(p2);
        points.push_back(n2);
    }
    const auto stride = 6;
    const auto tip_normal_offset = 5;
    const auto vertex_count = points.size() / stride;
    for (int i = 0; i < vertex_count; i++)
    {
        auto prev_id = i == 0 ? vertex_count - 1 : i - 1;
        auto next_id = i == vertex_count - 1 ? 0 : i + 1;

        prev_id = prev_id * stride + tip_normal_offset;
        next_id = next_id * stride + tip_normal_offset;
        const auto cur_id = i * stride + tip_normal_offset;
        points[cur_id] += points[prev_id] + points[next_id];
        points[cur_id].Normalize();
    }
    draw_manager->mesh(ViewportUiDrawManager::PrimitiveTypeTriangles, points);
    draw_manager->m_current_draw_call.model_matrix = model;
    draw_manager->end_drawable();
}

void draw_utils::draw_cube(ViewportUiDrawManager* draw_manager, const GfMatrix4f& mvp, const GfMatrix4f& model, const GfVec4f& color, float size,
                           uint32_t depth_priority, uint32_t selection_id)
{
    draw_manager->begin_drawable(selection_id);
    draw_manager->set_color(color);
    draw_manager->set_mvp_matrix(mvp);
    draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::SHADED);
    draw_manager->m_current_draw_call.model_matrix = model;
    std::vector<GfVec3f> points;

    points.emplace_back(size, size, size);
    points.emplace_back(GfVec3f::YAxis());
    points.emplace_back(size, size, -size);
    points.emplace_back(GfVec3f::YAxis());
    points.emplace_back(-size, size, -size);
    points.emplace_back(GfVec3f::YAxis());
    points.emplace_back(-size, size, -size);
    points.emplace_back(GfVec3f::YAxis());
    points.emplace_back(-size, size, size);
    points.emplace_back(GfVec3f::YAxis());
    points.emplace_back(size, size, size);
    points.emplace_back(GfVec3f::YAxis());

    points.emplace_back(-size, -size, -size);
    points.emplace_back(-GfVec3f::YAxis());
    points.emplace_back(size, -size, -size);
    points.emplace_back(-GfVec3f::YAxis());
    points.emplace_back(size, -size, size);
    points.emplace_back(-GfVec3f::YAxis());
    points.emplace_back(size, -size, size);
    points.emplace_back(-GfVec3f::YAxis());
    points.emplace_back(-size, -size, size);
    points.emplace_back(-GfVec3f::YAxis());
    points.emplace_back(-size, -size, -size);
    points.emplace_back(-GfVec3f::YAxis());

    points.emplace_back(size, size, size);
    points.emplace_back(GfVec3f::XAxis());
    points.emplace_back(size, size, -size);
    points.emplace_back(GfVec3f::XAxis());
    points.emplace_back(size, -size, -size);
    points.emplace_back(GfVec3f::XAxis());
    points.emplace_back(size, -size, -size);
    points.emplace_back(GfVec3f::XAxis());
    points.emplace_back(size, -size, size);
    points.emplace_back(GfVec3f::XAxis());
    points.emplace_back(size, size, size);
    points.emplace_back(GfVec3f::XAxis());

    points.emplace_back(-size, -size, -size);
    points.emplace_back(-GfVec3f::XAxis());
    points.emplace_back(-size, size, -size);
    points.emplace_back(-GfVec3f::XAxis());
    points.emplace_back(-size, size, size);
    points.emplace_back(-GfVec3f::XAxis());
    points.emplace_back(-size, size, size);
    points.emplace_back(-GfVec3f::XAxis());
    points.emplace_back(-size, -size, size);
    points.emplace_back(-GfVec3f::XAxis());
    points.emplace_back(-size, -size, -size);
    points.emplace_back(-GfVec3f::XAxis());

    points.emplace_back(-size, size, size);
    points.emplace_back(GfVec3f::ZAxis());
    points.emplace_back(size, size, size);
    points.emplace_back(GfVec3f::ZAxis());
    points.emplace_back(size, -size, size);
    points.emplace_back(GfVec3f::ZAxis());
    points.emplace_back(size, -size, size);
    points.emplace_back(GfVec3f::ZAxis());
    points.emplace_back(-size, -size, size);
    points.emplace_back(GfVec3f::ZAxis());
    points.emplace_back(-size, size, size);
    points.emplace_back(GfVec3f::ZAxis());

    points.emplace_back(size, -size, -size);
    points.emplace_back(-GfVec3f::ZAxis());
    points.emplace_back(size, size, -size);
    points.emplace_back(-GfVec3f::ZAxis());
    points.emplace_back(-size, size, -size);
    points.emplace_back(-GfVec3f::ZAxis());
    points.emplace_back(-size, size, -size);
    points.emplace_back(-GfVec3f::ZAxis());
    points.emplace_back(-size, -size, -size);
    points.emplace_back(-GfVec3f::ZAxis());
    points.emplace_back(size, -size, -size);
    points.emplace_back(-GfVec3f::ZAxis());

    draw_manager->mesh(ViewportUiDrawManager::PrimitiveTypeTriangles, points);
    draw_manager->set_depth_priority(depth_priority);
    draw_manager->end_drawable();
}

void draw_utils::draw_circle(ViewportUiDrawManager* draw_manager, const GfMatrix4f& mvp, const GfVec4f& color, const GfVec3f& orig,
                             const GfVec3f& vtx, const GfVec3f& vty, float lines_width, uint32_t depth_priority, uint32_t selection_id)
{
    draw_manager->begin_drawable(selection_id);
    draw_manager->set_color(color);
    draw_manager->set_mvp_matrix(mvp);
    draw_manager->set_depth_priority(depth_priority);
    draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::FLAT);
    draw_manager->set_line_width(lines_width);
    std::vector<GfVec3f> points;
    for (int i = 0; i < 50; i++)
    {
        GfVec3f vt;
        vt = vtx * cos((2 * M_PI / 50) * i);
        vt += vty * sin((2 * M_PI / 50) * i);
        vt += orig;
        points.push_back(vt);
    }
    draw_manager->mesh(ViewportUiDrawManager::PrimitiveTypeLinesLoop, points);
    draw_manager->end_drawable();
}
void draw_utils::draw_circle_half(ViewportUiDrawManager* draw_manager, const GfMatrix4f& mvp, const GfVec4f& color, const GfVec3f& orig,
                                  const GfVec3f& vtx, const GfVec3f& vty, GfPlane& camera_plane, uint32_t depth_priority, uint32_t selection_id)
{
    draw_manager->begin_drawable(selection_id);
    draw_manager->set_color(color);
    draw_manager->set_mvp_matrix(mvp);
    draw_manager->set_depth_priority(depth_priority);
    draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::FLAT);
    draw_manager->set_line_width(1.0f);
    std::vector<GfVec3f> points;
    for (int i = 0; i < 30; i++)
    {
        GfVec3f vt;
        vt = vtx * cos((M_PI / 30) * i);
        vt += vty * sin((M_PI / 30) * i);
        vt += orig;
        if (!camera_plane.IntersectsPositiveHalfSpace(vt))
        {
            points.push_back(vt);
        }
    }
    draw_manager->mesh(ViewportUiDrawManager::PrimitiveTypeLinesStrip, points);
    draw_manager->end_drawable();
}

void draw_utils::draw_outlined_quad(ViewportUiDrawManager* draw_manager, const GfMatrix4f& mvp, const GfVec4f& color, const GfVec4f& outline_color,
                                    const std::vector<GfVec3f> vertices, float outline_width /*= 1.0f*/, int handle_priority /*= 0*/,
                                    uint32_t selection_id /*= 0*/)
{
    if (!draw_manager)
        return;

    std::vector<GfVec3f> screen_space_verts;
    screen_space_verts.reserve(vertices.size());
    for (const auto& v : vertices)
    {
        GfVec4f v_ = GfVec4f(v[0], v[1], v[2], 1) * mvp;

        GfVec3f screen_space_pos;
        screen_space_pos[0] = v_[0] / v_[3];
        screen_space_pos[1] = v_[1] / v_[3];
        screen_space_pos[2] = 0.0;

        screen_space_verts.emplace_back(std::move(screen_space_pos));
    }
    draw_manager->begin_drawable(selection_id);
    draw_manager->set_color(color);
    draw_manager->set_depth_priority(handle_priority);
    draw_manager->set_mvp_matrix(GfMatrix4f().SetIdentity());
    draw_manager->mesh(ViewportUiDrawManager::PrimitiveTypeTriangleFan, screen_space_verts);
    draw_manager->end_drawable();

    draw_manager->begin_drawable(selection_id);
    draw_manager->set_color(outline_color);
    draw_manager->set_line_width(outline_width);
    draw_manager->set_depth_priority(handle_priority);
    draw_manager->set_mvp_matrix(mvp);
    draw_manager->mesh(ViewportUiDrawManager::PrimitiveTypeLinesLoop, vertices);
    draw_manager->end_drawable();
}

void draw_utils::draw_outlined_circle(ViewportUiDrawManager* draw_manager, const GfMatrix4f& mvp, const GfVec4f& color, const GfVec4f& outline_color,
                                      const GfVec3f& orig, const GfVec3f& vtx, const GfVec3f& vty, float outline_width /* = 1.0f */,
                                      uint32_t depth_priority /* = 0 */, uint32_t selection_id /* = 0 */)
{
    draw_manager->begin_drawable(selection_id);
    draw_manager->set_color(color);
    draw_manager->set_mvp_matrix(mvp);
    draw_manager->set_depth_priority(depth_priority);
    draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::FLAT);
    std::vector<GfVec3f> filled_circle_points;
    filled_circle_points.push_back(orig);
    for (int i = 0; i <= 50; i++)
    {
        GfVec3f vt;
        vt = vtx * cos((2 * M_PI / 50) * i);
        vt += vty * sin((2 * M_PI / 50) * i);
        vt += orig;
        filled_circle_points.push_back(vt);
    }
    draw_manager->mesh(ViewportUiDrawManager::PrimitiveTypeTriangleFan, filled_circle_points);
    draw_manager->end_drawable();

    draw_manager->begin_drawable(selection_id);
    draw_manager->set_color(outline_color);
    draw_manager->set_mvp_matrix(mvp);
    draw_manager->set_depth_priority(depth_priority);
    draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::FLAT);
    draw_manager->set_line_width(outline_width);
    std::vector<GfVec3f> points;
    for (int i = 0; i <= 50; i++)
    {
        GfVec3f vt;
        vt = vtx * cos((2 * M_PI / 50) * i);
        vt += vty * sin((2 * M_PI / 50) * i);
        vt += orig;
        points.push_back(vt);
    }
    draw_manager->mesh(ViewportUiDrawManager::PrimitiveTypeLinesLoop, points);
    draw_manager->end_drawable();
}

OPENDCC_NAMESPACE_CLOSE
