/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/viewport/viewport_hydra_engine.h"

PXR_NAMESPACE_OPEN_SCOPE
class HdxFullscreenShader;
PXR_NAMESPACE_CLOSE_SCOPE

OPENDCC_NAMESPACE_OPEN

enum class DepthStyle
{
    None,
    NDC,
    Linear,
    OpenGL
};

struct ViewportRenderDelegateInfo
{
    DepthStyle depth_style = DepthStyle::OpenGL;
};

class OPENDCC_API ViewportEngineProxy
{
public:
    ViewportEngineProxy(const std::unordered_set<PXR_NS::TfType, PXR_NS::TfHash>& delegate_types);
    ViewportEngineProxy(const std::shared_ptr<SceneIndexManager>& si_manager);
    virtual ~ViewportEngineProxy();

    void render(ViewportHydraEngineParams& params);
    void set_camera_state(const PXR_NS::GfMatrix4d& view_matrix, const PXR_NS::GfMatrix4d& projection_matrix);
    void set_render_viewport(const PXR_NS::GfVec4d& viewport);
    void set_lighting_state(PXR_NS::GlfSimpleLightVector const& lights, PXR_NS::GlfSimpleMaterial const& material,
                            PXR_NS::GfVec4f const& sceneAmbient);

    static PXR_NS::TfTokenVector get_render_plugins();
    static std::string get_render_display_name(PXR_NS::TfToken const& id);
    PXR_NS::TfToken get_current_render_id() const;
    bool set_renderer_plugin(PXR_NS::TfToken const& id);
    void set_selected(const SelectionList& selection_state, const RichSelection& rich_selection = RichSelection());
    void set_rollover_prims(const PXR_NS::SdfPathVector& rollover_prims);
    void set_selection_color(PXR_NS::GfVec4f const& color);
    bool is_converged() const;
    void update(ViewportHydraEngineParams& engine_params);
    PXR_NS::GfRange3d get_bbox(const PXR_NS::SdfPath& path);
    bool is_hd_st() const;
    PXR_NS::HdRenderSettingDescriptorList get_render_setting_descriptors() const;
    PXR_NS::VtValue get_render_setting(const PXR_NS::TfToken& key) const;
    void set_render_setting(const PXR_NS::TfToken& key, const PXR_NS::VtValue& value);
    void set_scene_delegates(const std::unordered_set<PXR_NS::TfType, PXR_NS::TfHash>& delegate_types);
    void set_scene_index_manager(const std::shared_ptr<SceneIndexManager>& si_manager);
#if PXR_VERSION >= 2005
    PXR_NS::TfTokenVector get_renderer_aovs() const;
    PXR_NS::HdRenderBuffer* get_aov_texture(const PXR_NS::TfToken& aov) const;
    void set_renderer_aov(const PXR_NS::TfToken& aov_name);
    bool has_aov(const PXR_NS::TfToken& aov_name) const;
    PXR_NS::TfToken get_current_aov() const;
    void set_render_settings(std::shared_ptr<HydraRenderSettings> render_settings);
    std::shared_ptr<HydraRenderSettings> get_render_settings() const;
    static const ViewportRenderDelegateInfo& get_renderer_info(const PXR_NS::TfToken& id);
#endif
    bool test_intersection(const ViewportHydraIntersectionParams& params, PXR_NS::HdxPickHit& out_hit);
    bool test_intersection_batch(const ViewportHydraIntersectionParams& params, PXR_NS::HdxPickHitVector& out_hits);
    void reset();
    void restart();
    void resume();
    void pause();
    bool is_pause_supported() const;
    void stop();
    bool is_stop_supported() const;
#if PXR_VERSION < 2005
    PXR_NS::SdfPath get_prim_path_from_instance_index(PXR_NS::SdfPath const& protoRprimPath, int instanceIndex, int* absoluteInstanceIndex = NULL,
                                                      PXR_NS::SdfPath* rprimPath = NULL, PXR_NS::SdfPathVector* instanceContext = NULL);
#else
    PXR_NS::SdfPath get_prim_path_from_instance_index(const PXR_NS::SdfPath& rprimId, int instanceIndex,
                                                      PXR_NS::HdInstancerContext* instancer_context = nullptr);
#endif
#if PXR_VERSION >= 2108
    void set_framing(const PXR_NS::CameraUtilFraming& framing);
#ifdef OPENDCC_USE_HYDRA_FRAMING_API
    void set_render_buffer_size(const PXR_NS::GfVec2i& size);
    void set_override_window_policy(const std::pair<bool, PXR_NS::CameraUtilConformWindowPolicy>& policy);
#endif
#endif
#if PXR_VERSION >= 2008
    void update_render_settings();
#endif
private:
    ViewportEngineProxy();
    static void init_render_delegate_info();
    bool init_renderer(std::unique_ptr<ViewportHydraEngine>& renderer, const PXR_NS::TfToken& plugin_id,
                       std::unordered_set<PXR_NS::TfType, PXR_NS::TfHash> delegates);
    bool init_renderer(std::unique_ptr<ViewportHydraEngine>& renderer, const PXR_NS::TfToken& plugin_id);
    bool use_hydra2() const;

    struct CommonEngineSettings
    {
        SelectionList selection;
        RichSelection rich_selection;
        PXR_NS::SdfPathVector rollover_prims;
        PXR_NS::GfVec4f selection_color;
#if PXR_VERSION >= 2005
        std::shared_ptr<HydraRenderSettings> render_settings;
#endif
    };

    std::unique_ptr<ViewportHydraEngine> m_main_renderer = nullptr;
    std::unique_ptr<ViewportHydraEngine> m_locator_renderer = nullptr;
    std::unordered_set<PXR_NS::TfType, PXR_NS::TfHash> m_scene_delegates;
    std::shared_ptr<SceneIndexManager> m_si_manager;
    CommonEngineSettings m_common_settings;
    static std::unordered_map<PXR_NS::TfToken, ViewportRenderDelegateInfo, PXR_NS::TfToken::HashFunctor> s_renderer_delegate_infos;
};

OPENDCC_NAMESPACE_CLOSE
