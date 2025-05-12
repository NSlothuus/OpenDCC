// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

// workaround
#include "opendcc/base/qt_python.h"

#include "opendcc/app/viewport/def_cam_settings.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/settings.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE;

static const std::string SETTINGS_CHANGED = "settings_changed";

float DefCamSettings::get_fov() const
{
    return m_persp_camera.GetFieldOfView(GfCamera::FOVHorizontal);
}

void DefCamSettings::set_fov(double fov)
{
    m_persp_camera.SetPerspectiveFromAspectRatioAndFieldOfView(get_aspect_ratio(), fov, GfCamera::FOVHorizontal,
                                                               m_persp_camera.GetHorizontalAperture());
    m_event_dispatcher.dispatch(SETTINGS_CHANGED, m_is_perspective ? m_persp_camera : m_ortho_camera);
}

float DefCamSettings::get_focal_length() const
{
    return m_persp_camera.GetFocalLength();
}

void DefCamSettings::set_focal_length(double focal_length)
{
    m_persp_camera.SetFocalLength(focal_length);
    m_event_dispatcher.dispatch(SETTINGS_CHANGED, m_is_perspective ? m_persp_camera : m_ortho_camera);
}

float DefCamSettings::get_near_clip_plane() const
{
    return m_persp_camera.GetClippingRange().GetMin();
}

void DefCamSettings::set_near_clip_plane(double near_clip_plane)
{
    m_persp_camera.SetClippingRange(GfRange1f(near_clip_plane, get_far_clip_plane()));
    m_ortho_camera.SetClippingRange(m_persp_camera.GetClippingRange());
    m_event_dispatcher.dispatch(SETTINGS_CHANGED, m_is_perspective ? m_persp_camera : m_ortho_camera);
}

float DefCamSettings::get_far_clip_plane() const
{
    return m_persp_camera.GetClippingRange().GetMax();
}

void DefCamSettings::set_far_clip_plane(double far_clip_plane)
{
    m_persp_camera.SetClippingRange(GfRange1f(get_near_clip_plane(), far_clip_plane));
    m_ortho_camera.SetClippingRange(m_persp_camera.GetClippingRange());
    m_event_dispatcher.dispatch(SETTINGS_CHANGED, m_is_perspective ? m_persp_camera : m_ortho_camera);
}

float DefCamSettings::get_vertical_aperture() const
{
    return m_persp_camera.GetVerticalAperture();
}

void DefCamSettings::set_vertical_aperture(float vertical_aperture)
{
    m_persp_camera.SetVerticalAperture(vertical_aperture);
    m_event_dispatcher.dispatch(SETTINGS_CHANGED, m_is_perspective ? m_persp_camera : m_ortho_camera);
}

float DefCamSettings::get_horizontal_aperture() const
{
    return m_persp_camera.GetHorizontalAperture();
}

void DefCamSettings::set_horizontal_aperture(float horizontal_aperture)
{
    m_persp_camera.SetHorizontalAperture(horizontal_aperture);
    m_event_dispatcher.dispatch(SETTINGS_CHANGED, m_is_perspective ? m_persp_camera : m_ortho_camera);
}

float DefCamSettings::get_aspect_ratio() const
{
    return m_persp_camera.GetAspectRatio();
}

void DefCamSettings::set_aspect_ratio(float aspect_ratio)
{
    set_horizontal_aperture(get_vertical_aperture() * aspect_ratio);
}

bool DefCamSettings::is_perspective() const
{
    return m_is_perspective;
}

void DefCamSettings::set_perspective(bool is_perspective)
{
    if (m_is_perspective == is_perspective)
        return;

    m_is_perspective = is_perspective;
    m_event_dispatcher.dispatch(SETTINGS_CHANGED, m_is_perspective ? m_persp_camera : m_ortho_camera);
}

void DefCamSettings::set_orthographic_size(float orthographic_size)
{
    m_ortho_camera.SetOrthographicFromAspectRatioAndSize(m_ortho_camera.GetAspectRatio(), orthographic_size, GfCamera::FOVHorizontal);
    m_event_dispatcher.dispatch(SETTINGS_CHANGED, m_is_perspective ? m_persp_camera : m_ortho_camera);
}

float DefCamSettings::get_orthographic_size() const
{
    return m_ortho_camera.GetHorizontalAperture() * GfCamera::APERTURE_UNIT;
}

DefCamSettings& DefCamSettings::instance()
{
    static DefCamSettings settings;
    return settings;
}

void DefCamSettings::save_settings() const
{
    auto settings = Application::instance().get_settings();
    settings->set("def_cam.fov", get_fov());
    settings->set("def_cam.near_clip_plane", get_near_clip_plane());
    settings->set("def_cam.far_clip_plane", get_far_clip_plane());
    settings->set("def_cam.focal_length", get_focal_length());
    settings->set("def_cam.vertical_aperture", get_vertical_aperture());
    settings->set("def_cam.horizontal_aperture", get_horizontal_aperture());
    settings->set("def_cam.aspect_ratio", get_aspect_ratio());
    settings->set("def_cam.is_perspective", is_perspective());
    settings->set("def_cam.orthographic_size", get_orthographic_size());
}

DefCamSettingsDispatcherHandle DefCamSettings::register_event_callback(const std::function<void(const GfCamera&)> callback)
{
    return m_event_dispatcher.appendListener(SETTINGS_CHANGED, callback);
}

void DefCamSettings::unregister_event_callback(DefCamSettingsDispatcherHandle& handle)
{
    m_event_dispatcher.removeListener(SETTINGS_CHANGED, handle);
}

DefCamSettings::DefCamSettings()
{
    const auto settings = Application::instance().get_settings();
    const auto near_clip_plane = settings->get("def_cam.near_clip_plane", 1.f);
    const auto far_clip_plane = settings->get("def_cam.far_clip_plane", 1000000.f);
    const auto focal_length = settings->get("def_cam.focal_length", 50.0f);
    const auto vertical_aperture = settings->get("def_cam.vertical_aperture", GfCamera::DEFAULT_VERTICAL_APERTURE);
    const auto horizontal_aperture = settings->get("def_cam.horizontal_aperture", GfCamera::DEFAULT_HORIZONTAL_APERTURE);
    m_is_perspective = settings->get("def_cam.is_perspective", true);
    const auto orthographic_size = settings->get("def_cam.orthographic_size", GfCamera::DEFAULT_HORIZONTAL_APERTURE * GfCamera::APERTURE_UNIT);

    m_persp_camera = GfCamera(GfMatrix4d(1.0), GfCamera::Projection::Perspective, horizontal_aperture, vertical_aperture, 0.0f, 0.0f, focal_length,
                              GfRange1f(near_clip_plane, far_clip_plane));

    m_ortho_camera = GfCamera(GfMatrix4d(1.0), GfCamera::Orthographic);
    m_ortho_camera.SetClippingRange(m_persp_camera.GetClippingRange());
    m_ortho_camera.SetOrthographicFromAspectRatioAndSize(m_persp_camera.GetAspectRatio(), orthographic_size, GfCamera::FOVHorizontal);
}

OPENDCC_NAMESPACE_CLOSE
