/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/anim_engine/core/api.h"
#include "opendcc/anim_engine/core/engine.h"
#include <pxr/usd/usd/stageCache.h>

OPENDCC_NAMESPACE_OPEN

class ANIM_ENGINE_API AnimEngineSession
{
public:
    enum class EventType
    {
        CURRENT_STAGE_ANIM_CHANGED,
        CURVES_ADDED,
        CURVES_REMOVED,
    };
    typedef eventpp::EventDispatcher<EventType, void()> EventDispatcher;
    typedef eventpp::EventDispatcher<EventType, void()>::Handle BasicEventDispatcherHandle;

    static AnimEngineSession& instance();
    AnimEnginePtr current_engine();
    AnimEnginePtr engine(const PXR_NS::UsdStageCache::Id& stage_id);
    static adsk::KeyId generate_unique_key_id();

    BasicEventDispatcherHandle register_event_callback(EventType type, const std::function<void()>& callback)
    {
        return m_dispatcher.appendListener(type, callback);
    }
    void unregister_event_callback(EventType type, BasicEventDispatcherHandle& handle) { m_dispatcher.removeListener(type, handle); }

private:
    AnimEngineSession();
    ~AnimEngineSession();
    void current_stage_changed();
    void current_time_changed();
    void session_stage_list_changed();
    AnimEnginePtr m_current_engine;
    std::map<AnimEngine::EventType, AnimEngine::CurveUpdateCallbackHandle> m_events;
    std::map<AnimEngine::EventType, AnimEngine::KeysListUpdateCallbackHandle> m_keys_events;
    std::map<Application::EventType, Application::CallbackHandle> m_application_event_handles;
    EventDispatcher m_dispatcher;
    std::unordered_map<long int, AnimEnginePtr> m_engines;
};

OPENDCC_NAMESPACE_CLOSE
