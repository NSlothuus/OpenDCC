// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/uv_editor/uv_rotate_manipulator.h"
#include "opendcc/usd_editor/uv_editor/uv_editor_gl_widget.h"
#include "opendcc/usd_editor/uv_editor/utils.h"

#include "opendcc/app/viewport/viewport_camera_controller.h"

#include <pxr/imaging/cameraUtil/conformWindow.h>

#include <QMouseEvent>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

static const GfVec2f origin(0.0f, 0.0f);
static const GfVec3f g_pie_color(203 / 255.0);

// clang-format off

static const GfVec4f s_free_color        (0.392f, 0.863f, 1.000f, 0.400f);

static const GfVec4f s_axis_select_color (1.000f, 1.000f, 0.000f, 1.000f);
static const GfVec4f s_free_select_color (1.000f, 1.000f, 0.000f, 0.500f);

static const GfVec4f s_axis_hover_color  (1.000f, 0.750f, 0.500f, 1.000f);
static const GfVec4f s_free_hover_color  (1.000f, 0.750f, 0.500f, 0.500f);
// clang-format on

UvRotateManipulator::UvRotateManipulator(UvTool* tool)
    : m_tool(tool)
{
    create_direction_handles();
}

UvRotateManipulator::~UvRotateManipulator() {}

void UvRotateManipulator::on_mouse_press(QMouseEvent* event)
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

    m_click = screen_to_clip(screen.x(), screen.y());
    m_click_moved_pos = m_click;
    m_is_rotate = true;
}

void UvRotateManipulator::on_mouse_move(QMouseEvent* event)
{
    if (m_direction == Direction::None)
    {
        return;
    }
    const auto screen = event->pos();

    m_click_moved_pos = screen_to_clip(screen.x(), screen.y());
}

void UvRotateManipulator::on_mouse_release(QMouseEvent* event)
{
    m_direction = Direction::None;
    m_click.Set(0.0f, 0.0f);
    m_click_moved_pos.Set(0.0f, 0.0f);
    m_is_rotate = false;
}

void UvRotateManipulator::draw(ViewportUiDrawManager* manager)
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
    auto start_vector_length = sqrt(start_vector[0] * start_vector[0] + start_vector[1] * start_vector[1]);
    start_vector[0] *= 1 / start_vector_length;
    start_vector[1] *= 1 / start_vector_length;

    auto end_vector = inverse_mvp.Transform(GfVec3f(m_click_moved_pos[0], m_click_moved_pos[1], 0.0f));
    auto end_vector_length = sqrt(end_vector[0] * end_vector[0] + end_vector[1] * end_vector[1]);
    end_vector[0] *= 1 / end_vector_length;
    end_vector[1] *= 1 / end_vector_length;

    GfVec3f view_direction = GfVec3f(frustum.ComputeViewDirection());
    GfVec3f up = GfVec3f(frustum.ComputeUpVector());
    up.Normalize();

    GfVec3f right, frnt;
    view_direction.Normalize();

    right = up ^ view_direction;
    right.Normalize();

    // circle
    {
        CircleInfo circle_info;
        circle_info.origin = origin;
        circle_info.mvp = mvp;
        circle_info.right = right;
        circle_info.up = up;
        circle_info.depth_priority = 255.0f;

        const auto color = colors.at(Direction::Free);
        const auto id = m_direction_to_handle[Direction::Free];

        circle_info.color = color;
        draw_circle(manager, circle_info, id);
    }

    if (m_is_rotate)
    {
        GfVec3f view = GfVec3f(frustum.ComputeViewDirection());
        auto v1_proj = start_vector - GfDot(start_vector, -view) * (-view);
        v1_proj.Normalize();
        auto v2_proj = end_vector - GfDot(end_vector, -view) * (-view);
        v2_proj.Normalize();

        GfRotation rotate(v1_proj, v2_proj);
        m_angle = -rotate.GetAngle();

        if (GfDot(GfCross(v2_proj, v1_proj), -view) < 0)
        {
            std::swap(v1_proj, v2_proj);
            m_angle = -m_angle;
        }

        PieInfo pie_info;
        pie_info.origin = GfVec3f(origin[0], origin[1], 0.0f);
        pie_info.mvp = mvp;
        pie_info.start = v1_proj;
        pie_info.end = v2_proj;
        pie_info.depth_priority = 2;
        pie_info.radius = 1.0f;
        pie_info.view = view;
        pie_info.angle = m_angle;
        pie_info.point_size = 8.0f;
        pie_info.color = GfVec4f(g_pie_color[0], g_pie_color[1], g_pie_color[2], 1);

        const auto id = m_direction_to_handle[Direction::Free];
        draw_pie(manager, pie_info, id);
    }
}

bool UvRotateManipulator::move_started() const
{
    return m_direction != Direction::None;
}

const double& UvRotateManipulator::get_angle() const
{
    return m_angle;
}

void UvRotateManipulator::set_pos(const PXR_NS::GfVec2f& pos)
{
    m_pos = pos;
}

void UvRotateManipulator::create_direction_handles()
{
    auto widget = m_tool->get_widget();
    auto manager = widget->get_draw_manager();

    const auto begin = static_cast<int>(Direction::Free);
    const auto end = static_cast<int>(Direction::None);

    for (int d = begin; d < end; ++d)
    {
        const auto pair = m_direction_to_handle.emplace(static_cast<Direction>(d), manager->create_selection_id());
        const auto it = pair.first;
        m_handle_to_direction.emplace(it->second, it->first);
    }
}

std::unordered_map<UvRotateManipulator::Direction, PXR_NS::GfVec4f> UvRotateManipulator::get_colors(const uint32_t hover_id) const
{
    std::unordered_map<UvRotateManipulator::Direction, PXR_NS::GfVec4f> result;

    const auto find = m_handle_to_direction.find(hover_id);

    const auto move = m_direction != Direction::None;
    const auto hover = find == m_handle_to_direction.end() ? false : find->second != Direction::None;

    result[Direction::Free] = s_free_color;

    if (move)
    {
        const auto free = m_direction == Direction::Free;
        result[m_direction] = free ? s_free_select_color : s_axis_select_color;
    }
    else if (hover)
    {
        result[find->second] = find->second == Direction::Free ? s_free_hover_color : s_axis_hover_color;
    }

    return result;
}

PXR_NS::GfVec2f UvRotateManipulator::screen_to_clip(const int x, const int y) const
{
    const auto widget = m_tool->get_widget();
    const auto w = widget->width();
    const auto h = widget->height();

    return OPENDCC_NAMESPACE::screen_to_clip(x, y, w, h);
}

PXR_NS::GfVec2f UvRotateManipulator::screen_to_clip(PXR_NS::GfVec2f pos) const
{
    const auto widget = m_tool->get_widget();
    const auto w = widget->width();
    const auto h = widget->height();

    return OPENDCC_NAMESPACE::screen_to_clip(pos, w, h);
}

OPENDCC_NAMESPACE_CLOSE
