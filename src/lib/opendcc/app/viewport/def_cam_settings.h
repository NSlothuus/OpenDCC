/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <QObject>
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"
#include <pxr/base/gf/camera.h>

OPENDCC_NAMESPACE_OPEN

typedef eventpp::EventDispatcher<std::string, void(const PXR_NS::GfCamera&)> DefCamSettingsDispatcher;
typedef eventpp::EventDispatcher<std::string, void(const PXR_NS::GfCamera&)>::Handle DefCamSettingsDispatcherHandle;

class OPENDCC_API DefCamSettings
{
public:
    float get_fov() const;
    void set_fov(double fov);
    float get_focal_length() const;
    void set_focal_length(double focal_length);
    float get_near_clip_plane() const;
    void set_near_clip_plane(double near_clip_plane);
    float get_far_clip_plane() const;
    void set_far_clip_plane(double far_clip_plane);
    float get_vertical_aperture() const;
    void set_vertical_aperture(float vertical_aperture);
    float get_horizontal_aperture() const;
    void set_horizontal_aperture(float horizontal_aperture);
    float get_aspect_ratio() const;
    void set_aspect_ratio(float aspect_ratio);
    bool is_perspective() const;
    void set_perspective(bool is_perspective);
    void set_orthographic_size(float orthographic_size);
    float get_orthographic_size() const;

    static DefCamSettings& instance();
    void save_settings() const;

    DefCamSettingsDispatcherHandle register_event_callback(const std::function<void(const PXR_NS::GfCamera&)> callback);
    void unregister_event_callback(DefCamSettingsDispatcherHandle& handle);

    DefCamSettings& operator=(const DefCamSettings&) = delete;

private:
    DefCamSettings();

    DefCamSettingsDispatcher m_event_dispatcher;

    PXR_NS::GfCamera m_persp_camera;
    PXR_NS::GfCamera m_ortho_camera;
    bool m_is_perspective = true;
};

OPENDCC_NAMESPACE_CLOSE
