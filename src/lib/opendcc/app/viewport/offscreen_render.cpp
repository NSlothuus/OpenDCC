// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <pxr/pxr.h>
#if PXR_VERSION < 2108
#include <pxr/imaging/glf/glew.h> // "glew.h" needs to be included before "gl.h". One of the "includes" below includes it, so the order of "includes" here is strange.
#else
#include <pxr/imaging/garch/glApi.h>
#endif
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdUtils/timeCodeRange.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>

#include "opendcc/app/viewport/offscreen_render.h"
#include "opendcc/app/viewport/viewport_engine_proxy.h"
#include "opendcc/app/viewport/viewport_render_frame_processor.h"

PXR_NAMESPACE_USING_DIRECTIVE

OPENDCC_NAMESPACE_OPEN

void ViewportOffscreenRender::set_render_params(const ViewportHydraEngineParams& params)
{
    m_params = params;
}

#if PXR_VERSION >= 2008
void ViewportOffscreenRender::set_render_settings(std::shared_ptr<HydraRenderSettings> render_settings)
{
    m_render_settings = render_settings;

#if PXR_VERSION >= 2008
    m_engine->set_render_settings(m_render_settings);
#endif
}

std::shared_ptr<HydraRenderSettings> ViewportOffscreenRender::get_render_settings() const
{
    return m_render_settings;
}
#endif

void ViewportOffscreenRender::set_camera_prim(PXR_NS::UsdPrim cam_prim)
{
    m_camera_prim = cam_prim;
}

void ViewportOffscreenRender::set_camera_state(const PXR_NS::GfMatrix4d& view, const PXR_NS::GfMatrix4d& proj, const PXR_NS::GfVec3d& pos)
{
    m_view = view;
    m_proj = proj;
    m_cam_pos = pos;
    m_camera_prim = UsdPrim();
}

render_system::RenderStatus ViewportOffscreenRender::render(UsdTimeCode start_frame, UsdTimeCode end_frame,
                                                            std::shared_ptr<ViewportRenderFrameProcessor> processor)
{
    if (!processor)
        return render_system::RenderStatus::FAILED;

    const auto w = m_params.render_resolution[0];
    const auto h = m_params.render_resolution[1];
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glViewport(0, 0, w, h);
    set_lighting_state();
#if PXR_VERSION >= 2108
    CameraUtilFraming framing { GfRange2f(GfVec2f(0, 0), GfVec2f(m_params.render_resolution[0], m_params.render_resolution[1])),
                                m_params.crop_region };
    m_engine->set_framing(framing);
#ifdef OPENDCC_USE_HYDRA_FRAMING_API
    if (framing.IsValid())
    {
        m_engine->set_render_buffer_size(GfVec2i(viewport_dim.width, viewport_dim.height));
        m_engine->set_override_window_policy({ true, CameraUtilFit });
    }
    else
    {
        m_engine->set_render_viewport(GfVec4d(0, 0, m_params.render_resolution[0], m_params.render_resolution[1]));
    }
#endif
#else
    m_engine->set_render_viewport(GfVec4d(0, 0, m_params.render_resolution[0], m_params.render_resolution[1]));
#endif
    for (auto time : UsdUtilsTimeCodeRange(start_frame, end_frame))
    {
        m_params.frame = time;
        if (auto camera = UsdGeomCamera(m_camera_prim))
        {
            auto frustum = camera.GetCamera(time).GetFrustum();
            CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit, h != 0 ? (double)w / h : 1.0);
            m_view = frustum.ComputeViewMatrix();
            m_proj = frustum.ComputeProjectionMatrix();
            m_cam_pos = frustum.GetPosition();
            set_lighting_state();
        }
        m_engine->set_camera_state(m_view, m_proj);
        m_engine->update(m_params);
        m_engine->update(m_params);
        m_engine->render(m_params);
        processor->post_frame(m_params, m_engine);
    }
    return render_system::RenderStatus::FINISHED;
}

void ViewportOffscreenRender::update()
{
    if (m_engine)
    {
        m_engine->update(m_params);
    }
}

bool ViewportOffscreenRender::is_converged() const
{
    return m_engine->is_converged();
}

void ViewportOffscreenRender::stop()
{
    if (m_engine->is_stop_supported())
    {
        m_engine->stop();
    }
}

void ViewportOffscreenRender::pause()
{
    if (m_engine->is_pause_supported())
    {
        m_engine->pause();
    }
}

void ViewportOffscreenRender::resume()
{
    if (m_engine->is_pause_supported())
    {
        m_engine->resume();
    }
}

std::shared_ptr<ViewportEngineProxy> ViewportOffscreenRender::get_engine()
{
    return m_engine;
}

#if PXR_VERSION >= 2108
void ViewportOffscreenRender::set_crop_region(const GfRect2i& crop_region)
{
    m_params.crop_region = crop_region;
}

const PXR_NS::GfRect2i& ViewportOffscreenRender::get_crop_region() const
{
    return m_params.crop_region;
}
#endif

void ViewportOffscreenRender::set_lighting_state()
{
    GlfSimpleLightVector lights;
    if (m_params.use_camera_light)
    {
        GlfSimpleLight camera_light;
        camera_light.SetAmbient(GfVec4f(0, 0, 0, 0));
        camera_light.SetPosition(GfVec4f(m_cam_pos[0], m_cam_pos[1], m_cam_pos[2], 1));
        lights.push_back(camera_light);
    }
    GlfSimpleMaterial material;
    material.SetAmbient(GfVec4f(0.2, 0.2, 0.2, 1));
    material.SetSpecular(GfVec4f(0.1, 0.1, 0.1, 1));
    material.SetShininess(32.0);
    GfVec4f scene_ambient(0.01, 0.01, 0.01, 1.0);
    m_engine->set_lighting_state(lights, material, scene_ambient);
}

const ViewportHydraEngineParams& ViewportOffscreenRender::get_render_params() const
{
    return m_params;
}

TfToken ViewportOffscreenRender::get_renderer_plugin(const TfToken& id) const
{
    return m_engine->get_current_render_id();
}

ViewportOffscreenRender::ViewportOffscreenRender(const std::shared_ptr<ViewportSceneContext>& scene_context)
{
    TF_AXIOM(scene_context);
    if (scene_context->use_hydra2())
    {
        m_engine = std::make_shared<ViewportEngineProxy>(scene_context->get_index_manager());
    }
    else
    {
        m_engine = std::make_shared<ViewportEngineProxy>(scene_context->get_delegates());
    }
}

bool ViewportOffscreenRender::set_renderer_plugin(const TfToken& id)
{
    return m_engine->set_renderer_plugin(id);
}

OPENDCC_NAMESPACE_CLOSE
