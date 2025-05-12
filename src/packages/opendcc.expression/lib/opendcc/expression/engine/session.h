/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "api.h"
#include <pxr/usd/usd/stageCache.h>
#include "engine.h"
#include <pxr/base/vt/value.h>

OPENDCC_NAMESPACE_OPEN

class EXPRESSION_ENGINE_API ExpressionSession
{
public:
    using EvaluateFunction = std::function<bool(const ExpressionContext& context, const std::string& key, std::string& value)>;

    static ExpressionSession& instance();
    ExpressionEnginePtr current_engine();
    ExpressionEnginePtr engine(const PXR_NS::UsdStageCache::Id& stage_id);
    void create_engine_for_current_stage();
    bool is_enabled() const;
    void set_enabled(bool enable);

    void add_evaluate_function(const EvaluateFunction& evaluate_function);

    bool evaluate_string(const ExpressionContext& context, const std::string& key, std::string& value) const;

    void add_skip_prim_type(const PXR_NS::TfType& skip_prim_type);
    void add_skip_API_type(const PXR_NS::TfType& type);

    bool need_skip(const PXR_NS::UsdPrim& prim) const;

private:
    ExpressionSession();
    ~ExpressionSession();
    ExpressionSession(const ExpressionSession&) = delete;
    ExpressionSession(ExpressionSession&&) = delete;
    ExpressionSession& operator=(const ExpressionSession&) = delete;
    ExpressionSession& operator=(ExpressionSession&&) = delete;

    void current_stage_changed();
    void selection_changed();
    void session_stage_list_changed();
    std::unordered_map<long int, ExpressionEnginePtr> m_engines;
    std::map<Application::EventType, Application::CallbackHandle> m_application_event_handles;
    bool m_is_enable = false;

    std::vector<EvaluateFunction> m_evaluate_functions;
    std::vector<PXR_NS::TfType> m_skip_prim_type;
    std::vector<PXR_NS::TfType> m_skip_API_type;
};

OPENDCC_NAMESPACE_CLOSE
