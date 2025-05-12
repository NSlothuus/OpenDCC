// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/editContext.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include "engine.h"
#include "opendcc/app/core/undo/stack.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"
#include "opendcc/base/logging/logger.h"
#include "opendcc/app/core/undo/block.h"
#include "expression_factory.h"
#include <opendcc/expression/usd_schema/tokens.h>
#include <iomanip>
#include <atomic>
#include "pxr/imaging/hd/primGather.h"

#include "session.h"

PXR_NAMESPACE_USING_DIRECTIVE

OPENDCC_NAMESPACE_OPEN

const char* ExpressionEngine::logger_channel = "ExpressionEngine";

namespace
{
    static ExpressionEngine::CallbackId generate_unique_id()
    {
        static std::atomic<uint64_t> id { 1 };
        return id.fetch_add(1);
    }

    class Scope
    {
    public:
        Scope(std::function<void()> do_it)
            : _do_it(do_it)
        {
        }
        ~Scope() { _do_it(); }

    private:
        std::function<void()> _do_it;
    };
}

void ExpressionEngine::ExpressionsContainer::remove(SdfPath path)
{
    auto it = m_expressions.find(path);
    if (it != m_expressions.end())
    {
        m_expressions.erase(path);
        m_sorted_paths.Remove(path);
    }
}

ExpressionEngine::EngineExpressionPtr ExpressionEngine::ExpressionsContainer::create(SdfPath path, IExpressionPtr expression, const TfToken& type,
                                                                                     const std::string& expression_str)
{
    auto& engine_expression = m_expressions[path];

    engine_expression = std::make_shared<EngineExpression>();
    engine_expression->expression = expression;
    m_sorted_paths.Insert(path);
    return engine_expression;
}

ExpressionEngine::CallbackId ExpressionEngine::invalid_callback_id()
{
    return (uint64_t)-1;
}

ExpressionEngine::ExpressionEngine(UsdStageRefPtr stage)
    : m_stage(stage)
{
    m_application_event_handles[Application::EventType::CURRENT_TIME_CHANGED] = Application::instance().register_event_callback(
        Application::EventType::CURRENT_TIME_CHANGED, [this]() { on_changed(m_data.expressions(), true); });

    for (UsdPrim prim : m_stage->Traverse())
    {
        for (UsdAttribute& attr : prim.GetAttributes())
        {
            if (attr.HasMetadata(UsdExpressionTokens->expression_type) && attr.HasMetadata(UsdExpressionTokens->expression_string))
            {
                TfToken type;
                std::string expression_str;
                bool type_ok = attr.GetMetadata(UsdExpressionTokens->expression_type, &type);
                bool string_ok = attr.GetMetadata(UsdExpressionTokens->expression_string, &expression_str);
                if (type_ok && string_ok)
                {
                    set_expression(attr, type, expression_str, false);
                }
                else
                {
                    OPENDCC_ERROR("invalid metadata in attribute {}", attr.GetPath().GetText());
                }
            }
        }
    }
    double time = Application::instance().get_current_time();
    update_time_variables(time);
    on_changed(m_data.expressions(), false);
    m_objects_changed_notice_key = TfNotice::Register(TfCreateWeakPtr(this), &ExpressionEngine::on_objects_changed, m_stage);
}

ExpressionEngine::ExpressionEngine() {}

ExpressionEngine::ExpressionEngine(const ExpressionEngine& other)
    : ExpressionEngine(other.m_stage)
{
}

ExpressionEngine::~ExpressionEngine()
{
    for (auto& it : m_application_event_handles)
        Application::instance().unregister_event_callback(it.first, it.second);
}

bool ExpressionEngine::set_expression(UsdAttribute attr, TfToken type, std::string expression_str, bool update_attr_value)
{
    if (!attr)
    {
        OPENDCC_ERROR("Not valid attribute: {}", attr.GetPath().GetText());
        return false;
    }
    IExpressionPtr new_expression = ExpressionFactory::instance().create_expression(type, attr.GetTypeName(), expression_str);
    if (!new_expression)
        return false;

    MuteScope scope(this);
    auto new_engine_expression = m_data.create(attr.GetPath(), new_expression, type, expression_str);

    attr.SetMetadata(UsdExpressionTokens->expression_type, VtValue(type));
    attr.SetMetadata(UsdExpressionTokens->expression_string, VtValue(expression_str));
    if (update_attr_value)
        on_changed(ExpressionsMap { { attr.GetPath(), new_engine_expression } }, false);

    return true;
}

bool ExpressionEngine::set_expression(UsdAttribute attr) // then metadates is exist
{
    if (!attr)
        return false;

    TfToken type;
    std::string expression_str;
    bool type_exist = attr.GetMetadata(UsdExpressionTokens->expression_type, &type);
    bool string_exist = attr.GetMetadata(UsdExpressionTokens->expression_string, &expression_str);
    if (type_exist && string_exist)
    {
        IExpressionPtr new_expression = ExpressionFactory::instance().create_expression(type, attr.GetTypeName(), expression_str);
        if (!new_expression)
            return false;

        auto new_engine_expression = m_data.create(attr.GetPath(), new_expression, type, expression_str);
        on_changed(ExpressionsMap { { attr.GetPath(), new_engine_expression } }, false);
    }

    return true;
}

bool ExpressionEngine::remove_expression(UsdAttribute attr)
{
    return remove_expression(attr.GetPath());
}

bool ExpressionEngine::remove_expression(SdfPath attr_path)
{
    auto it = m_data.expressions().find(attr_path);
    if (it == m_data.expressions().end())
        return false;
    MuteScope scope(this);
    Scope end([&]() { m_data.remove(attr_path); });

    for (auto it : it->second->callbacks)
        m_callback_id_to_attr_path.erase(it.first);

    auto prim = m_stage->GetPrimAtPath(attr_path.GetAbsoluteRootOrPrimPath());
    if (!prim)
        return false;
    UsdAttribute attr = prim.GetAttribute(attr_path.GetNameToken());
    if (!attr.IsValid())
        return false;
    attr.ClearMetadata(UsdExpressionTokens->expression_type);
    attr.ClearMetadata(UsdExpressionTokens->expression_string);

    auto session_layer = m_stage->GetSessionLayer();
    auto session_layer_prim_spec = session_layer->GetPrimAtPath(prim.GetPath());
    if (session_layer_prim_spec && session_layer_prim_spec->GetSpecifier() == SdfSpecifier::SdfSpecifierOver)
    {
        for (auto& prop : attr.GetPropertyStack())
            session_layer_prim_spec->RemoveProperty(prop);
        session_layer->RemovePrimIfInert(session_layer_prim_spec);
    }

    return true;
}

bool ExpressionEngine::has_expression(SdfPath attr_path) const
{
    return m_data.expressions().find(attr_path) != m_data.expressions().end();
}

ExpressionEngine::CallbackId ExpressionEngine::register_expression_callback(SdfPath attr_path, ExpressionCallback callback)
{
    auto it = m_data.expressions().find(attr_path);
    if (it == m_data.expressions().end())
        return invalid_callback_id();

    auto id = generate_unique_id();
    it->second->callbacks[id] = callback;
    m_callback_id_to_attr_path[id] = attr_path;
    return id;
}

bool ExpressionEngine::unregister_callback(CallbackId id)
{
    auto attr_it = m_callback_id_to_attr_path.find(id);
    if (attr_it != m_callback_id_to_attr_path.end())
    {
        auto it = m_data.expressions().find(attr_it->second);
        if (it != m_data.expressions().end())
            it->second->callbacks.erase(id);

        m_callback_id_to_attr_path.erase(id);
        return true;
    }

    auto value_it = m_callback_id_to_variable_name.find(id);
    if (value_it != m_callback_id_to_variable_name.end())
    {
        auto it = m_variablse_callback_map.find(value_it->second);
        if (it != m_variablse_callback_map.end())
            it->second.erase(id);

        m_callback_id_to_variable_name.erase(id);
        return true;
    }
    return false;
}

ExpressionEngine::CallbackId ExpressionEngine::register_variable_changed_callback(std::string variable_name, VariableCallback callback)
{
    if (m_context.variables.find(variable_name) == m_context.variables.end())
        return invalid_callback_id();
    VariableCallbacksMap& callbacks_map = m_variablse_callback_map[variable_name];

    auto id = generate_unique_id();
    callbacks_map[id] = callback;
    m_callback_id_to_variable_name[id] = variable_name;
    return id;
}

bool ExpressionEngine::bake_all(SdfLayerRefPtr layer, double start_frame, double end_frame, const std::vector<double>& frame_samples,
                                bool remove_origin)
{
    SdfPathVector attrs_paths;
    for (auto it : m_data.expressions())
        attrs_paths.push_back(it.first);

    return bake(layer, attrs_paths, start_frame, end_frame, frame_samples, remove_origin);
}

bool ExpressionEngine::bake(SdfLayerRefPtr layer, const SdfPathVector& attrs_paths, double start_frame, double end_frame,
                            const std::vector<double>& frame_samples, bool remove_origin)
{
    MuteScope scope(this);
    std::map<SdfPath, UsdAttribute> attributes_map;
    for (auto path : attrs_paths)
    {
        if (has_expression(path))
        {
            if (auto prim = m_stage->GetPrimAtPath(path.GetAbsoluteRootOrPrimPath()))
            {
                if (UsdAttribute attr = prim.GetAttribute(path.GetNameToken()))
                    attributes_map[path] = attr;
            }
        }
    }

    if (attributes_map.empty())
        return true;

    commands::UsdEditsUndoBlock block;
    {
        { // context
            UsdEditContext context(m_stage, layer);

            for (double frame = start_frame; frame < end_frame + 1e-3; frame += 1.0)
            {
                for (double sample : frame_samples)
                {
                    double time = frame + sample;
                    update_time_variables(time);
                    for (auto it : attributes_map)
                    {
                        bool success;
                        m_context.attribute_path = it.first;
                        m_context.frame = time;
                        VtValue vt_value = m_data.expressions().at(it.first)->expression->evaluate(m_context, success);

                        if (success && !vt_value.IsEmpty())
                            it.second.Set(vt_value, UsdTimeCode(time));
                    }
                }
            }
        } // end layer context

#ifndef OPENDCC_EXPRESSIONS_USE_COMPUTE_GRAPH
        if (remove_origin && (layer != m_stage->GetSessionLayer()))
        {
            UsdEditContext context(m_stage, m_stage->GetSessionLayer());
            for (auto it : attributes_map)
            {
                it.second.GetPrim().RemoveProperty(it.second.GetName());
            }
        }
#endif
    } // end usd undo block

    if (remove_origin)
    {
        for (auto& attr : attrs_paths)
        {
            remove_expression(attr);
        }
    }

    m_context.frame = 0;
    m_context.attribute_path = SdfPath::EmptyPath();

    return true;
}

void ExpressionEngine::set_variable(const std::string& name, VtValue value)
{
    m_context.variables[name] = value;
    on_variable_changed(name, VtValue(value));
    on_changed(m_data.expressions(), false);
}

VtValue ExpressionEngine::get_variable(const std::string& name) const
{
    auto it = m_context.variables.find(name);
    if (it != m_context.variables.end())
        return it->second;
    else
        return VtValue();
}

std::vector<std::string> ExpressionEngine::get_variables_list() const
{
    std::vector<std::string> result;
    for (auto it : m_context.variables)
        result.push_back(it.first);
    return result;
}

void ExpressionEngine::erase_variable(const std::string& name)
{
    m_context.variables.erase(name);
    auto variablse_callback_it = m_variablse_callback_map.find(name);
    if (variablse_callback_it != m_variablse_callback_map.end())
    {
        for (auto& it : variablse_callback_it->second)
            m_callback_id_to_variable_name.erase(it.first);
    }
    on_changed(m_data.expressions(), false);
}

PXR_NS::VtValue ExpressionEngine::evaluate_get(const PXR_NS::UsdAttribute& attribute, const double time)
{
    const auto& expressions = m_data.expressions();

    const auto path = attribute.GetPath();

    const auto find = expressions.find(path);

    if (find == expressions.end())
    {
        return PXR_NS::VtValue();
    }

    bool success;
    m_context.attribute_path = path;
    m_context.frame = time;
    VtValue vt_value = find->second->expression->evaluate(m_context, success);

    return success ? vt_value : PXR_NS::VtValue();
}

VtValue ExpressionEngine::evaluate(const SdfPath& attribute_path, UsdTimeCode time /*= PXR_NS::UsdTimeCode::Default()*/)
{
    return evaluate_get(attribute_path, time.GetValue());
}

VtValue ExpressionEngine::evaluate_get(const SdfPath& attribute_path, const double time /*= 0*/)
{
    if (auto attr = m_stage->GetAttributeAtPath(attribute_path))
    {
        return evaluate_get(attr, time);
    }

    return VtValue();
}

bool ExpressionEngine::evaluate_set(const PXR_NS::UsdAttribute& attribute, const double time /* = 0 */)
{

    VtValue vt_value = evaluate_get(attribute, time);
    auto layer = m_stage->GetSessionLayer();
    if (!layer)
        return false;
    MuteScope scope(this);
    UsdEditContext context(m_stage, layer);

    SdfChangeBlock change_block;

    if (!vt_value.IsEmpty())
    {
        const auto expressions = m_data.expressions();
        const auto find = expressions.find(attribute.GetPath());

        for (auto& callback : find->second->callbacks)
            callback.second(find->first, vt_value);

        return true;
    }

    return false;
}

bool ExpressionEngine::evaluate_all(const PXR_NS::UsdPrim& prim, const double time)
{
    for (UsdAttribute& attr : prim.GetAttributes())
    {
        if (attr.HasMetadata(UsdExpressionTokens->expression_type) && attr.HasMetadata(UsdExpressionTokens->expression_string))
        {
            if (!evaluate_set(attr, time))
            {
                OPENDCC_ERROR("can't evaluate UsdAttribute: {}", attr.GetName().GetText());
                return false;
            }
        }
    }

    return true;
}

void ExpressionEngine::update_time_variables(double time)
{
    const size_t n_zero = 4;
    std::ostringstream stream_F;
    stream_F << std::fixed << std::setprecision(0) << time;
    auto str = stream_F.str();

    static auto env_value = getenv("OPENDCC_EXPRESSION_F_VARIABLE_OLD_MODE");
    if (env_value && (std::string(env_value) == "1" || std::string(env_value) == "ON" || std::string(env_value) == "on"))
    {
        auto padding_str = std::string(n_zero - std::min(n_zero, str.length()), '0') + str;
        m_context.variables[std::string("F")] = padding_str;
        on_variable_changed("F", VtValue(padding_str));
    }
    else
    {
        m_context.variables[std::string("F")] = str;
        on_variable_changed("F", VtValue(str));
    }

    std::ostringstream stream_FF;
    stream_FF << std::fixed << std::setprecision(2) << time;
    str = stream_FF.str();
    auto ff_padding_str = std::string(n_zero + 3 - std::min(n_zero + 3, str.length()), '0') + str;
    m_context.variables[std::string("FF")] = ff_padding_str;
    on_variable_changed("FF", VtValue(ff_padding_str));
}

void ExpressionEngine::on_objects_changed(UsdNotice::ObjectsChanged const& notice, UsdStageWeakPtr const& sender)
{
    using PathRange = UsdNotice::ObjectsChanged::PathRange;
    const PathRange paths_to_resync = notice.GetResyncedPaths();
    const PathRange paths_to_update = notice.GetChangedInfoOnlyPaths();

    if (m_mute_recursion_depth > 0)
        return;

    MuteScope scope(this);

    HdPrimGather gather;
    SdfPathVector expressions_paths;
    std::unordered_set<SdfPath, SdfPath::Hash> paths_to_remove;
    std::unordered_map<SdfPath, UsdAttribute, SdfPath::Hash> paths_to_create;

    auto check_attr = [&](const UsdAttribute& attr) {
        if (attr && attr.HasMetadata(UsdExpressionTokens->expression_type) && attr.HasMetadata(UsdExpressionTokens->expression_string))
            paths_to_create[attr.GetPath()] = attr;
        else if (expressions_paths.size() > 0 && m_data.expressions().find(attr.GetPath()) != m_data.expressions().end())
            paths_to_remove.insert(attr.GetPath());
    };

    auto traverse = [&](const SdfPath& path) {
        gather.Subtree(m_data.sorted_paths(), path.GetPrimPath(), &expressions_paths);
        auto prim = m_stage->GetPrimAtPath(path.GetAbsoluteRootOrPrimPath());
        if (prim)
        {
            if (path.IsPrimPath())
            {
                UsdPrimRange range(prim);
                if (!range)
                {
                    auto session_prim = m_stage->GetSessionLayer()->GetPrimAtPath(path);
                    if (session_prim && session_prim->GetSpecifier() == SdfSpecifierOver)
                    {
                        for (auto& path : expressions_paths)
                            paths_to_remove.insert(path);
                    }
                }

                for (auto sub_path = range.begin(); sub_path != range.end(); ++sub_path)
                {
                    UsdPrim prim = m_stage->GetPrimAtPath(sub_path->GetPath());
                    if (prim)
                    {
                        auto attrs = prim.GetAttributes();
                        for (auto& attr : attrs)
                            check_attr(attr);
                    }
                }
            }
            else
            {
                auto attr = prim.GetAttribute(path.GetNameToken());
                check_attr(attr);
            }
        }
        else
        {
            for (auto& path : expressions_paths)
                paths_to_remove.insert(path);
        }
    };

    for (PathRange::const_iterator path = paths_to_resync.begin(); path != paths_to_resync.end(); ++path)
    {
        traverse(*path);
    }

    for (PathRange::const_iterator path = paths_to_update.begin(); path != paths_to_update.end(); ++path)
    {
        traverse(*path);
    }

    for (auto path : paths_to_remove)
    {
        remove_expression(path);
    }
    for (auto it : paths_to_create)
    {
        set_expression(it.second);
    }

}

void ExpressionEngine::on_variable_changed(const std::string& variable_name, VtValue value)
{
    auto it = m_variablse_callback_map.find(variable_name);
    if (it == m_variablse_callback_map.end())
        return;
    for (auto callback_it : it->second)
        callback_it.second(variable_name, value);
}

void ExpressionEngine::on_changed(const ExpressionsMap& expressions, bool do_update_time_variables)
{
    if (expressions.empty())
        return;

    auto layer = m_stage->GetSessionLayer();
    if (!layer)
        return;

    MuteScope scope(this);

    UsdEditContext context(m_stage, layer);

    SdfChangeBlock change_block;
    double time = Application::instance().get_current_time();
    if (do_update_time_variables)
        update_time_variables(time);
    SdfPathVector expressions_to_remove;

    const auto& session = ExpressionSession::instance();

    for (auto it : expressions)
    {
        auto prim = m_stage->GetPrimAtPath(it.first.GetAbsoluteRootOrPrimPath());
        if (!prim)
        {
            expressions_to_remove.push_back(it.first);
            continue;
        }

        UsdAttribute attr = prim.GetAttribute(it.first.GetNameToken());
        if (!attr)
        {
            expressions_to_remove.push_back(it.first);
            continue;
        }

        bool success;
        m_context.attribute_path = it.first;

        if (session.need_skip(prim))
        {
            continue;
        }

        m_context.frame = time;

        auto vt_value = it.second->expression->evaluate(m_context, success);

        if (success && !vt_value.IsEmpty())
        {
            for (auto& callback : it.second->callbacks)
                callback.second(it.first, vt_value);

            attr.Set(vt_value);
        }
    }

    for (auto& path : expressions_to_remove)
        remove_expression(path);

    m_context.frame = 0;
    m_context.attribute_path = PXR_NS::SdfPath::EmptyPath();
}

OPENDCC_NAMESPACE_CLOSE
