/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "api.h"
#include "opendcc/usd_editor/bullet_physics/engine.h"
#include <pxr/usd/usd/stageCache.h>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API_EXPORT BulletPhysicsSession
{
public:
    static BulletPhysicsSession& instance();
    BulletPhysicsEnginePtr current_engine();
    BulletPhysicsEnginePtr engine(const PXR_NS::UsdStageCache::Id& stage_id);
    void create_engin_for_current_stage();
    bool is_enabled() const;
    void set_enabled(bool enable);

private:
    BulletPhysicsSession();
    ~BulletPhysicsSession();
    void current_stage_changed();
    void selection_changed();
    void session_stage_list_changed();
    std::unordered_map<long int, BulletPhysicsEnginePtr> m_engines;
    std::map<Application::EventType, Application::CallbackHandle> m_application_event_handles;
    bool m_is_enable = false;
};

OPENDCC_NAMESPACE_CLOSE
