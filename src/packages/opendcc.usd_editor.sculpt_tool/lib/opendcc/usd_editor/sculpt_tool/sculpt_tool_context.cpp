// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

// workaround
#include "opendcc/base/qt_python.h"

#include <QMouseEvent>

#include <algorithm>
#include <random>
#include <algorithm>

#include <ImathMatrix.h>
#include <ImathQuat.h>

#include <pxr/pxr.h>
#include <pxr/base/gf/transform.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>

#include "opendcc/app/viewport/viewport_camera_controller.h"
#include "opendcc/app/viewport/viewport_ui_draw_manager.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/app/viewport/viewport_view.h"

#include "opendcc/app/core/application.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"
#include "opendcc/app/ui/application_ui.h"

#include "opendcc/usd_editor/sculpt_tool/sculpt_tool_context.h"
#include "opendcc/usd_editor/sculpt_tool/sculpt_functions.h"
#include "opendcc/usd_editor/sculpt_tool/utils.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    static int points_in_unit_radius = 50;
}

static GfMatrix4f get_vp_matrix(ViewportGLWidget* viewport)
{
    const GfCamera camera = viewport->get_camera();
    GfFrustum frustum = camera.GetFrustum();
    const GfVec4d viewport_resolution(0, 0, viewport->width(), viewport->height());
    CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit,
                            viewport_resolution[3] != 0.0 ? viewport_resolution[2] / viewport_resolution[3] : 1.0);

    const GfMatrix4d m = frustum.ComputeViewMatrix() * frustum.ComputeProjectionMatrix();

    return GfMatrix4f((float)m[0][0], (float)m[0][1], (float)m[0][2], (float)m[0][3], (float)m[1][0], (float)m[1][1], (float)m[1][2], (float)m[1][3],
                      (float)m[2][0], (float)m[2][1], (float)m[2][2], (float)m[2][3], (float)m[3][0], (float)m[3][1], (float)m[3][2], (float)m[3][3]);
}

// SculptToolContext

SculptToolContext::SculptToolContext()
{
    auto settings = Application::instance().get_settings();

    update_sculpt_strategy(static_cast<Mode>(settings->get(settings_prefix() + ".last_mode", 0)));
    update_context();

    m_cursor = new QCursor(QPixmap(":/icons/cursor_crosshair"), -10, -10);

    m_selection_event_hndl = Application::instance().register_event_callback(Application::EventType::SELECTION_CHANGED, [this] { update_context(); });
    m_current_stage_changed_event_hndl = Application::instance().register_event_callback(Application::EventType::CURRENT_STAGE_CHANGED, [this] {
        TfNotice::Revoke(m_objects_changed_notice_key);
        m_objects_changed_notice_key = TfNotice::Register(TfCreateWeakPtr(this), &SculptToolContext::on_objects_changed,
                                                          Application::instance().get_session()->get_current_stage());
        update_context();
    });
    m_objects_changed_notice_key =
        TfNotice::Register(TfCreateWeakPtr(this), &SculptToolContext::on_objects_changed, Application::instance().get_session()->get_current_stage());
}

SculptToolContext::~SculptToolContext()
{
    Application::instance().unregister_event_callback(Application::EventType::SELECTION_CHANGED, m_selection_event_hndl);
    Application::instance().unregister_event_callback(Application::EventType::CURRENT_STAGE_CHANGED, m_current_stage_changed_event_hndl);
    TfNotice::Revoke(m_objects_changed_notice_key);

    delete m_cursor;
}

bool SculptToolContext::on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                       ViewportUiDrawManager* draw_manager)
{
    m_ignore_stage_changing = true;
    if (!m_mesh_data)
    {
        OPENDCC_WARN("No mesh selected");
        return false;
    }

    if (m_is_b_key_pressed)
    {
        m_start_radius = m_sculpt_strategy->get_properties().radius;
        m_start_x = mouse_event.x();
        m_is_adjust_radius = true;
        return true;
    }

    if (!m_sculpt_strategy->on_mouse_press(mouse_event, viewport_view, draw_manager))
    {
        return false;
    }

    return m_sculpt_strategy->is_intersect();
}

bool SculptToolContext::on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                      ViewportUiDrawManager* draw_manager)
{
    if (!m_mesh_data)
    {
        return false;
    }

    if (m_is_adjust_radius)
    {
        m_current_x = mouse_event.x();
        adjust_radius();
        draw(viewport_view, draw_manager);
        return true;
    }

    return m_sculpt_strategy->on_mouse_move(mouse_event, viewport_view, draw_manager);
}

bool SculptToolContext::on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                         ViewportUiDrawManager* draw_manager)
{
    m_is_adjust_radius = false;
    if (m_mesh_data)
    {
        m_mesh_data->on_finish();
        bool ok = m_mesh_data->update_geometry();
        if (!ok)
            m_mesh_data.reset();
    }
    m_ignore_stage_changing = false;

    return m_sculpt_strategy->on_mouse_release(mouse_event, viewport_view, draw_manager);
}

bool SculptToolContext::on_key_press(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    m_is_b_key_pressed = (key_event->key() == Qt::Key_B);
    return m_is_b_key_pressed;
}

bool SculptToolContext::on_key_release(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    m_is_b_key_pressed = false;
    if (m_mesh_data)
        m_mesh_data->on_finish();
    return (key_event->key() == Qt::Key_B);
}

QCursor* SculptToolContext::get_cursor()
{
    return m_cursor;
}

void SculptToolContext::draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    if (!viewport_view || !m_sculpt_strategy->is_intersect())
        return;

    const float up_shift = 0.03f;
    const float R = m_sculpt_strategy->get_properties().radius;
    const int N = points_in_unit_radius * std::ceil(R);

    const auto& hit_point = m_sculpt_strategy->get_draw_point();
    const auto& hit_normal = m_sculpt_strategy->get_draw_normal();

    const Imath::V3f p(hit_point[0], hit_point[1], hit_point[2]);
    const Imath::V3f n(hit_normal[0], hit_normal[1], hit_normal[2]);

    Imath::V3f e = Imath::V3f(1, 0, 0);
    e = std::abs(e ^ n) > 0.8f ? Imath::V3f(0, 1, 0) : e;

    const Imath::V3f x_axis = e.cross(n).normalized();
    const Imath::V3f y_axis = n.cross(x_axis).normalized();

    std::vector<GfVec3f> points(N + 1);
    for (int i = 0; i <= N; ++i)
    {
        Imath::V3f pp = p + R * x_axis * cos((2 * M_PI * i) / N) + R * y_axis * sin((2 * M_PI * i) / N) + up_shift * n;
        points[i] = GfVec3f(pp.x, pp.y, pp.z);
    }

    auto active_view = ApplicationUI::instance().get_active_view();
    if (!active_view)
        return;

    auto viewport = active_view->get_gl_widget();
    GfMatrix4f mf = get_vp_matrix(viewport);

    draw_manager->begin_drawable();
    draw_manager->set_mvp_matrix(mf);
    draw_manager->mesh(ViewportUiDrawManager::PrimitiveTypeLinesStrip, points);
    draw_manager->end_drawable();

    draw_manager->begin_drawable();
    draw_manager->set_mvp_matrix(mf);
    draw_manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeLines);
    const float half_R = R / 2;
    draw_manager->line(GfVec3f(p.x, p.y, p.z), GfVec3f(p.x + n.x * half_R, p.y + n.y * half_R, p.z + n.z * half_R));
    draw_manager->end_drawable();
}

TfToken SculptToolContext::get_name() const
{
    return TfToken("Sculpt");
}

void SculptToolContext::set_on_mesh_changed_callback(std::function<void()> on_mesh_changed)
{
    m_on_mesh_changed = on_mesh_changed;
}

Properties SculptToolContext::properties() const
{
    return m_sculpt_strategy->get_properties();
}

void SculptToolContext::set_properties(const Properties& properties)
{
    update_sculpt_strategy(properties.mode);
    m_sculpt_strategy->set_properties(properties);
    properties.write_to_settings(SculptToolContext::settings_prefix());
}

std::string SculptToolContext::settings_prefix()
{
    return "sculpt_tool_context.properties";
}

bool SculptToolContext::empty() const
{
    return m_mesh_data == nullptr;
}

void SculptToolContext::update_sculpt_strategy(const Mode& mode)
{
    m_sculpt_strategy = mode == Mode::Move ? std::unique_ptr<SculptStrategy>(new MoveStrategy) : std::unique_ptr<SculptStrategy>(new DefaultStrategy);

    m_sculpt_strategy->set_mesh_data(m_mesh_data);
}

void SculptToolContext::adjust_radius()
{
    const float distance = m_current_x - m_start_x;

    Properties properties = m_sculpt_strategy->get_properties();

    properties.radius = distance >= 0.0f ? m_start_radius + distance / points_in_unit_radius
                                         : m_start_radius * (points_in_unit_radius - std::min(float(points_in_unit_radius), std::abs(distance))) /
                                               points_in_unit_radius;

    properties.radius = std::max(properties.radius, 0.1f);

    Application::instance().get_settings()->set(settings_prefix() + ".radius.current", properties.radius);

    m_sculpt_strategy->set_properties(properties);
}

void SculptToolContext::update_context()
{
    auto stage = Application::instance().get_session()->get_current_stage();
    if (!stage)
        return;

    m_mesh_data.reset();

    const SelectionList& selection_list = Application::instance().get_selection();
    if (selection_list.empty())
    {
        m_on_mesh_changed();
        return;
    }

    else if (selection_list.fully_selected_paths_size() > 1)
    {
        OPENDCC_WARN("Multiple Selection");
        m_on_mesh_changed();
        return;
    }

    SdfPath prim_path = selection_list.begin()->first;
    UsdPrim prim = stage->GetPrimAtPath(prim_path);

    if (prim && prim.IsA<UsdGeomMesh>())
    {
        bool ok;
        m_mesh_data = std::make_shared<UndoMeshManipulationData>(UsdGeomMesh(prim), ok);
        if (!ok)
            m_mesh_data.reset();

        m_sculpt_strategy->set_mesh_data(m_mesh_data);
    }
    m_on_mesh_changed();
}

void SculptToolContext::on_objects_changed(UsdNotice::ObjectsChanged const& notice, UsdStageWeakPtr const& sender)
{
    if (m_ignore_stage_changing)
        return;
    if (m_mesh_data)
    {
        auto mesh_path = m_mesh_data->mesh.GetPrim().GetPath();

        using PathRange = UsdNotice::ObjectsChanged::PathRange;
        const PathRange paths_to_resync = notice.GetResyncedPaths();
        const PathRange paths_to_update = notice.GetChangedInfoOnlyPaths();

        for (PathRange::const_iterator path = paths_to_resync.begin(); path != paths_to_resync.end(); ++path)
        {
            if (mesh_path.HasPrefix(*path))
            {
                update_context();
                return;
            }
        }
        for (PathRange::const_iterator path = paths_to_update.begin(); path != paths_to_update.end(); ++path)
        {
            if (mesh_path.HasPrefix(path->GetPrimPath()))
            {
                update_context();
                return;
            }
        }
    }
    else
        update_context(); // to handle case, when prim creating while sculpt tool is active
}

OPENDCC_NAMESPACE_CLOSE
