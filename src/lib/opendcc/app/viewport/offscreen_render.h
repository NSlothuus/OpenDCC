/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/viewport/viewport_hydra_engine.h"

#if PXR_VERSION >= 2108
#include <pxr/imaging/cameraUtil/framing.h>
#endif
#include "opendcc/render_system/irender.h"

OPENDCC_NAMESPACE_OPEN

class ViewportRenderFrameProcessor;
class ViewportEngineProxy;
class ViewportSceneContext;

class OPENDCC_API ViewportOffscreenRender
{
public:
    ViewportOffscreenRender(const std::shared_ptr<ViewportSceneContext>& scene_context);
    bool set_renderer_plugin(const PXR_NS::TfToken& id);
    PXR_NS::TfToken get_renderer_plugin(const PXR_NS::TfToken& id) const;
    void set_render_params(const ViewportHydraEngineParams& params);
    const ViewportHydraEngineParams& get_render_params() const;
    void set_camera_prim(PXR_NS::UsdPrim cam_prim);
#if PXR_VERSION >= 2008
    void set_render_settings(std::shared_ptr<HydraRenderSettings> render_settings);
    std::shared_ptr<HydraRenderSettings> get_render_settings() const;
#endif
    void set_camera_state(const PXR_NS::GfMatrix4d& view, const PXR_NS::GfMatrix4d& proj, const PXR_NS::GfVec3d& pos);
    render_system::RenderStatus render(PXR_NS::UsdTimeCode start_frame, PXR_NS::UsdTimeCode end_frame,
                                       std::shared_ptr<ViewportRenderFrameProcessor> processor);
    void update();
    bool is_converged() const;
    void stop();
    void pause();
    void resume();
    std::shared_ptr<ViewportEngineProxy> get_engine();

#if PXR_VERSION >= 2108
    void set_crop_region(const PXR_NS::GfRect2i& crop_region);
    const PXR_NS::GfRect2i& get_crop_region() const;
#endif

private:
    void set_lighting_state();

    ViewportHydraEngineParams m_params;
    std::shared_ptr<ViewportEngineProxy> m_engine;
#if PXR_VERSION >= 2008
    std::shared_ptr<HydraRenderSettings> m_render_settings;
#endif

    PXR_NS::UsdPrim m_camera_prim;
    PXR_NS::GfMatrix4d m_view;
    PXR_NS::GfMatrix4d m_proj;
    PXR_NS::GfVec3d m_cam_pos;
};

OPENDCC_NAMESPACE_CLOSE
