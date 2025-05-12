/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/app/core/selection_list.h"
#include <pxr/base/tf/type.h>
#include <pxr/base/tf/hash.h>
#include <unordered_map>
#include <unordered_set>
#include <pxr/base/tf/token.h>
#include <opendcc/base/vendor/eventpp/eventdispatcher.h>
#include <opendcc/app/core/application.h>

OPENDCC_NAMESPACE_OPEN

class HydraRenderSettings;
class UsdHydraRenderSettings;

class SceneIndexManager
{
public:
    virtual ~SceneIndexManager() = default;
    virtual PXR_NS::HdSceneIndexBaseRefPtr get_terminal_index() = 0;
    virtual void set_selection(const SelectionList& selection_list) = 0;
};

class OPENDCC_API ViewportSceneContext
{
public:
    enum class EventType
    {
        DirtyRenderSettings
    };
    using Dispatcher = eventpp::EventDispatcher<EventType, void()>;
    using DispatcherHandle = Dispatcher::Handle;

    using SceneDelegateCollection = std::unordered_set<PXR_NS::TfType, PXR_NS::TfHash>;

    ViewportSceneContext(const PXR_NS::TfToken& context_name);
    virtual SceneDelegateCollection get_delegates() const = 0;
    virtual std::shared_ptr<SceneIndexManager> get_index_manager() const { return nullptr; };
    virtual SelectionList get_selection() const = 0;
    virtual void resolve_picking(PXR_NS::HdxPickHitVector& pick_hits) = 0;
    virtual std::shared_ptr<HydraRenderSettings> get_render_settings() const = 0;
    const PXR_NS::TfToken& get_context_name() const;
    virtual bool use_hydra2() const;
    DispatcherHandle register_event_handler(EventType event, const std::function<void()>& callback);
    void unregister_event_handler(EventType event, DispatcherHandle handle);

protected:
    void dispatch(EventType event);

private:
    PXR_NS::TfToken m_context_name;
    Dispatcher m_dispatcher;
};
using ViewportSceneContextPtr = std::shared_ptr<ViewportSceneContext>;

class OPENDCC_API ViewportSceneContextRegistry
{
public:
    using SceneContextCreateFn = std::function<ViewportSceneContextPtr()>;

    bool register_scene_context(const PXR_NS::TfToken& context_name, SceneContextCreateFn fn);
    bool unregister_scene_context(const PXR_NS::TfToken& context_name);
    ViewportSceneContextPtr create_scene_context(const PXR_NS::TfToken& context_name) const;

    static ViewportSceneContextRegistry& get_instance();

private:
    ViewportSceneContextRegistry() = default;
    ViewportSceneContextRegistry(const ViewportSceneContextRegistry&) = delete;
    ViewportSceneContextRegistry(ViewportSceneContextRegistry&&) = delete;
    ViewportSceneContextRegistry& operator=(const ViewportSceneContextRegistry&) = delete;
    ViewportSceneContextRegistry& operator=(ViewportSceneContextRegistry&&) = delete;

    std::unordered_map<PXR_NS::TfToken, SceneContextCreateFn, PXR_NS::TfToken::HashFunctor> m_registry;
};

class ViewportUsdSceneContext final : public ViewportSceneContext
{
public:
    ViewportUsdSceneContext(const PXR_NS::TfToken& context_name);
    ~ViewportUsdSceneContext();
    SceneDelegateCollection get_delegates() const override;
    SelectionList get_selection() const override;
    void resolve_picking(PXR_NS::HdxPickHitVector& pick_hits) override;
    virtual std::shared_ptr<HydraRenderSettings> get_render_settings() const override;

private:
    void update_stage_watcher();

    SceneDelegateCollection m_delegates;
    std::shared_ptr<UsdHydraRenderSettings> m_render_settings;
    PXR_NS::SdfPath m_render_settings_path;
    Application::CallbackHandle m_current_stage_changed_cid;
    Application::CallbackHandle m_time_changed_cid;
    std::unique_ptr<StageObjectChangedWatcher> m_stage_watcher;
};

OPENDCC_NAMESPACE_CLOSE
