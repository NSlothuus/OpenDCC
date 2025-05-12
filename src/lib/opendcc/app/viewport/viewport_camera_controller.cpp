// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

// workaround
#include "opendcc/base/qt_python.h"

#include "opendcc/app/viewport/viewport_camera_controller.h"
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/transform.h>
#include <pxr/base/gf/math.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdGeom/metrics.h>
#include "opendcc/app/viewport/def_cam_settings.h"
#include "opendcc/app/core/session.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    double get_orthographic_size(double horizontal_aperture)
    {
        return horizontal_aperture * GfCamera::APERTURE_UNIT;
    }
};

class FollowModeStrategy
{
public:
    FollowModeStrategy() = default;
    FollowModeStrategy(const FollowModeStrategy* other)
    {
        if (!other)
        {
            reset_to_default();
            return;
        }

        m_camera = other->m_camera;
        m_center = other->m_center;
        m_rot_theta = other->m_rot_theta;
        m_rot_phi = other->m_rot_phi;
        m_rot_psi = other->m_rot_psi;
        m_dist = other->m_dist;
        m_sel_size = other->m_sel_size;
        m_time = other->m_time;
        m_up_axis = other->m_up_axis;
        m_display_size = other->m_display_size;
        m_yz_up_matrix = other->m_yz_up_matrix;
        m_inv_yz_up_matrix = other->m_inv_yz_up_matrix;
    }
    virtual ~FollowModeStrategy() = default;

    virtual void set_display_size(int w, int h) { m_display_size = { w, h }; };
    bool is_perspective() const { return m_camera.GetProjection() == GfCamera::Projection::Perspective; }

    virtual void truck(double delta_right, double delta_up)
    {
        const GfFrustum frustum = get_camera().GetFrustum();
        const GfVec3d camera_up = frustum.ComputeUpVector();
        const GfVec3d camera_right = GfCross(frustum.ComputeViewDirection(), camera_up);
        m_center += (delta_right * camera_right + delta_up * camera_up);

        m_is_camera_transform_dirty = true;
    }

    virtual void pan_tilt(double delta_pan, double delta_tilt)
    {
        GfMatrix4d new_transform = GfMatrix4d(1.0).SetRotate(GfRotation(GfVec3d::XAxis(), delta_tilt)) *
                                   GfMatrix4d(1.0).SetRotate(GfRotation(GfVec3d::YAxis(), delta_pan)) * m_camera.GetTransform();
        m_camera.SetTransform(new_transform);
        pull();
        m_rot_psi = 0.0;

        m_is_camera_transform_dirty = true;
    }

    virtual void tumble(double delta_theta, double delta_phi)
    {
        m_rot_theta += delta_theta;
        m_rot_phi += delta_phi;

        m_is_camera_transform_dirty = true;
    }

    virtual void adjust_distance(double scale_factor)
    {
        if (is_perspective())
        {
            m_dist = GfClamp(m_dist * scale_factor, (double)m_camera.GetClippingRange().GetMin(), (double)m_camera.GetClippingRange().GetMax());
        }
        else
        {
            m_ortho_size =
                GfClamp(m_ortho_size * scale_factor, (double)m_camera.GetClippingRange().GetMin(), (double)m_camera.GetClippingRange().GetMax());
        }

        m_is_camera_transform_dirty = true;
    }

    double compute_pixels_to_world_factor(int viewport_height)
    {
        const auto camera = get_camera();
        if (is_perspective())
        {
            const double frustum_height = camera.GetFrustum().GetWindow().GetSize()[1];
            return frustum_height * m_dist / viewport_height;
        }
        else
        {
            return get_orthographic_size(camera.GetVerticalAperture()) / viewport_height;
        }
    }

    virtual void frame_selection(const GfBBox3d& selection_bbox, double frame_fit = 1.1)
    {
        m_center = selection_bbox.ComputeCentroid();
        const GfRange3d sel_range = selection_bbox.ComputeAlignedRange();

        m_sel_size = sel_range.GetSize().GetLength();
        if (GfIsClose(m_sel_size, 0.0, 1e-6))
            m_sel_size = 1.0f;

        const auto camera = get_camera();
        if (is_perspective())
        {
            const double half_fov = get_camera().GetFieldOfView(GfCamera::FOVVertical) * 0.5;
            m_dist = (m_sel_size * frame_fit * 0.5) / std::tan(GfDegreesToRadians(half_fov));
            m_dist = std::max(m_dist, get_camera().GetClippingRange().GetMin() * 1.5);
        }
        else
        {
            m_dist = m_sel_size + camera.GetClippingRange().GetMin();
            m_ortho_size = m_sel_size * frame_fit;
            if (m_camera.GetAspectRatio() > 1.0)
                m_ortho_size *= m_camera.GetAspectRatio();
            else
                m_ortho_size /= m_camera.GetAspectRatio();
        }
        m_is_camera_transform_dirty = true;
    }

    virtual void push()
    {
        auto transform = compute_transform();
        m_camera.SetTransform(transform);
        m_camera.SetFocusDistance(m_dist);
        if (!is_perspective())
        {
            m_camera.SetOrthographicFromAspectRatioAndSize(m_camera.GetAspectRatio(), m_ortho_size, GfCamera::FOVHorizontal);
        }
    }
    virtual void pull() = 0;
    virtual ViewportCameraController::FollowMode get_follow_mode() const = 0;
    virtual GfCamera get_camera() = 0;
    virtual void set_time(UsdTimeCode time) { m_time = time; }

    virtual double get_dist() const { return m_dist; }
    virtual UsdTimeCode get_time() const { return m_time; }
    virtual void set_up_axis(const TfToken& up_axis)
    {
        if (m_up_axis == up_axis)
            return;

        reset_to_default();
        m_up_axis = up_axis;
        if (m_up_axis == UsdGeomTokens->z)
        {
            // Rotate around X axis by -90 degrees
            m_yz_up_matrix.Set(1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1);
            m_inv_yz_up_matrix = m_yz_up_matrix.GetTranspose();
        }
        else
        {
            m_yz_up_matrix.SetIdentity();
            m_inv_yz_up_matrix.SetIdentity();
        }
        FollowModeStrategy::push();
    }

private:
    void reset_to_default()
    {
        const auto& def_cam_settings = DefCamSettings::instance();
        m_camera = GfCamera();
        m_camera.SetClippingRange(GfRange1f(def_cam_settings.get_near_clip_plane(), def_cam_settings.get_far_clip_plane()));
        m_camera.SetProjection(DefCamSettings::instance().is_perspective() ? GfCamera::Projection::Perspective : GfCamera::Projection::Orthographic);
        m_center = GfVec3d(0, 0, 0);
        m_dist = GfVec3d(12, 9, 12).GetLength();
        m_rot_phi = 45;
        m_rot_theta = 45;
        m_yz_up_matrix = GfMatrix4d(1);
        m_inv_yz_up_matrix = GfMatrix4d(1);
        m_up_axis = UsdGeomTokens->y;
        m_rot_psi = 0;
        m_sel_size = 10;
        m_is_camera_transform_dirty = false;
        m_time = UsdTimeCode();
    }

protected:
    GfMatrix4d compute_transform() const
    {
        static auto rotate_matrix = [](const GfVec3d& vec, double angle) -> GfMatrix4d {
            return GfMatrix4d(1.0).SetRotate(GfRotation(vec, angle));
        };
        auto transform = GfMatrix4d().SetTranslate(GfVec3d::ZAxis() * m_dist);
        transform *=
            rotate_matrix(GfVec3d::ZAxis(), -m_rot_psi) * rotate_matrix(GfVec3d::XAxis(), -m_rot_phi) * rotate_matrix(GfVec3d::YAxis(), -m_rot_theta);
        transform *= m_inv_yz_up_matrix;
        transform *= GfMatrix4d().SetTranslate(m_center);
        return transform;
    }

    void extract_params_from_camera()
    {
        const GfMatrix4d cam_transform = m_camera.GetTransform();
        const double dist = m_camera.GetFocusDistance();
        const GfFrustum frustum = m_camera.GetFrustum();
        const GfVec3d camera_pos = frustum.GetPosition();
        const GfVec3d camera_axis = frustum.ComputeViewDirection();

        m_dist = dist;
        m_sel_size = dist / 10.0;
        m_center = camera_pos + dist * camera_axis;
        m_ortho_size = get_orthographic_size(m_camera.GetHorizontalAperture());

        GfMatrix4d transform = cam_transform * m_yz_up_matrix;
        transform.Orthonormalize();
        GfRotation rotation = transform.ExtractRotation();

        auto decomposed_rot = -rotation.Decompose(GfVec3d::YAxis(), GfVec3d::XAxis(), GfVec3d::ZAxis());
        m_rot_theta = decomposed_rot[0];
        m_rot_phi = decomposed_rot[1];
        m_rot_psi = decomposed_rot[2];
    }
    GfCamera m_camera;
    GfMatrix4d m_yz_up_matrix = GfMatrix4d(1);
    GfMatrix4d m_inv_yz_up_matrix = GfMatrix4d(1);
    TfToken m_up_axis = UsdGeomTokens->y;
    GfVec3d m_center = GfVec3d(0, 0, 0);
    double m_rot_theta = 0;
    double m_rot_phi = 0;
    double m_rot_psi = 0;
    double m_dist = 100;
    double m_sel_size = 10;
    double m_ortho_size = 2.355;
    GfVec2i m_display_size = GfVec2i(600, 300);
    bool m_is_camera_transform_dirty = false;
    UsdTimeCode m_time;
};

class DefCamFollowStrategy : public FollowModeStrategy
{
public:
    DefCamFollowStrategy(const FollowModeStrategy* other)
        : FollowModeStrategy(other)
    {
        auto& camera_settings = DefCamSettings::instance();
        if (camera_settings.is_perspective())
            m_camera.SetPerspectiveFromAspectRatioAndFieldOfView(camera_settings.get_aspect_ratio(), camera_settings.get_fov(),
                                                                 GfCamera::FOVHorizontal, DefCamSettings::instance().get_horizontal_aperture());
        else
            m_camera.SetOrthographicFromAspectRatioAndSize(m_display_size[0] / (double)m_display_size[1],
                                                           get_orthographic_size(camera_settings.get_horizontal_aperture()), GfCamera::FOVHorizontal);

        m_camera.SetClippingRange(GfRange1f(camera_settings.get_near_clip_plane(), camera_settings.get_far_clip_plane()));
        m_def_cam_settings_dispatcher_handle = DefCamSettings::instance().register_event_callback([this](const GfCamera& camera) {
            if (camera.GetProjection() == GfCamera::Projection::Perspective)
            {
                m_camera = GfCamera(m_camera.GetTransform(), camera.GetProjection(), camera.GetHorizontalAperture(), camera.GetVerticalAperture(),
                                    camera.GetHorizontalApertureOffset(), camera.GetVerticalApertureOffset(), camera.GetFocalLength(),
                                    camera.GetClippingRange(), camera.GetClippingPlanes(), m_camera.GetFStop(), m_camera.GetFocusDistance());
            }
            else
            {
                m_camera = GfCamera(m_camera.GetTransform(), camera.GetProjection(), m_camera.GetHorizontalAperture(), m_camera.GetVerticalAperture(),
                                    m_camera.GetHorizontalApertureOffset(), m_camera.GetVerticalApertureOffset(), m_camera.GetFocalLength(),
                                    camera.GetClippingRange(), camera.GetClippingPlanes(), m_camera.GetFStop(), m_camera.GetFocusDistance());
                m_camera.SetOrthographicFromAspectRatioAndSize(m_display_size[0] / (double)m_display_size[1], m_ortho_size, GfCamera::FOVHorizontal);
            }
        });
        m_rot_psi = 0;
        FollowModeStrategy::push();
    }
    ~DefCamFollowStrategy() { DefCamSettings::instance().unregister_event_callback(m_def_cam_settings_dispatcher_handle); }

    void set_display_size(int w, int h) override
    {
        FollowModeStrategy::set_display_size(w, h);
        if (!is_perspective())
            m_camera.SetOrthographicFromAspectRatioAndSize(w / (double)h, m_ortho_size, GfCamera::FOVHorizontal);
    }
    void push() override
    {
        if (!m_is_camera_transform_dirty)
            return;
        FollowModeStrategy::push();
        m_is_camera_transform_dirty = false;
    }
    void pull() override { extract_params_from_camera(); }

    GfCamera get_camera() override
    {
        push();
        return m_camera;
    }

    ViewportCameraController::FollowMode get_follow_mode() const override { return ViewportCameraController::FollowMode::DefCam; }

private:
    DefCamSettingsDispatcherHandle m_def_cam_settings_dispatcher_handle;
};

class ReadOnlyStageCameraStrategy : public FollowModeStrategy
{
public:
    ReadOnlyStageCameraStrategy(ViewportCameraMapperPtr camera_mapper)
        : m_camera_mapper(camera_mapper)
    {
    }
    ReadOnlyStageCameraStrategy(const FollowModeStrategy* other, ViewportCameraMapperPtr camera_mapper)
        : FollowModeStrategy(other)
        , m_camera_mapper(camera_mapper)
    {
    }
    ~ReadOnlyStageCameraStrategy() = default;

    void push() override {}
    void pull() override
    {
        m_camera = m_camera_mapper->pull(m_time);
        if (m_camera.GetFocusDistance() <= 0.0)
        {
            m_camera.SetFocusDistance(m_dist);
        }
        extract_params_from_camera();
    }

    GfCamera get_camera() override
    {
        if (m_time_changed)
        {
            pull();
            m_time_changed = false;
        }
        return m_camera;
    }

    ViewportCameraController::FollowMode get_follow_mode() const override { return ViewportCameraController::FollowMode::StageCamPrimReadOnly; }

    virtual void truck(double delta_right, double delta_up) override {}
    virtual void pan_tilt(double delta_pan, double delta_tilt) override {}
    virtual void tumble(double delta_theta, double delta_phi) override {}
    virtual void adjust_distance(double scale_factor) override {}

    virtual void frame_selection(const GfBBox3d& selection_bbox, double frame_fit = 1.1) override {}
    virtual void set_time(UsdTimeCode time) override
    {
        FollowModeStrategy::set_time(time);
        m_time_changed = true;
    }

private:
    ViewportCameraMapperPtr m_camera_mapper;
    bool m_time_changed = false;
};

class StageCameraStrategy : public FollowModeStrategy
{
public:
    StageCameraStrategy(const FollowModeStrategy* other, ViewportCameraMapperPtr camera_mapper)
        : FollowModeStrategy(other)
        , m_camera_mapper(camera_mapper)
    {
    }
    ~StageCameraStrategy() = default;

    void push() override
    {
        if (!m_is_camera_transform_dirty)
            return;
        FollowModeStrategy::push();

        m_camera_mapper->push(m_camera, m_time);
        m_is_camera_transform_dirty = false;
    }

    void pull() override
    {
        m_camera = m_camera_mapper->pull(m_time);
        if (m_camera.GetFocusDistance() <= 0.0)
        {
            m_camera.SetFocusDistance(m_dist);
        }
        extract_params_from_camera();
    }

    GfCamera get_camera() override
    {
        push();
        if (m_time_changed)
        {
            pull();
            m_time_changed = false;
        }
        return m_camera;
    }

    ViewportCameraController::FollowMode get_follow_mode() const override { return ViewportCameraController::FollowMode::StageCamPrim; }
    virtual void set_time(UsdTimeCode time) override
    {
        FollowModeStrategy::set_time(time);
        m_time_changed = true;
    }

private:
    ViewportCameraMapperPtr m_camera_mapper;
    bool m_time_changed = false;
};

class StageXformableStrategy : public DefCamFollowStrategy
{
public:
    StageXformableStrategy(const FollowModeStrategy* other, ViewportCameraMapperPtr camera_mapper)
        : DefCamFollowStrategy(other)
        , m_camera_mapper(camera_mapper)
    {
    }
    ~StageXformableStrategy() override = default;

    void push() override
    {
        if (!m_is_camera_transform_dirty)
            return;
        DefCamFollowStrategy::push();

        m_camera_mapper->push(m_camera.GetTransform(), m_time);
    }

    void pull() override
    {
        m_camera = m_camera_mapper->pull(m_time);
        m_camera.SetFocusDistance(m_dist);
        DefCamFollowStrategy::pull();
    }

    GfCamera get_camera() override
    {
        push();
        if (m_time_changed)
        {
            pull();
            m_time_changed = false;
        }
        return m_camera;
    }

    ViewportCameraController::FollowMode get_follow_mode() const override { return ViewportCameraController::FollowMode::StageXformablePrim; }
    virtual void set_time(UsdTimeCode time) override
    {
        FollowModeStrategy::set_time(time);
        m_time_changed = true;
    }

private:
    ViewportCameraMapperPtr m_camera_mapper;
    bool m_time_changed = false;
};

ViewportCameraController::ViewportCameraController(ViewportCameraMapperPtr camera_mapper, QObject* parent /*= nullptr*/)
    : QObject(parent)
{
    m_camera_mapper = camera_mapper;
    if (TF_VERIFY(m_camera_mapper, "Failed to register a camera mapper callback. The camera mapper is null."))
    {
        m_camera_mapper->set_prim_changed_callback([this] {
            if (!m_camera_mapper->is_valid())
            {
                set_follow_prim(SdfPath::EmptyPath());
            }
            else
            {
                m_follow_mode_strategy->pull();
            }
        });
    }
    update_follow_mode_strategy();
}

ViewportCameraController::~ViewportCameraController() {}

void ViewportCameraController::truck(double delta_right, double delta_up)
{
    m_follow_mode_strategy->truck(delta_right, delta_up);
}

void ViewportCameraController::pan_tilt(double delta_pan, double delta_tilt)
{
    m_follow_mode_strategy->pan_tilt(delta_pan, delta_tilt);
}

void ViewportCameraController::tumble(double delta_theta, double delta_phi)
{
    m_follow_mode_strategy->tumble(delta_theta, delta_phi);
}

SdfPath ViewportCameraController::get_follow_prim_path() const
{
    return m_camera_mapper ? m_camera_mapper->get_path() : SdfPath();
}

void ViewportCameraController::set_up_axis(const TfToken& up_axis)
{
    m_follow_mode_strategy->set_up_axis(up_axis);
}

GfCamera ViewportCameraController::get_gf_camera()
{
    return m_follow_mode_strategy->get_camera();
}

GfFrustum ViewportCameraController::get_frustum()
{
    auto frustum = get_gf_camera().GetFrustum();
    frustum.SetViewDistance(m_follow_mode_strategy->get_dist());
    return frustum;
}

void ViewportCameraController::set_follow_prim(const SdfPath& prim_path)
{
    m_camera_mapper->set_path(prim_path);
    update_follow_mode_strategy();
    camera_changed(prim_path);
}

double ViewportCameraController::get_fov() const
{
    // For orthographic cameras fov is the height of the view frustum, in world units.
    const auto camera = m_follow_mode_strategy->get_camera();
    return m_follow_mode_strategy->is_perspective() ? camera.GetFieldOfView(GfCamera::FOVHorizontal)
                                                    : get_orthographic_size(camera.GetHorizontalAperture());
}

ViewportCameraController::FollowMode ViewportCameraController::get_follow_mode() const
{
    return m_follow_mode_strategy->get_follow_mode();
}

void ViewportCameraController::update_camera_mapper(ViewportCameraMapperPtr camera_mapper)
{
    m_camera_mapper = camera_mapper;
    if (m_camera_mapper)
    {
        m_camera_mapper->set_prim_changed_callback([this] {
            if (!m_camera_mapper->is_valid())
            {
                set_follow_prim(SdfPath::EmptyPath());
            }
            else
            {
                m_follow_mode_strategy->pull();
            }
        });
    }
    m_follow_mode_strategy = std::make_unique<DefCamFollowStrategy>(m_follow_mode_strategy.get());
    update_follow_mode_strategy();
    camera_mapper_changed();
}

void OPENDCC_NAMESPACE::ViewportCameraController::set_time(PXR_NS::UsdTimeCode time)
{
    if (m_follow_mode_strategy)
        m_follow_mode_strategy->set_time(time);
}

void ViewportCameraController::update_follow_mode_strategy()
{
    const auto current_follow_mode = m_follow_mode_strategy ? m_follow_mode_strategy->get_follow_mode() : FollowMode::Invalid;
    if (!m_camera_mapper->is_valid())
    {
        if (current_follow_mode != FollowMode::DefCam)
            m_follow_mode_strategy = std::make_unique<DefCamFollowStrategy>(m_follow_mode_strategy.get());
    }
    else if (m_camera_mapper->is_read_only())
    {
        if (current_follow_mode != FollowMode::StageCamPrimReadOnly)
            m_follow_mode_strategy = std::make_unique<ReadOnlyStageCameraStrategy>(m_follow_mode_strategy.get(), m_camera_mapper);
    }
    else if (m_camera_mapper->is_camera_prim())
    {
        if (current_follow_mode != FollowMode::StageCamPrim)
            m_follow_mode_strategy = std::make_unique<StageCameraStrategy>(m_follow_mode_strategy.get(), m_camera_mapper);
    }
    else if (m_camera_mapper->is_valid() && !m_camera_mapper->is_camera_prim())
    {
        if (current_follow_mode != FollowMode::StageXformablePrim)
            m_follow_mode_strategy = std::make_unique<StageXformableStrategy>(m_follow_mode_strategy.get(), m_camera_mapper);
    }

    if (m_follow_mode_strategy)
    {
        m_follow_mode_strategy->pull();
    }
}

void ViewportCameraController::set_default_camera()
{
    set_follow_prim(SdfPath::EmptyPath());
}

void ViewportCameraController::adjust_distance(double scale_factor)
{
    m_follow_mode_strategy->adjust_distance(scale_factor);
}

double ViewportCameraController::compute_pixels_to_world_factor(int viewport_height)
{
    return m_follow_mode_strategy->compute_pixels_to_world_factor(viewport_height);
}

void ViewportCameraController::frame_selection(const PXR_NS::GfBBox3d& selection_bbox, double frame_fit)
{
    m_follow_mode_strategy->frame_selection(selection_bbox, frame_fit);
}

void ViewportCameraController::set_display_size(int w, int h)
{
    m_follow_mode_strategy->set_display_size(w, h);
}

OPENDCC_NAMESPACE_CLOSE
