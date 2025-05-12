// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_engine_proxy.h"
#include <pxr/imaging/hf/pluginDesc.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/base/tf/getenv.h>
#include <pxr/imaging/hdx/colorCorrectionTask.h>
#include "viewport_locator_delegate.h"
#include "pxr/base/tf/fileUtils.h"
#include <opendcc/base/vendor/jsoncpp/json.h>
#if PXR_VERSION >= 2005
#include <pxr/imaging/hdx/fullscreenShader.h>
#endif
#include "pxr/base/plug/registry.h"
#include "pxr/base/plug/plugin.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE;

namespace
{
    DepthStyle str_to_depth_style(const std::string& str, const std::string& plugin_id)
    {
        if (str == "none")
            return DepthStyle::None;
        if (str == "ndc")
            return DepthStyle::NDC;
        if (str == "linear")
            return DepthStyle::Linear;
        if (str == "opengl")
            return DepthStyle::OpenGL;

        OPENDCC_WARN("Failed to parse 'depth_style' value '{}' for plugin '{}'. Fallback to OpenGL style.", str, plugin_id);
        return DepthStyle::OpenGL;
    }

};

std::unordered_map<TfToken, ViewportRenderDelegateInfo, TfToken::HashFunctor> ViewportEngineProxy::s_renderer_delegate_infos;

ViewportEngineProxy::ViewportEngineProxy()
{
    static std::once_flag once_flag;
    std::call_once(once_flag, [] { init_render_delegate_info(); });
    m_common_settings.selection_color = Application::instance().get_settings()->get("viewport.selection_color", GfVec4f(1, 1, 0, 0.5));
}

ViewportEngineProxy::ViewportEngineProxy(const std::unordered_set<TfType, TfHash>& delegate_types)
    : ViewportEngineProxy()
{
    m_scene_delegates = delegate_types;
    auto locator_delegate_type = TfType::FindByName("ViewportLocatorDelegate");
    auto main_renderer_delegates = m_scene_delegates;
    if (main_renderer_delegates.erase(locator_delegate_type) > 0)
        init_renderer(m_locator_renderer, ViewportHydraEngine::get_default_render_plugin(),
                      decltype(main_renderer_delegates)({ locator_delegate_type }));
    else
        m_locator_renderer = nullptr;
    init_renderer(m_main_renderer, ViewportHydraEngine::get_default_render_plugin(), main_renderer_delegates);
}

ViewportEngineProxy::ViewportEngineProxy(const std::shared_ptr<SceneIndexManager>& si_manager)
    : ViewportEngineProxy()
{
    m_si_manager = si_manager;
    // init_renderer(m_locator_renderer, ViewportHydraEngine::get_default_render_plugin());
    init_renderer(m_main_renderer, ViewportHydraEngine::get_default_render_plugin());
}

ViewportEngineProxy::~ViewportEngineProxy()
{
    // Bear in mind that locator renderer depends on main renderer's HdRenderIndex.
    // This means we should delete locator renderer first.
    m_locator_renderer = nullptr;
    m_main_renderer = nullptr;
}

void ViewportEngineProxy::render(ViewportHydraEngineParams& params)
{
    m_main_renderer->render(params);
    const auto& renderer_info = s_renderer_delegate_infos[m_main_renderer->get_current_render_id()];

    // Convert depth to OpenGL format that hydra uses [0; 1]
    if (renderer_info.depth_style != DepthStyle::OpenGL)
    {
        // init_depth_compositor(renderer_info.depth_style);
    }
    if (m_locator_renderer)
    {
        auto locator_params = params;
        locator_params.color_correction_mode = HdxColorCorrectionTokens->disabled;
        locator_params.enable_scene_materials = true;
        m_locator_renderer->render(locator_params);
    }
}

void ViewportEngineProxy::set_camera_state(const GfMatrix4d& view_matrix, const GfMatrix4d& projection_matrix)
{
    m_main_renderer->set_camera_state(view_matrix, projection_matrix);
    if (m_locator_renderer)
        m_locator_renderer->set_camera_state(view_matrix, projection_matrix);
}

void ViewportEngineProxy::set_render_viewport(const GfVec4d& viewport)
{
    m_main_renderer->set_render_viewport(viewport);
    if (m_locator_renderer)
        m_locator_renderer->set_render_viewport(viewport);
}

void ViewportEngineProxy::set_lighting_state(GlfSimpleLightVector const& lights, GlfSimpleMaterial const& material, GfVec4f const& sceneAmbient)
{
    m_main_renderer->set_lighting_state(lights, material, sceneAmbient);
    if (m_locator_renderer)
        m_locator_renderer->set_lighting_state(lights, material, sceneAmbient);
}

TfTokenVector ViewportEngineProxy::get_render_plugins()
{
    return ViewportHydraEngine::get_render_plugins();
}

std::string ViewportEngineProxy::get_render_display_name(TfToken const& id)
{
    return ViewportHydraEngine::get_render_display_name(id);
}

void ViewportEngineProxy::init_render_delegate_info()
{
    const auto root_path = Application::instance().get_application_root_path();
    const auto plugin_path = root_path + "/plugin";

    const auto renderer_info_paths = TfGetenv("OPENDCC_RENDERER_INFO_PATH");
#ifdef OPENDCC_OS_WINDOWS
    auto plugin_dirs = TfStringSplit(renderer_info_paths, ";");
#else
    auto plugin_dirs = TfStringSplit(renderer_info_paths, ":");
#endif
    const auto root_plugin_dir = TfListDir(plugin_path);
    plugin_dirs.insert(plugin_dirs.end(), root_plugin_dir.begin(), root_plugin_dir.end());

    for (const auto& plugin_dir : plugin_dirs)
    {
        if (!TfIsDir(plugin_dir))
            continue;
        const auto renderer_info_file = plugin_dir + "/renderer_info.json";
        if (!TfIsFile(renderer_info_file))
            continue;

        Json::Value root;
        std::ifstream file(renderer_info_file);
        if (!file.is_open())
        {
            OPENDCC_ERROR("Failed to open '{}'.", renderer_info_file);
            continue;
        }
        Json::CharReaderBuilder builder;
        std::string errs;
        if (!parseFromStream(builder, file, &root, &errs) || !errs.empty())
        {
            OPENDCC_ERROR("Failed to parse '{}'. {}", renderer_info_file, errs);
            continue;
        }
        for (auto plug_iter = root.begin(); plug_iter != root.end(); ++plug_iter)
        {
            const auto plugin_id = plug_iter.name();
            if (s_renderer_delegate_infos.find(TfToken(plugin_id)) != s_renderer_delegate_infos.end())
                continue;

            Json::Value depth_style("opengl");
            depth_style = plug_iter->get("depth_style", depth_style);

            ViewportRenderDelegateInfo renderer_info;
            renderer_info.depth_style = str_to_depth_style(depth_style.asString(), plugin_id);

            s_renderer_delegate_infos.emplace(TfToken(plugin_id), std::move(renderer_info));
        }
    }
}

TfToken ViewportEngineProxy::get_current_render_id() const
{
    return m_main_renderer->get_current_render_id();
}

bool ViewportEngineProxy::set_renderer_plugin(TfToken const& id)
{
    auto renderer_plugin_id = id;
    if (id.IsEmpty())
        renderer_plugin_id = HdRendererPluginRegistry::GetInstance().GetDefaultPluginId();

    if (m_main_renderer->get_current_render_id() == id)
        return false;

    // Use only one hydra engine for HdStorm render delegate
    // Use m_locator_renderer only for locators and guide rendering
    if (use_hydra2())
    {
        // init_renderer(m_locator_renderer, ViewportHydraEngine::get_default_render_plugin());
        return init_renderer(m_main_renderer, renderer_plugin_id);
    }
    else
    {
        auto locator_delegate_type = TfType::FindByTypeid(typeid(ViewportLocatorDelegate));
        auto main_renderer_delegates = m_scene_delegates;
        if (main_renderer_delegates.erase(locator_delegate_type) > 0)
            init_renderer(m_locator_renderer, ViewportHydraEngine::get_default_render_plugin(),
                          decltype(main_renderer_delegates)({ locator_delegate_type }));
        else
            m_locator_renderer = nullptr;
        return init_renderer(m_main_renderer, renderer_plugin_id, main_renderer_delegates);
    }
}

void ViewportEngineProxy::set_selected(const SelectionList& selection_state, const RichSelection& rich_selection /*= RichSelection()*/)
{
    m_common_settings.selection = selection_state;
    m_common_settings.rich_selection = rich_selection;
    m_main_renderer->set_selected(selection_state, rich_selection);
    if (m_locator_renderer)
        m_locator_renderer->set_selected(selection_state, rich_selection);
}

void ViewportEngineProxy::set_rollover_prims(const SdfPathVector& rollover_prims)
{
    m_common_settings.rollover_prims = rollover_prims;
    m_main_renderer->set_rollover_prims(rollover_prims);
    if (m_locator_renderer)
        m_locator_renderer->set_rollover_prims(rollover_prims);
}

void ViewportEngineProxy::set_selection_color(GfVec4f const& color)
{
    m_common_settings.selection_color = color;
    m_main_renderer->set_selection_color(color);
    if (m_locator_renderer)
        m_locator_renderer->set_selection_color(color);
}

bool ViewportEngineProxy::is_converged() const
{
    return m_main_renderer->is_converged() && (!m_locator_renderer || m_locator_renderer->is_converged());
}

void ViewportEngineProxy::update(ViewportHydraEngineParams& engine_params)
{
    engine_params.main_render_index = m_main_renderer->get_render_index();
    auto cpy = engine_params;
    m_main_renderer->update_init(cpy);
    cpy.main_render_index = m_main_renderer->get_render_index();
    if (use_hydra2())
    {
        m_main_renderer->update_scene_indices(cpy);
    }
    else
    {
        m_main_renderer->update_delegates(cpy);
    }
    if (m_locator_renderer)
    {
        auto locator_params = cpy;
        locator_params.color_correction_mode = HdxColorCorrectionTokens->disabled;
        locator_params.enable_scene_materials = true;
        m_locator_renderer->update_init(locator_params);
        if (use_hydra2())
        {
            m_locator_renderer->update_scene_indices(locator_params);
        }
        else
        {
            m_locator_renderer->update_delegates(locator_params);
        }
    }
}

GfRange3d ViewportEngineProxy::get_bbox(const SdfPath& path)
{
    if (m_locator_renderer)
        return m_main_renderer->get_bbox(path).UnionWith(m_locator_renderer->get_bbox(path));
    else
        return m_main_renderer->get_bbox(path);
}

bool ViewportEngineProxy::is_hd_st() const
{
    return m_main_renderer->is_hd_st();
}

HdRenderSettingDescriptorList ViewportEngineProxy::get_render_setting_descriptors() const
{
    return m_main_renderer->get_render_setting_descriptors();
}

VtValue ViewportEngineProxy::get_render_setting(const TfToken& key) const
{
    return m_main_renderer->get_render_setting(key);
}

void ViewportEngineProxy::set_render_setting(const TfToken& key, const VtValue& value)
{
    m_main_renderer->set_render_setting(key, value);
}

void ViewportEngineProxy::set_scene_delegates(const std::unordered_set<TfType, TfHash>& delegate_types)
{
    m_si_manager = nullptr;
    m_scene_delegates = delegate_types;
    auto locator_delegate_type = TfType::FindByTypeid(typeid(ViewportLocatorDelegate));
    auto main_renderer_delegates = m_scene_delegates;
    if (main_renderer_delegates.erase(locator_delegate_type))
        init_renderer(m_locator_renderer, ViewportHydraEngine::get_default_render_plugin(),
                      decltype(main_renderer_delegates)({ locator_delegate_type }));
    else
        m_locator_renderer = nullptr;
    m_main_renderer->set_scene_delegates(main_renderer_delegates);
}

void ViewportEngineProxy::set_scene_index_manager(const std::shared_ptr<SceneIndexManager>& si_manager)
{
    m_si_manager = si_manager;
    // For now only for main renderer
    // m_locator_renderer->set_scene_index_manager(si_manager);
    m_locator_renderer = nullptr;
    m_main_renderer->set_scene_index_manager(si_manager);
}

#if PXR_VERSION >= 2005
TfTokenVector ViewportEngineProxy::get_renderer_aovs() const
{
    return m_main_renderer->get_renderer_aovs();
}

HdRenderBuffer* OPENDCC_NAMESPACE::ViewportEngineProxy::get_aov_texture(const TfToken& aov) const
{
    return m_main_renderer->get_aov_texture(aov);
}

void ViewportEngineProxy::set_renderer_aov(const TfToken& aov_name)
{
    m_main_renderer->set_renderer_aov(aov_name);
}

bool ViewportEngineProxy::has_aov(const TfToken& aov_name) const
{
    return m_main_renderer->has_aov(aov_name);
}

TfToken ViewportEngineProxy::get_current_aov() const
{
    return m_main_renderer->get_current_aov();
}

void ViewportEngineProxy::set_render_settings(std::shared_ptr<HydraRenderSettings> render_settings)
{
    m_common_settings.render_settings = render_settings;
    m_main_renderer->set_render_settings(render_settings);
}

std::shared_ptr<HydraRenderSettings> ViewportEngineProxy::get_render_settings() const
{
    return m_main_renderer->get_render_settings();
}

const ViewportRenderDelegateInfo& ViewportEngineProxy::get_renderer_info(const TfToken& id)
{
    static ViewportRenderDelegateInfo def_value;
    auto it = s_renderer_delegate_infos.find(id);
    if (it == s_renderer_delegate_infos.end())
        return def_value;
    return it->second;
}

#endif

bool ViewportEngineProxy::test_intersection(const ViewportHydraIntersectionParams& params, HdxPickHit& out_hit)
{
    HdxPickHitVector out_hits;
    auto result = m_main_renderer->test_intersection_batch(params, out_hits);
    if (result)
    {
        out_hit = out_hits[0];
    }
    // Avoid unnecessary pick task for components of locators.
    // It makes sense to select only full prims.
    if (m_locator_renderer && (params.pick_target & SelectionList::SelectionFlags::FULL_SELECTION))
    {
        HdxPickHitVector locator_pick_result;
        auto locator_hit_result = m_locator_renderer->test_intersection_batch(params, locator_pick_result);
#if PXR_VERSION >= 2002
        if ((locator_hit_result && !result) || (locator_hit_result && locator_pick_result[0].normalizedDepth < out_hit.normalizedDepth))
#else
        if ((locator_hit_result && !result) || (locator_hit_result && locator_pick_result.ndcDepth < out_hit.ndcDepth))
#endif
        {
            result = locator_hit_result;
            out_hit = locator_pick_result[0];
        }
    }
    return result;
}

bool ViewportEngineProxy::test_intersection_batch(const ViewportHydraIntersectionParams& params, HdxPickHitVector& out_hits)
{
    auto result = m_main_renderer->test_intersection_batch(params, out_hits);
    // Avoid unnecessary pick task for components of locators.
    // It makes sense to select only full prims.
    if (m_locator_renderer && (params.pick_target & SelectionList::SelectionFlags::FULL_SELECTION))
    {
        HdxPickHitVector locator_pick_results;
        result |= m_locator_renderer->test_intersection_batch(params, locator_pick_results);
        out_hits.insert(out_hits.end(), locator_pick_results.begin(), locator_pick_results.end());
    }
    return result;
}

void ViewportEngineProxy::reset()
{
    if (m_locator_renderer)
        m_locator_renderer->reset();
    m_main_renderer->reset();
}

void ViewportEngineProxy::restart()
{
    if (m_locator_renderer)
        m_locator_renderer->restart();
    m_main_renderer->restart();
}

void ViewportEngineProxy::resume()
{
    m_main_renderer->resume();
}

void ViewportEngineProxy::pause()
{
    m_main_renderer->pause();
}

bool ViewportEngineProxy::is_pause_supported() const
{
    return m_main_renderer->is_pause_supported();
}

void ViewportEngineProxy::stop()
{
    m_main_renderer->stop();
}

bool ViewportEngineProxy::is_stop_supported() const
{
    return m_main_renderer->is_stop_supported();
}

#if PXR_VERSION < 2005
SdfPath ViewportEngineProxy::get_prim_path_from_instance_index(SdfPath const& protoRprimPath, int instanceIndex, int* absoluteInstanceIndex,
                                                               SdfPath* rprimPath, SdfPathVector* instanceContext)
{
    return m_main_renderer->get_prim_path_from_instance_index(protoRprimPath, instanceIndex, absoluteInstanceIndex, rprimPath, instanceContext);
}
#else
SdfPath ViewportEngineProxy::get_prim_path_from_instance_index(const SdfPath& rprimId, int instanceIndex, HdInstancerContext* instancer_context)
{
    return m_main_renderer->get_prim_path_from_instance_index(rprimId, instanceIndex, instancer_context);
}
#endif

#if PXR_VERSION >= 2108
void ViewportEngineProxy::set_framing(const CameraUtilFraming& framing)
{
    m_main_renderer->set_framing(framing);
    if (m_locator_renderer)
        m_locator_renderer->set_framing(framing);
}
#ifdef OPENDCC_USE_HYDRA_FRAMING_API

void ViewportEngineProxy::set_render_buffer_size(const GfVec2i& size)
{
    m_main_renderer->set_render_buffer_size(size);
    if (m_locator_renderer)
        m_locator_renderer->set_render_buffer_size(size);
}

void ViewportEngineProxy::set_override_window_policy(const std::pair<bool, CameraUtilConformWindowPolicy>& policy)
{
    m_main_renderer->set_override_window_policy(policy);
    if (m_locator_renderer)
        m_locator_renderer->set_override_window_policy(policy);
}
#endif
#endif

#if PXR_VERSION >= 2008
void ViewportEngineProxy::update_render_settings()
{
    m_main_renderer->update_render_settings();
}
#endif

bool ViewportEngineProxy::init_renderer(std::unique_ptr<ViewportHydraEngine>& renderer, const TfToken& plugin_id,
                                        std::unordered_set<TfType, TfHash> delegates)
{
    bool result = true;
    renderer = std::make_unique<ViewportHydraEngine>(delegates);
    if (!renderer->set_renderer_plugin(plugin_id))
    {
        renderer->set_renderer_plugin(ViewportHydraEngine::get_default_render_plugin());
        result = false;
    }
    renderer->set_selection_color(m_common_settings.selection_color);
    renderer->set_selected(m_common_settings.selection, m_common_settings.rich_selection);
    renderer->set_rollover_prims(m_common_settings.rollover_prims);
#if PXR_VERSION >= 2005
    renderer->set_render_settings(m_common_settings.render_settings);
#endif
    return result;
}

bool ViewportEngineProxy::init_renderer(std::unique_ptr<ViewportHydraEngine>& renderer, const PXR_NS::TfToken& plugin_id)
{
    bool result = true;
    renderer = std::make_unique<ViewportHydraEngine>(m_si_manager);
    if (!renderer->set_renderer_plugin(plugin_id))
    {
        renderer->set_renderer_plugin(ViewportHydraEngine::get_default_render_plugin());
        result = false;
    }
    renderer->set_selection_color(m_common_settings.selection_color);
    renderer->set_selected(m_common_settings.selection, m_common_settings.rich_selection);
    renderer->set_rollover_prims(m_common_settings.rollover_prims);
#if PXR_VERSION >= 2005
    renderer->set_render_settings(m_common_settings.render_settings);
#endif
    return result;
}

bool ViewportEngineProxy::use_hydra2() const
{
    return m_si_manager != nullptr;
}

OPENDCC_NAMESPACE_CLOSE
