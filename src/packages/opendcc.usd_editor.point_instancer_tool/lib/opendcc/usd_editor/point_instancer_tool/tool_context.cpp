// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

// workaround
#include "opendcc/base/qt_python.h"

#include <QMouseEvent>
#include "opendcc/app/core/application.h"
#include "opendcc/app/ui/application_ui.h"
#include <pxr/pxr.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/transform.h>
#include <pxr/base/gf/camera.h>
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/pointInstancer.h"

#include <pxr/imaging/cameraUtil/conformWindow.h>
#include "opendcc/app/viewport/viewport_ui_draw_manager.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/app/viewport/viewport_camera_controller.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"
#include "opendcc/app/core/undo/stack.h"

#include "opendcc/app/viewport/viewport_view.h"
#include "opendcc/usd_editor/point_instancer_tool/tool_context.h"
#include <ImathMatrix.h>
#include <ImathQuat.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    static std::uniform_real_distribution<float> float_distribution(0, 1);
    float falloff_function(float falloff, float normalize_radius)
    {
        if (falloff < 0.05)
            return 1.0;
        else if (falloff > 0.51)
            return std::exp(-(falloff - 0.5f) * 10 * normalize_radius);
        else if (falloff < 0.49)
            return 1 - std::exp((falloff - 0.5f) * 30 * (1 - normalize_radius));
        else
            return 1 - normalize_radius;
    };
}

void PointInstancerToolContext::update_context()
{
    auto stage = Application::instance().get_session()->get_current_stage();
    if (!stage)
        return;

    m_instancer = UsdGeomPointInstancer();

    const SelectionList& selection_list = Application::instance().get_selection();
    if (selection_list.empty())
        return;

    else if (selection_list.fully_selected_paths_size() < 1)
    {
        OPENDCC_WARN(i18n("tool_settings.PointInstancer", "No Selected PointInstancer").toStdString());
        return;
    }

    for (auto it : selection_list)
    {
        SdfPath prim_path = it.first;
        UsdPrim prim = stage->GetPrimAtPath(prim_path);

        if (prim && prim.IsA<UsdGeomPointInstancer>())
        {
            m_instancer = UsdGeomPointInstancer(prim);
        }
    }
    m_properties.current_proto_idx = 0;
    m_on_instancer_changed();
    m_generated_uv = generate_uv();
}

Imath::V3f PointInstancerToolContext::main_direction() const
{
    return m_n;
}

Imath::V3f PointInstancerToolContext::compute_scale()
{
    Imath::V3f result;
    for (size_t i = 0; i < 3; ++i)
        result[i] = m_properties.scale[i] + m_properties.scale_randomness[i] * (float_distribution(m_rand_engine) - 0.5) * 0.01f;

    return result;
}

Imath::V3f PointInstancerToolContext::compute_point(const Imath::V3f& direction)
{
    return m_p + direction * m_properties.vertical_offset;
}

GfQuath PointInstancerToolContext::compute_quat(const Imath::V3f& direction)
{
    GfQuath bend_quat = GfQuath::GetIdentity();
    if (std::abs(m_properties.bend_randomness) > 1e-3)
    {
        Imath::V3f e = Imath::V3f(1, 0, 0);

        if (std::abs(e ^ direction) > 0.8f)
            e = Imath::V3f(0, 1, 0);

        const Imath::V3f x_axis = e.cross(direction).normalized();
        const Imath::V3f y_axis = direction.cross(x_axis).normalized();

        float bend_angle_0 = 2 * m_properties.bend_randomness * (float_distribution(m_rand_engine) - 0.5f);
        float bend_angle_1 = 2 * m_properties.bend_randomness * (float_distribution(m_rand_engine) - 0.5f);

        Imath::Quatf quat_bend_0;
        quat_bend_0.setAxisAngle(x_axis, bend_angle_0 / 180 * M_PI);
        Imath::Quatf quat_bend_1;
        quat_bend_1.setAxisAngle(y_axis, bend_angle_1 / 180 * M_PI);

        Imath::Quatf q = quat_bend_0 * quat_bend_1;
        bend_quat = GfQuath(q.r, q.v.x, q.v.y, q.v.z);
    }

    GfVec3f direction_gf(direction.x, direction.y, direction.z);
    double angle = m_properties.rotation_min + (m_properties.rotation_max - m_properties.rotation_min) * float_distribution(m_rand_engine);
    GfRotation rotation(direction_gf, angle);
    auto quatd = rotation.GetQuat();

    auto result_quat = GfQuath(quatd.GetReal(), quatd.GetImaginary()[0], quatd.GetImaginary()[1], quatd.GetImaginary()[2]) * bend_quat;

    if (m_properties.rotate_to_normal)
    {
        GfRotation rotation_to_normal(GfVec3f(0, 1, 0), direction_gf);
        GfQuatd quat_d = rotation_to_normal.GetQuat();
        GfQuath rotate_to_normal_quath = GfQuath(quat_d.GetReal(), quat_d.GetImaginary()[0], quat_d.GetImaginary()[1], quat_d.GetImaginary()[2]);
        result_quat = rotate_to_normal_quath * result_quat;
    }
    return result_quat;
}

std::vector<Imath::V2f> PointInstancerToolContext::generate_uv()
{
    std::vector<Imath::V2f> result;
    size_t num_points = size_t(4.0f * m_properties.radius * m_properties.radius * m_properties.density) + 1;
    if (m_properties.mode == Mode::Single)
    {
        return { Imath::V2f(0, 0) };
    }
    for (size_t i = 0; i < num_points; ++i)
    {
        float u = 2 * (float_distribution(m_rand_engine) - 0.5f);
        float v = 2 * (float_distribution(m_rand_engine) - 0.5f);
        if (u * u + v * v > 1)
            continue;

        if (m_properties.falloff > 0.01f)
        {
            float falloff_value = falloff_function(m_properties.falloff, std::sqrt(u * u + v * v));
            if (float_distribution(m_rand_engine) > falloff_value)
                continue;
        }
        result.push_back(Imath::V2f(u, v));
    }
    return result;
}

void PointInstancerToolContext::generate(PXR_NS::VtVec3fArray& new_points, PXR_NS::VtQuathArray& new_orientations, PXR_NS::VtVec3fArray& new_scales)
{
    new_points.clear();
    new_orientations.clear();
    new_scales.clear();
    std::vector<Imath::V3f> generated_points_normals;

    Imath::V3f direction = main_direction();

    if (m_properties.mode == Mode::Single)
    {
        auto p = compute_point(direction);
        new_points.push_back(GfVec3f(p.x, p.y, p.z));
    }
    else
    {
        VtVec3fArray generated_points;
        Imath::V3f e = Imath::V3f(1, 0, 0);

        if (std::abs(e ^ direction) > 0.8f)
            e = Imath::V3f(0, 1, 0);

        const Imath::V3f x_axis = e.cross(direction).normalized();
        const Imath::V3f y_axis = direction.cross(x_axis).normalized();

        for (auto uv : m_generated_uv)
        {
            auto p = uv.x * x_axis * m_properties.radius + uv.y * y_axis * m_properties.radius + m_p + direction * m_properties.vertical_offset;
            generated_points.push_back(GfVec3f(p.x, p.y, p.z));
        }

        if (m_bvh)
        {
            float r2 = m_properties.radius * m_properties.radius;
            for (size_t i = 0; i < generated_points.size(); ++i)
            {
                GfVec3f dir(-direction.x, -direction.y, -direction.z);
                GfVec3f origin(generated_points[i] - 10 * m_properties.radius * dir);

                GfVec3f hit_point, hit_normal;
                bool is_bvh_intersect = m_bvh->cast_ray(origin, dir, hit_point, hit_normal);
                if (is_bvh_intersect && ((hit_point - GfVec3f(m_p.x, m_p.y, m_p.z)).GetLengthSq() < r2))
                {
                    new_points.push_back(hit_point);
                    auto n = Imath::V3f(hit_normal[0], hit_normal[1], hit_normal[2]);
                    if (n.dot(m_n) < 0)
                        n = -n;
                    generated_points_normals.push_back(n);
                }
            }
        }
        else
        {
            new_points = generated_points;
        }
    }

    for (size_t i = 0; i < new_points.size(); ++i)
    {
        Imath::V3f s = compute_scale();
        new_scales.push_back(GfVec3f(s.x, s.y, s.z));
        if (generated_points_normals.size() > 0)
            new_orientations.push_back(compute_quat(generated_points_normals[i]));
        else
            new_orientations.push_back(compute_quat(direction));
    }
}

Imath::M44f get_vp_matrix(ViewportGLWidget* viewport)
{
    GfCamera camera = viewport->get_camera();
    GfFrustum frustum = camera.GetFrustum();
    GfVec4d viewport_resolution(0, 0, viewport->width(), viewport->height());
    CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit,
                            viewport_resolution[3] != 0.0 ? viewport_resolution[2] / viewport_resolution[3] : 1.0);

    GfMatrix4d m = frustum.ComputeViewMatrix() * frustum.ComputeProjectionMatrix();

    return Imath::M44f((float)m[0][0], (float)m[0][1], (float)m[0][2], (float)m[0][3], (float)m[1][0], (float)m[1][1], (float)m[1][2], (float)m[1][3],
                       (float)m[2][0], (float)m[2][1], (float)m[2][2], (float)m[2][3], (float)m[3][0], (float)m[3][1], (float)m[3][2],
                       (float)m[3][3]);
}

PointInstancerToolContext::PointInstancerToolContext()
{
    m_selection_event_hndl = Application::instance().register_event_callback(Application::EventType::SELECTION_CHANGED, [this] { update_context(); });
    m_properties.read_from_settings(settings_prefix());
    m_properties.current_proto_idx = 0;
    update_context();

    m_cursor = new QCursor(QPixmap(":/icons/cursor_crosshair"), -10, -10);
}

PointInstancerToolContext::~PointInstancerToolContext()
{
    Application::instance().unregister_event_callback(Application::EventType::SELECTION_CHANGED, m_selection_event_hndl);
    delete m_cursor;
}

bool PointInstancerToolContext::on_key_press(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    is_b_key_pressed = (key_event->key() == Qt::Key_B);
    return is_b_key_pressed;
}

bool PointInstancerToolContext::on_key_release(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    is_b_key_pressed = false;
    return (key_event->key() == Qt::Key_B);
}

void PointInstancerToolContext::draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    if (!viewport_view || !m_is_intersect)
        return;

    float up_shift = 0.03f;
    float R = m_properties.radius;
    int N = points_in_unit_radius * std::ceil(R);

    Imath::V3f p = m_p;
    Imath::V3f n = m_n;

    Imath::V3f e = Imath::V3f(1, 0, 0);

    if (std::abs(e ^ n) > 0.8f)
        e = Imath::V3f(0, 1, 0);

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
    Imath::M44f m = get_vp_matrix(viewport);
    GfMatrix4f mf = { m[0][0], m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3],
                      m[2][0], m[2][1], m[2][2], m[2][3], m[3][0], m[3][1], m[3][2], m[3][3] };

    draw_manager->begin_drawable();
    draw_manager->set_mvp_matrix(mf);
    draw_manager->mesh(ViewportUiDrawManager::PrimitiveTypeLinesStrip, points);
    draw_manager->end_drawable();

    draw_manager->begin_drawable();
    draw_manager->set_mvp_matrix(mf);
    draw_manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeLines);
    float half_R = R / 2;
    draw_manager->line(GfVec3f(p.x, p.y, p.z), GfVec3f(p.x + n.x * half_R, p.y + n.y * half_R, p.z + n.z * half_R));
    draw_manager->end_drawable();

    std::vector<GfVec3f> instances_positions;
    for (auto uv : m_generated_uv)
    {
        Imath::V3f pp = p + R * x_axis * uv.x + R * y_axis * uv.y;
        instances_positions.push_back(GfVec3f(pp.x, pp.y, pp.z));
    }
    draw_manager->begin_drawable();
    draw_manager->set_mvp_matrix(mf);
    draw_manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypePoints);
    draw_manager->set_point_size(8);
    draw_manager->set_color(GfVec4f(0.2, 0.8, 1.0, 0.5f));
    draw_manager->mesh(ViewportUiDrawManager::PrimitiveTypePoints, instances_positions);
    draw_manager->end_drawable();
}

PXR_NS::TfToken PointInstancerToolContext::get_name() const
{
    return TfToken("PointInstancer");
}

void PointInstancerToolContext::add_selected_items()
{
    if (!m_instancer)
        return;

    const SelectionList& selection_list = Application::instance().get_selection();
    if (selection_list.empty())
        return;

    for (auto it : selection_list)
    {
        if (it.first == m_instancer.GetPath())
            continue;
        SdfPathVector targets;
        m_instancer.GetPrototypesRel().GetTargets(&targets);
        bool is_exist = false;
        for (auto& target : targets)
            if (it.first == target)
            {
                is_exist = true;
                break;
            }
        if (!is_exist)
            m_instancer.GetPrototypesRel().AddTarget(it.first);
    }
}

void PointInstancerToolContext::set_properties(const Properties& properties)
{
    m_properties = properties;
    m_properties.write_to_settings(settings_prefix());
    m_generated_uv = generate_uv();
}

bool PointInstancerToolContext::on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                               ViewportUiDrawManager* draw_manager)
{
    if (!m_instancer)
    {
        OPENDCC_WARN("No Selected PointInstancer");
        return false;
    }

    if (is_b_key_pressed)
    {
        m_start_radius = m_properties.radius;
        m_start_x = mouse_event.x();
        is_adjust_radius = true;
        return true;
    }

    on_mouse_move(mouse_event, viewport_view, draw_manager);

    if (!m_is_intersect)
        return false;

    VtVec3fArray new_points;
    VtQuathArray new_orientations;
    VtVec3fArray new_scales;
    generate(new_points, new_orientations, new_scales);
    if (new_points.size() > 0)
    {
        commands::UsdEditsUndoBlock block;

        VtVec3fArray points;
        m_instancer.GetPositionsAttr().Get(&points);

        VtIntArray indices;
        m_instancer.GetProtoIndicesAttr().Get(&indices);
        VtQuathArray orientations;
        m_instancer.GetOrientationsAttr().Get(&orientations);
        VtVec3fArray scales;
        m_instancer.GetScalesAttr().Get(&scales);

        for (size_t i = 0; i < new_points.size(); ++i)
        {
            points.push_back(new_points[i]);
            indices.push_back(m_properties.current_proto_idx);
            orientations.push_back(new_orientations[i]);
            scales.push_back(new_scales[i]);
        }

        m_instancer.GetPositionsAttr().Set(points);
        m_instancer.GetProtoIndicesAttr().Set(indices);
        m_instancer.GetOrientationsAttr().Set(orientations);
        m_instancer.GetScalesAttr().Set(scales);

        for (const auto& viewport : ViewportWidget::get_live_widgets())
            viewport->get_gl_widget()->get_engine()->set_selected(Application::instance().get_selection(),
                                                                  Application::instance().get_rich_selection());
    }

    return true;
}

bool PointInstancerToolContext::on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                              ViewportUiDrawManager* draw_manager)
{
    if (!m_instancer)
    {
        m_is_intersect = false;
        return false;
    }

    if (is_adjust_radius)
    {
        auto x = mouse_event.x();
        float distance = x - m_start_x;
        if (distance >= 0.0f)
        {
            m_properties.radius = m_start_radius + distance / points_in_unit_radius;
        }
        else
        {
            float mult = (points_in_unit_radius - std::min(float(points_in_unit_radius), std::abs(distance))) / points_in_unit_radius;
            m_properties.radius = m_start_radius * mult;
        }
        m_properties.radius = std::max(m_properties.radius, 0.1f);
        draw(viewport_view, draw_manager);
        Application::instance().get_settings()->set(settings_prefix() + ".radius", m_properties.radius);
        return true;
    }

    auto custom_collection =
        HdRprimCollection(HdTokens->geometry, HdReprSelector(HdReprTokens->refined, HdReprTokens->hull), SdfPath::AbsoluteRootPath());
    custom_collection.SetExcludePaths({ m_instancer.GetPath() });

    auto intersect = viewport_view->intersect(GfVec2f(mouse_event.x(), mouse_event.y()), SelectionList::SelectionFlags::FULL_SELECTION, true,
                                              &custom_collection, { HdTokens->geometry });
    m_is_intersect = intersect.second;
    if (intersect.second)
    {
        const auto& hit = intersect.first.front();
        auto usd_p = hit.worldSpaceHitPoint;
        m_p = Imath::V3f(usd_p[0], usd_p[1], usd_p[2]);
        auto usd_n = hit.worldSpaceHitNormal;
        m_n = Imath::V3f(usd_n[0], usd_n[1], usd_n[2]);
        if (m_geom_id != hit.objectId)
        {
            if (m_properties.mode == Mode::Random)
            {
                auto prim = Application::instance().get_session()->get_current_stage()->GetPrimAtPath(hit.objectId);
                if (prim && prim.IsA<UsdGeomMesh>())
                {
                    m_bvh = std::make_unique<MeshBvh>(prim);
                    m_geom_id = hit.objectId;
                }
            }
        }
    }

    return true;
}

bool PointInstancerToolContext::on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                                 ViewportUiDrawManager* draw_manager)
{
    m_generated_uv = generate_uv();
    is_adjust_radius = false;
    return true;
}

void PointInstancerToolContext::Properties::read_from_settings(const std::string& prefix)
{
    auto settings = Application::instance().get_settings();
    current_proto_idx = settings->get(prefix + ".current_proto_idx", 0);
    scale[0] = settings->get(prefix + ".scale_x", 1.0f);
    scale[1] = settings->get(prefix + ".scale_y", 1.0f);
    scale[2] = settings->get(prefix + ".scale_z", 1.0f);
    scale_randomness[0] = settings->get(prefix + ".scale_randomness_x", 0.0f);
    scale_randomness[1] = settings->get(prefix + ".scale_randomness_y", 0.0f);
    scale_randomness[2] = settings->get(prefix + ".scale_randomness_z", 0.0f);
    vertical_offset = settings->get(prefix + ".vertical_offset", 0.0f);
    bend_randomness = settings->get(prefix + ".bend_randomness", 0.0f);
    rotation_min = settings->get(prefix + ".rotation_min", 0.0f);
    rotation_max = settings->get(prefix + ".rotation_max", 0.0f);
    density = settings->get(prefix + ".density", 1.0f);
    radius = settings->get(prefix + ".radius", 1.0f);
    falloff = settings->get(prefix + ".falloff", 0.3f);
    rotate_to_normal = settings->get(prefix + ".rotate_to_normal", false);
    mode = settings->get(prefix + ".mode", Mode::Random);
}

void PointInstancerToolContext::Properties::write_to_settings(const std::string& prefix) const
{
    auto settings = Application::instance().get_settings();
    settings->set(prefix + ".current_proto_idx", current_proto_idx);
    settings->set(prefix + ".scale_x", scale[0]);
    settings->set(prefix + ".scale_y", scale[1]);
    settings->set(prefix + ".scale_z", scale[2]);
    settings->set(prefix + ".scale_randomness_x", scale_randomness[0]);
    settings->set(prefix + ".scale_randomness_y", scale_randomness[1]);
    settings->set(prefix + ".scale_randomness_z", scale_randomness[2]);
    settings->set(prefix + ".vertical_offset", vertical_offset);
    settings->set(prefix + ".bend_randomness", bend_randomness);
    settings->set(prefix + ".rotation_min", rotation_min);
    settings->set(prefix + ".rotation_max", rotation_max);
    settings->set(prefix + ".density", density);
    settings->set(prefix + ".radius", radius);
    settings->set(prefix + ".falloff", falloff);
    settings->set(prefix + ".rotate_to_normal", rotate_to_normal);
    settings->set(prefix + ".mode", (int)mode);
}
OPENDCC_NAMESPACE_CLOSE
