// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/uv_editor/uv_scale_manipulator.h"
#include "opendcc/usd_editor/uv_editor/uv_editor_gl_widget.h"
#include "opendcc/usd_editor/uv_editor/utils.h"

#include "opendcc/app/viewport/viewport_camera_controller.h"

#include <pxr/imaging/cameraUtil/conformWindow.h>

#include <QMouseEvent>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

static float s_axis_length = 0.63f;
static float s_triangle_base = 0.07f;
static float s_triangle_height = 0.1f;
static float s_quad_length = 0.1f * s_axis_length;

static const GfVec2f origin(0.0f, 0.0f);
static const GfVec2f x_axis(1.0f, 0.0f);
static const GfVec2f y_axis(0.0f, 1.0f);

// clang-format off
static const GfVec4f s_x_color           (1.000f, 0.000f, 0.000f, 1.000f);
static const GfVec4f s_y_color           (0.000f, 1.000f, 0.000f, 1.000f);

static const GfVec4f s_free_color        (0.392f, 0.863f, 1.000f, 0.400f);

static const GfVec4f s_axis_select_color (1.000f, 1.000f, 0.000f, 1.000f);
static const GfVec4f s_free_select_color (1.000f, 1.000f, 0.000f, 0.500f);

static const GfVec4f s_axis_hover_color  (1.000f, 0.750f, 0.500f, 1.000f);
static const GfVec4f s_free_hover_color  (1.000f, 0.750f, 0.500f, 0.500f);
// clang-format on

UvScaleManipulator::UvScaleManipulator(UvTool* tool)
    : m_tool(tool)
{
    create_direction_handles();
}

UvScaleManipulator::~UvScaleManipulator() {}

void UvScaleManipulator::on_mouse_press(QMouseEvent* event)
{
    m_direction = Direction::None;
    const auto buttons = event->buttons();
    const auto left = buttons & Qt::LeftButton;
    if (!left)
    {
        return;
    }

    auto widget = m_tool->get_widget();
    auto manager = widget->get_draw_manager();
    auto iter = m_handle_to_direction.find(manager->get_current_selection());
    if (iter == m_handle_to_direction.end())
    {
        return;
    }
    m_direction = iter->second;

    const auto screen = event->pos();
    const auto controller = widget->get_camera_controller();
    const auto frustum = controller->get_frustum();

    const auto w = widget->width();
    const auto h = widget->height();

    m_click = screen_to_clip(screen.x(), screen.y());
    m_click_moved = m_click;

    m_prev_pos = m_pos;
}

void UvScaleManipulator::on_mouse_move(QMouseEvent* event)
{
    if (m_direction == Direction::None)
    {
        return;
    }

    auto widget = m_tool->get_widget();
    const auto screen = event->pos();
    const auto controller = widget->get_camera_controller();
    const auto frustum = controller->get_frustum();

    const auto w = widget->width();
    const auto h = widget->height();

    m_click_moved = screen_to_clip(screen.x(), screen.y());
}

void UvScaleManipulator::on_mouse_release(QMouseEvent* event)
{
    m_direction = Direction::None;
    m_prev_pos.Set(0.0f, 0.0f);
    m_click.Set(0.0f, 0.0f);
    m_delta.Set(0.0f, 0.0f);
    m_click_moved.Set(0.0f, 0.0f);
}

void UvScaleManipulator::draw(ViewportUiDrawManager* manager)
{
    const auto colors = get_colors(manager->get_current_selection());

    auto widget = m_tool->get_widget();
    const auto controller = widget->get_camera_controller();
    auto frustum = controller->get_frustum();

    const auto device_pixel_ratio = widget->devicePixelRatio();
    const auto w = device_pixel_ratio * widget->width();
    const auto h = device_pixel_ratio * widget->height();
    CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit, h != 0 ? static_cast<double>(w) / h : 1.0);

    const auto view = frustum.ComputeViewMatrix();
    const auto proj = frustum.ComputeProjectionMatrix();

    const auto vp = view * proj;

    auto center_4d = GfVec4d(m_pos[0], m_pos[1], 0.0f, 1.0f);
    center_4d = center_4d * vp;
    const double display_scale = Application::instance().get_settings()->get("viewport.manipulators.global_scale", 1.0);
    double screen_factor = display_scale * 0.15 * center_4d[3];

    if (frustum.GetProjectionType() == GfFrustum::ProjectionType::Perspective)
    {
        screen_factor = screen_factor * frustum.GetFOV() / 35; // reference value for gizmo
    }
    else
    {
        const double right = frustum.GetWindow().GetMax()[0];
        const double left = frustum.GetWindow().GetMin()[0];
        screen_factor = screen_factor * (right - left);
    }

    auto scale = GfMatrix4d();
    scale.SetScale(screen_factor);

    const auto center_3d = GfVec3d(m_pos[0], m_pos[1], 0);

    auto model = GfMatrix4d();
    model.SetIdentity();
    model.SetTranslate(center_3d);
    model = scale * model;

    const auto mvp = GfMatrix4f(model * vp);

    auto inverse_mvp = mvp.GetInverse();
    auto start_vector = inverse_mvp.Transform(GfVec3f(m_click[0], m_click[1], 0.0f));
    auto end_vector = inverse_mvp.Transform(GfVec3f(m_click_moved[0], m_click_moved[1], 0.0f));
    auto delta = end_vector - start_vector;
    m_delta[0] = delta[0];
    m_delta[1] = delta[1];

    switch (m_direction)
    {
    case Direction::Horizontal:
    {
        m_delta[1] = 0.0f;
        break;
    }
    case Direction::Vertical:
    {
        m_delta[0] = 0.0f;
        break;
    }
    case Direction::Free:
    {
        m_delta[1] = m_delta[0];
        break;
    }
    }

    AxisInfo axis_info;
    axis_info.begin = origin;
    axis_info.mvp = mvp;

    // x
    {
        const auto color = colors.at(Direction::Horizontal);
        const auto id = m_direction_to_handle[Direction::Horizontal];

        axis_info.color = color;
        axis_info.direction = x_axis;
        axis_info.length = s_axis_length + m_delta[0];
        draw_axis(manager, axis_info, id);

        const GfVec2f diff = GfVec2f(s_quad_length, s_quad_length);

        QuadInfo quad_info;
        quad_info.mvp = mvp;
        quad_info.max = diff;
        quad_info.min = -diff;
        quad_info.color = color;
        quad_info.depth_priority = 255.0f;
        quad_info.outlined = false;

        draw_quad(manager, quad_info, id, axis_info.length);
    }

    // y
    {
        const auto color = colors.at(Direction::Vertical);
        const auto id = m_direction_to_handle[Direction::Vertical];

        axis_info.color = color;
        axis_info.direction = y_axis;
        axis_info.length = s_axis_length + m_delta[1];
        draw_axis(manager, axis_info, id);

        const GfVec2f diff = GfVec2f(s_quad_length, s_quad_length);

        QuadInfo quad_info;
        quad_info.mvp = mvp;
        quad_info.max = diff;
        quad_info.min = -diff;
        quad_info.color = color;
        quad_info.depth_priority = 255.0f;
        quad_info.outlined = false;

        draw_quad(manager, quad_info, id, 0, axis_info.length);
    }

    // free quad
    {
        auto color = colors.at(Direction::Free);
        const auto id = m_direction_to_handle[Direction::Free];
        const GfVec2f diff = GfVec2f(s_quad_length, s_quad_length);

        QuadInfo quad_info;
        quad_info.mvp = mvp;
        quad_info.max = diff;
        quad_info.min = -diff;
        quad_info.color = color;
        quad_info.depth_priority = 255.0f;
        quad_info.outlined = true;

        color[3] = 1.0f;
        quad_info.outlined_color = color;
        draw_quad(manager, quad_info, id);
    }
}

bool UvScaleManipulator::move_started() const
{
    return m_direction != Direction::None;
}

const PXR_NS::GfVec2f& UvScaleManipulator::get_delta() const
{
    return m_delta;
}

void UvScaleManipulator::set_pos(const PXR_NS::GfVec2f& pos)
{
    m_pos = pos;
}

void UvScaleManipulator::create_direction_handles()
{
    auto widget = m_tool->get_widget();
    auto manager = widget->get_draw_manager();

    const auto begin = static_cast<int>(Direction::Horizontal);
    const auto end = static_cast<int>(Direction::None);

    for (int d = begin; d < end; ++d)
    {
        const auto pair = m_direction_to_handle.emplace(static_cast<Direction>(d), manager->create_selection_id());
        const auto it = pair.first;
        m_handle_to_direction.emplace(it->second, it->first);
    }
}

std::unordered_map<UvScaleManipulator::Direction, PXR_NS::GfVec4f> UvScaleManipulator::get_colors(const uint32_t hover_id) const
{
    std::unordered_map<UvScaleManipulator::Direction, PXR_NS::GfVec4f> result;

    const auto find = m_handle_to_direction.find(hover_id);

    const auto move = m_direction != Direction::None;
    const auto hover = find == m_handle_to_direction.end() ? false : find->second != Direction::None;

    result[Direction::Horizontal] = s_x_color;
    result[Direction::Vertical] = s_y_color;
    result[Direction::Free] = s_free_color;

    if (move)
    {
        const auto free = m_direction == Direction::Free;
        result[m_direction] = free ? s_free_select_color : s_axis_select_color;
        if (free)
        {
            result[Direction::Horizontal] = s_axis_select_color;
            result[Direction::Vertical] = s_axis_select_color;
        }
    }
    else if (hover)
    {
        result[find->second] = find->second == Direction::Free ? s_free_hover_color : s_axis_hover_color;
    }

    return result;
}

PXR_NS::GfVec2f UvScaleManipulator::screen_to_clip(const int x, const int y) const
{
    const auto widget = m_tool->get_widget();
    const auto w = widget->width();
    const auto h = widget->height();

    return OPENDCC_NAMESPACE::screen_to_clip(x, y, w, h);
}

PXR_NS::GfVec2f UvScaleManipulator::screen_to_clip(PXR_NS::GfVec2f pos) const
{
    const auto widget = m_tool->get_widget();
    const auto w = widget->width();
    const auto h = widget->height();

    return OPENDCC_NAMESPACE::screen_to_clip(pos, w, h);
}

OPENDCC_NAMESPACE_CLOSE
