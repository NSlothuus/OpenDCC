// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "set_attr.h"
#include <pxr/base/tf/refPtr.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/meshSchema.h>
#include <pxr/imaging/hd/collectionExpressionEvaluator.h>
#include "opendcc/base/utils/string_utils.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    std::tuple<SdfPathExpression, TfToken, VtValue> get_scene_index_args(const UsdHydraOpSetAttribute& set_attr)
    {
        SdfPathExpression apply_to;
        TfToken attr_name;
        TfToken attr_type;
        set_attr.GetInputsApplyToAttr().Get(&apply_to);
        set_attr.GetInputsAttrNameAttr().Get(&attr_name);
        set_attr.GetInputsAttrTypeAttr().Get(&attr_type);

        VtValue set_val;
        if (attr_type == UsdHydraOpTokens->int_)
        {
            set_attr.GetInputsValue_intAttr().Get(&set_val);
        }
        else if (attr_type == UsdHydraOpTokens->float_)
        {
            set_attr.GetInputsValue_floatAttr().Get(&set_val);
        }
        else if (attr_type == UsdHydraOpTokens->double_)
        {
            set_attr.GetInputsValue_doubleAttr().Get(&set_val);
        }
        else if (attr_type == UsdHydraOpTokens->string)
        {
            set_attr.GetInputsValue_stringAttr().Get(&set_val);
        }
        return { apply_to, attr_name, set_val };
    }
}

TfRefPtr<SetAttrSceneIndex> SetAttrSceneIndex::New(const HdSceneIndexBaseRefPtr& inputSceneIndex, const SdfPathExpression& path, const TfToken& attr,
                                                   const VtValue& val)
{
    return TfCreateRefPtr(new SetAttrSceneIndex(inputSceneIndex, path, attr, val));
}

HdSceneIndexPrim SetAttrSceneIndex::GetPrim(const SdfPath& prim_path) const
{
    HdSceneIndexPrim prim;

    if (HdSceneIndexBaseRefPtr input = _GetInputSceneIndex())
    {
        prim = input->GetPrim(prim_path);
    }
    else
    {
        return prim;
    }

    SdfPathExpressionEval<const SdfPath&> eval = SdfMakePathExpressionEval<const SdfPath&>(m_path_expression, SdfPredicateLibrary<const SdfPath&>());
    if (eval.Match(prim_path, [](const SdfPath& p) { return p; }))
    {
        if (!prim.dataSource)
        {
            return prim;
        }

        prim.dataSource =
            HdOverlayContainerDataSource::New(HdRetainedContainerDataSource::New(m_attr, HdCreateTypedRetainedDataSource(m_value)), prim.dataSource);
    }

    return prim;
}

SdfPathVector SetAttrSceneIndex::GetChildPrimPaths(const SdfPath& primPath) const
{
    if (HdSceneIndexBaseRefPtr input = _GetInputSceneIndex())
    {
        return input->GetChildPrimPaths(primPath);
    }
    return {};
}

void SetAttrSceneIndex::set_args(const PXR_NS::SdfPathExpression& prim_path, const PXR_NS::TfToken& attr, const PXR_NS::VtValue& val)
{
    HdSceneIndexObserver::DirtiedPrimEntries dirties;
    if (!m_path_expression.IsEmpty())
    {
        SdfPathVector dirty_paths;
        auto eval = HdCollectionExpressionEvaluator(_GetInputSceneIndex(), m_path_expression);
        eval.PopulateAllMatches(SdfPath::AbsoluteRootPath(), &dirty_paths);

        // old
        std::transform(dirty_paths.begin(), dirty_paths.end(), std::back_inserter(dirties),
                       [this](const auto& p) { return HdSceneIndexObserver::DirtiedPrimEntry { p, HdDataSourceLocator(m_attr) }; });
    }

    if (!prim_path.IsEmpty())
    {
        SdfPathVector dirty_paths;
        auto eval = HdCollectionExpressionEvaluator(_GetInputSceneIndex(), prim_path);
        eval.PopulateAllMatches(SdfPath::AbsoluteRootPath(), &dirty_paths);

        // new
        std::transform(dirty_paths.begin(), dirty_paths.end(), std::back_inserter(dirties),
                       [this](const auto& p) { return HdSceneIndexObserver::DirtiedPrimEntry { p, HdDataSourceLocator(m_attr) }; });
    }

    m_path_expression = prim_path;
    m_attr = attr;
    m_value = val;
    _SendPrimsDirtied(dirties);
}
const PXR_NS::VtValue& SetAttrSceneIndex::get_value() const
{
    return m_value;
}
const PXR_NS::TfToken& SetAttrSceneIndex::get_attr_name() const
{
    return m_attr;
}
const PXR_NS::SdfPathExpression& SetAttrSceneIndex::get_path_expression() const
{
    return m_path_expression;
}
void SetAttrSceneIndex::set_path_expression(const PXR_NS::SdfPathExpression& path_expr)
{
    set_args(path_expr, m_attr, m_value);
}

void SetAttrSceneIndex::set_attr_name(const PXR_NS::TfToken& attr_name)
{
    set_args(m_path_expression, attr_name, m_value);
}

void SetAttrSceneIndex::set_value(const PXR_NS::VtValue& value)
{
    set_args(m_path_expression, m_attr, value);
}

void SetAttrSceneIndex::_PrimsAdded(const HdSceneIndexBase& sender, const HdSceneIndexObserver::AddedPrimEntries& entries)
{
    _SendPrimsAdded(entries);
}
void SetAttrSceneIndex::_PrimsRemoved(const HdSceneIndexBase& sender, const HdSceneIndexObserver::RemovedPrimEntries& entries)
{
    _SendPrimsRemoved(entries);
}
void SetAttrSceneIndex::_PrimsDirtied(const HdSceneIndexBase& sender, const HdSceneIndexObserver::DirtiedPrimEntries& entries)
{
    _SendPrimsDirtied(entries);
}

SetAttrSceneIndex::SetAttrSceneIndex(const HdSceneIndexBaseRefPtr& inputSceneIndex, const SdfPathExpression& prim_path, const TfToken& attr,
                                     const VtValue& val)
    : HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
    , m_path_expression(prim_path)
    , m_attr(attr)
    , m_value(val)
{
}

SetAttrTranslator::DirtyTypeFlags SetAttrTranslator::get_dirty_flags_impl(const PXR_NS::UsdHydraOpSetAttribute& prim,
                                                                          const PXR_NS::TfToken& property_name)
{
    DirtyTypeFlags result { DirtyType::Clean };

    if (property_name == UsdHydraOpTokens->inputsIn)
    {
        result |= DirtyType::DirtyInput;
    }

    if (property_name == UsdHydraOpTokens->inputsApplyTo || property_name == UsdHydraOpTokens->inputsAttrType ||
        property_name == UsdHydraOpTokens->inputsValue_int || property_name == UsdHydraOpTokens->inputsValue_float ||
        property_name == UsdHydraOpTokens->inputsValue_double || property_name == UsdHydraOpTokens->inputsValue_string)
    {
        result |= DirtyType::DirtyArgs;
    }
    return result;
}

PXR_NS::HdSceneIndexBaseRefPtr SetAttrTranslator::populate_impl(const PXR_NS::UsdHydraOpSetAttribute& prim,
                                                                const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& inputs)
{
    auto [expr, attr, val] = get_scene_index_args(prim);
    return SetAttrSceneIndex::New(inputs.empty() ? nullptr : inputs[0], expr, attr, val);
}

void SetAttrTranslator::process_args_change_impl(const PXR_NS::UsdHydraOpSetAttribute& prim, const PXR_NS::TfTokenVector& property_names,
                                                 const PXR_NS::HdSceneIndexBaseRefPtr& scene_index)
{
    auto set_attr_index = TfDynamic_cast<TfRefPtr<SetAttrSceneIndex>>(scene_index);
    if (!set_attr_index)
    {
        return;
    }

    TfToken attr_type;
    prim.GetInputsAttrTypeAttr().Get(&attr_type);
    const auto value_attr_name = TfToken("inputs:value_" + attr_type.GetString());
    auto cur_expr = set_attr_index->get_path_expression();
    auto cur_attr = set_attr_index->get_attr_name();
    auto cur_val = set_attr_index->get_value();
    bool changed = false;
    for (const auto& name : property_names)
    {
        if (name == UsdHydraOpTokens->inputsApplyTo)
        {
            SdfPathExpression expr;
            prim.GetInputsApplyToAttr().Get(&expr);
            cur_expr = expr;
            changed = true;
        }
        else if (name == UsdHydraOpTokens->inputsAttrName)
        {
            TfToken attr_name;
            prim.GetInputsAttrNameAttr().Get(&attr_name);
            cur_attr = attr_name;
            changed = true;
        }
        else if (name == UsdHydraOpTokens->inputsAttrType || name == value_attr_name)
        {
            VtValue set_val;
            auto attr = prim.GetPrim().GetAttribute(value_attr_name);
            attr.Get(&set_val);
            cur_val = set_val;
            changed = true;
        }
    }

    if (changed)
        set_attr_index->set_args(cur_expr, cur_attr, cur_val);
}

OPENDCC_NAMESPACE_CLOSE
