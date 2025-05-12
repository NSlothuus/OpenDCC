// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/core/session.h"
#include "opendcc/base/logging/logger.h"
#include "opendcc/usd_editor/bullet_physics/session.h"

PXR_NAMESPACE_USING_DIRECTIVE;

OPENDCC_NAMESPACE_OPEN

BulletPhysicsSession& BulletPhysicsSession::instance()
{
    static BulletPhysicsSession engine;
    return engine;
}

BulletPhysicsEnginePtr BulletPhysicsSession::current_engine()
{
    auto stage_id = Application::instance().get_session()->get_current_stage_id();
    return engine(stage_id);
}

BulletPhysicsEnginePtr BulletPhysicsSession::engine(const PXR_NS::UsdStageCache::Id& stage_id)
{
    auto stage = Application::instance().get_session()->get_stage_cache().Find(stage_id);
    if (!stage)
        return nullptr;

    auto it = instance().m_engines.find(stage_id.ToLongInt());
    if (it != instance().m_engines.end())
    {
        return it->second;
    }
    else
    {
        auto new_engine = std::make_shared<BulletPhysicsEngine>(stage, Application::instance().get_current_time());
        m_engines.emplace(stage_id.ToLongInt(), new_engine);
        return new_engine;
    }
}

void BulletPhysicsSession::create_engin_for_current_stage()
{
    engine(Application::instance().get_session()->get_current_stage_id());
}

void BulletPhysicsSession::set_enabled(bool enable)
{
    if (m_is_enable == enable)
        return;

    for (auto it : m_engines)
        if (enable)
            it.second->activate();
        else
            it.second->deactivate();
    m_is_enable = enable;
}

bool BulletPhysicsSession::is_enabled() const
{
    return m_is_enable;
}

BulletPhysicsSession::BulletPhysicsSession()
{
    m_application_event_handles[Application::EventType::SELECTION_CHANGED] =
        Application::instance().register_event_callback(Application::EventType::SELECTION_CHANGED, [this] { selection_changed(); });
    m_application_event_handles[Application::EventType::SESSION_STAGE_LIST_CHANGED] = Application::instance().register_event_callback(
        Application::EventType::SESSION_STAGE_LIST_CHANGED, [this]() { session_stage_list_changed(); });
    m_application_event_handles[Application::EventType::CURRENT_STAGE_CHANGED] =
        Application::instance().register_event_callback(Application::EventType::CURRENT_STAGE_CHANGED, [this]() { current_stage_changed(); });
}

BulletPhysicsSession::~BulletPhysicsSession()
{
    for (auto& it : m_application_event_handles)
        Application::instance().unregister_event_callback(it.first, it.second);
}

void BulletPhysicsSession::current_stage_changed() {}

void BulletPhysicsSession::session_stage_list_changed()
{
    std::vector<long int> to_delete;
    for (auto it : m_engines)
    {
        auto stage_id = UsdStageCache::Id::FromLongInt(it.first);
        auto stage = Application::instance().get_session()->get_stage_cache().Find(stage_id);
        if (!stage)
            to_delete.push_back(it.first);
    }

    for (auto id : to_delete)
        m_engines.erase(id);

    auto stage = Application::instance().get_session()->get_current_stage();
    if (stage)
        create_engin_for_current_stage();
}

void BulletPhysicsSession::selection_changed()
{
    auto engine = current_engine();
    if (engine)
    {
        engine->on_selection_changed();
    }
}

OPENDCC_NAMESPACE_CLOSE
