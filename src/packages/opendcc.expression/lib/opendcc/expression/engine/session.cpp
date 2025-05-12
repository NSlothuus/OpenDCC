// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/core/session.h"
#include "opendcc/base/logging/logger.h"
#include "session.h"

#include <pxr/usd/usd/stageCache.h>
#include <pxr/usd/usd/inherits.h>

PXR_NAMESPACE_USING_DIRECTIVE;

OPENDCC_NAMESPACE_OPEN

ExpressionSession& ExpressionSession::instance()
{
    static ExpressionSession session;
    return session;
}

ExpressionEnginePtr ExpressionSession::current_engine()
{
    auto stage_id = Application::instance().get_session()->get_current_stage_id();
    return engine(stage_id);
}

ExpressionEnginePtr ExpressionSession::engine(const PXR_NS::UsdStageCache::Id& stage_id)
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
        auto new_engine = std::make_shared<ExpressionEngine>(stage);
        m_engines.emplace(stage_id.ToLongInt(), new_engine);
        return new_engine;
    }
}

void ExpressionSession::create_engine_for_current_stage()
{
    engine(Application::instance().get_session()->get_current_stage_id());
}

void ExpressionSession::set_enabled(bool enable)
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

void ExpressionSession::add_evaluate_function(const EvaluateFunction& evaluate_function)
{
    m_evaluate_functions.push_back(evaluate_function);
}

bool ExpressionSession::evaluate_string(const ExpressionContext& context, const std::string& key, std::string& value) const
{
    for (const auto& function : m_evaluate_functions)
    {
        if (function(context, key, value))
        {
            return true;
        }
    }

    return false;
}

void ExpressionSession::add_skip_prim_type(const PXR_NS::TfType& skip_prim_type)
{
    m_skip_prim_type.push_back(skip_prim_type);
}

void ExpressionSession::add_skip_API_type(const PXR_NS::TfType& type)
{
    m_skip_API_type.emplace_back(type);
}

bool ExpressionSession::need_skip(const PXR_NS::UsdPrim& prim) const
{
    for (const auto& skip : m_skip_prim_type)
    {
        if (prim.IsA(skip))
        {
            return true;
        }
    }

    for (const auto& api : m_skip_API_type)
    {
        if (prim.HasAPI(api))
        {
            return true;
        }
    }

    return false;
}

bool ExpressionSession::is_enabled() const
{
    return m_is_enable;
}

ExpressionSession::ExpressionSession()
{
    m_application_event_handles[Application::EventType::SELECTION_CHANGED] =
        Application::instance().register_event_callback(Application::EventType::SELECTION_CHANGED, [this] { selection_changed(); });
    m_application_event_handles[Application::EventType::SESSION_STAGE_LIST_CHANGED] = Application::instance().register_event_callback(
        Application::EventType::SESSION_STAGE_LIST_CHANGED, [this]() { session_stage_list_changed(); });
    m_application_event_handles[Application::EventType::CURRENT_STAGE_CHANGED] =
        Application::instance().register_event_callback(Application::EventType::CURRENT_STAGE_CHANGED, [this]() { current_stage_changed(); });
}

ExpressionSession::~ExpressionSession()
{
    for (auto& it : m_application_event_handles)
        Application::instance().unregister_event_callback(it.first, it.second);
}

void ExpressionSession::current_stage_changed() {}

void ExpressionSession::session_stage_list_changed()
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
        create_engine_for_current_stage();
}

void ExpressionSession::selection_changed() {}

OPENDCC_NAMESPACE_CLOSE
