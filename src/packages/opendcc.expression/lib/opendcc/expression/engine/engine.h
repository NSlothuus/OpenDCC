/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "pxr/usd/usd/attribute.h"
#include "pxr/usd/sdf/layer.h"
#include "pxr/usd/usd/notice.h"
#include <atomic>
#include <functional>
#include <memory>
#include <set>
#include <unordered_map>
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"
#include "api.h"
#include "opendcc/app/core/application.h"
#include "pxr/usd/usd/stage.h"
#include "iexpression.h"

OPENDCC_NAMESPACE_OPEN

class ExpressionEngine : public PXR_NS::TfWeakBase
{
public:
    using CallbackId = uint32_t;
    using ExpressionCallback = std::function<void(PXR_NS::SdfPath path, const PXR_NS::VtValue& expression_result)>;
    using VariableCallback = std::function<void(const std::string& variable_name, const PXR_NS::VtValue& value)>;

    static const char* logger_channel;
    EXPRESSION_ENGINE_API static CallbackId invalid_callback_id();
    EXPRESSION_ENGINE_API void activate() {};
    EXPRESSION_ENGINE_API void deactivate() {};
    EXPRESSION_ENGINE_API ExpressionEngine();
    EXPRESSION_ENGINE_API ExpressionEngine(PXR_NS::UsdStageRefPtr stage);
    EXPRESSION_ENGINE_API ExpressionEngine(const ExpressionEngine&);
    EXPRESSION_ENGINE_API ~ExpressionEngine();

    EXPRESSION_ENGINE_API bool set_expression(PXR_NS::UsdAttribute attr);
    EXPRESSION_ENGINE_API bool set_expression(PXR_NS::UsdAttribute attr, PXR_NS::TfToken type, std::string expression, bool update_attr_value);
    EXPRESSION_ENGINE_API bool remove_expression(PXR_NS::UsdAttribute attr);
    EXPRESSION_ENGINE_API bool remove_expression(PXR_NS::SdfPath attr_path);
    EXPRESSION_ENGINE_API bool has_expression(PXR_NS::SdfPath attr_path) const;

    EXPRESSION_ENGINE_API CallbackId register_expression_callback(PXR_NS::SdfPath attr_path, ExpressionCallback callback);
    EXPRESSION_ENGINE_API CallbackId register_variable_changed_callback(std::string variable_name, VariableCallback callback);
    EXPRESSION_ENGINE_API bool unregister_callback(CallbackId id);

    EXPRESSION_ENGINE_API bool bake(PXR_NS::SdfLayerRefPtr layer, const PXR_NS::SdfPathVector& attrs_paths, double start_frame, double end_frame,
                                    const std::vector<double>& frame_samples, bool remove_origin);
    EXPRESSION_ENGINE_API bool bake_all(PXR_NS::SdfLayerRefPtr layer, double start_frame, double end_frame, const std::vector<double>& frame_samples,
                                        bool remove_origin);

    EXPRESSION_ENGINE_API void set_variable(const std::string& name, PXR_NS::VtValue value);
    EXPRESSION_ENGINE_API PXR_NS::VtValue get_variable(const std::string& name) const;
    EXPRESSION_ENGINE_API std::vector<std::string> get_variables_list() const;
    EXPRESSION_ENGINE_API void erase_variable(const std::string& name);

    PXR_NS::VtValue evaluate_get(const PXR_NS::UsdAttribute& attribute, const double time = 0);
    PXR_NS::VtValue evaluate_get(const PXR_NS::SdfPath& attribute_path, const double time = 0);

    EXPRESSION_ENGINE_API PXR_NS::VtValue evaluate(const PXR_NS::SdfPath& attribute_path, PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default());
    EXPRESSION_ENGINE_API bool evaluate_set(const PXR_NS::UsdAttribute& attribute, const double time = 0);
    EXPRESSION_ENGINE_API bool evaluate_all(const PXR_NS::UsdPrim& prim, const double time = 0);

private:
    struct EngineExpression
    {
        IExpressionPtr expression;
        std::map<CallbackId, ExpressionCallback> callbacks;
    };

    using EngineExpressionPtr = std::shared_ptr<EngineExpression>;
    using ExpressionsMap = std::unordered_map<PXR_NS::SdfPath, EngineExpressionPtr, PXR_NS::SdfPath::Hash>;
    using VariableCallbacksMap = std::unordered_map<CallbackId, VariableCallback>;

    class ExpressionsContainer // added for data consistencies guaranty
    {
    public:
        const ExpressionsMap& expressions() const { return m_expressions; }
        EngineExpressionPtr create(PXR_NS::SdfPath path, IExpressionPtr expression, const PXR_NS::TfToken& type, const std::string& expression_str);
        void remove(PXR_NS::SdfPath path);
        const PXR_NS::SdfPathVector& sorted_paths() const { return m_sorted_paths.GetIds(); }

    private:
        ExpressionsMap m_expressions;
        mutable PXR_NS::Hd_SortedIds m_sorted_paths;
    };

    class MuteScope
    {
    public:
        MuteScope(ExpressionEngine* self)
            : m_self(self)
        {
            ++m_self->m_mute_recursion_depth;
        }
        ~MuteScope() { --m_self->m_mute_recursion_depth; }

    private:
        ExpressionEngine* m_self = nullptr;
    };

    void on_changed(const ExpressionsMap& expressions, bool do_update_time_variables);
    void on_variable_changed(const std::string& variable_name, PXR_NS::VtValue value);
    void update_time_variables(double time);
    void on_objects_changed(PXR_NS::UsdNotice::ObjectsChanged const& notice, PXR_NS::UsdStageWeakPtr const& sender);

    PXR_NS::UsdStageRefPtr m_stage;
    PXR_NS::TfNotice::Key m_objects_changed_notice_key;
    std::map<Application::EventType, Application::CallbackHandle> m_application_event_handles;

    ExpressionContext m_context;
    std::unordered_map<CallbackId, PXR_NS::SdfPath> m_callback_id_to_attr_path;
    std::unordered_map<std::string, VariableCallbacksMap> m_variablse_callback_map;
    std::unordered_map<CallbackId, std::string> m_callback_id_to_variable_name;
    size_t m_mute_recursion_depth = 0;
    ExpressionsContainer m_data;
};

using ExpressionEnginePtr = std::shared_ptr<ExpressionEngine>;

OPENDCC_NAMESPACE_CLOSE
