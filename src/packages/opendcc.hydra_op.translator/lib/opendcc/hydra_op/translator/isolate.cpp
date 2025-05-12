// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "isolate.h"
#include <pxr/base/tf/refPtr.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/meshSchema.h>
#include <pxr/imaging/hd/collectionExpressionEvaluator.h>
#include "opendcc/base/utils/string_utils.h"
#include "opendcc/usd_editor/scene_indices/isolate_scene_index.h"
#include <opendcc/base/logging/logger.h>

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

IsolateTranslator::DirtyTypeFlags IsolateTranslator::get_dirty_flags_impl(const PXR_NS::UsdHydraOpIsolate& prim, const PXR_NS::TfToken& property_name)
{
    DirtyTypeFlags result { DirtyType::Clean };
    if (property_name == UsdHydraOpTokens->inputsIn)
    {
        result |= DirtyType::DirtyInput;
    }
    else if (property_name == UsdHydraOpTokens->inputsApplyTo || property_name == UsdHydraOpTokens->inputsIsolateFrom)
    {
        result |= DirtyType::DirtyArgs;
    }

    return result;
}

PXR_NS::HdSceneIndexBaseRefPtr IsolateTranslator::populate_impl(const PXR_NS::UsdHydraOpIsolate& prim,
                                                                const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& inputs)
{
    auto result = HdSceneIndexBaseRefPtr(new IsolateSceneIndex((inputs.empty() ? nullptr : inputs[0])));
    process_args_change_impl(prim, { UsdHydraOpTokens->inputsApplyTo, UsdHydraOpTokens->inputsIsolateFrom }, result);
    return result;
}

void IsolateTranslator::process_args_change_impl(const PXR_NS::UsdHydraOpIsolate& prim, const PXR_NS::TfTokenVector& property_names,
                                                 const PXR_NS::HdSceneIndexBaseRefPtr& scene_index)
{
    auto isolate_si = PXR_NS::TfDynamic_cast<TfRefPtr<IsolateSceneIndex>>(scene_index);
    SdfPathExpressionEval<const SdfPath&> apply_to_eval;

    SdfPathExpression apply_to_expr;
    if (prim.GetInputsApplyToAttr().Get(&apply_to_expr))
    {
        apply_to_eval = SdfMakePathExpressionEval<const SdfPath&>(apply_to_expr, SdfPredicateLibrary<const SdfPath&>());
    }
    std::string isolate_from_str;
    prim.GetInputsIsolateFromAttr().Get(&isolate_from_str);

    std::string error;
    if (!SdfPath::IsValidPathString(isolate_from_str, &error))
    {
        OPENDCC_ERROR("Failed to set valid 'inputs:isolateFrom' path on node '{}': expected empty, absolute root or prim path, got {}. {}",
                      prim.GetPath().GetString(), isolate_from_str, error);
        isolate_si->set_args(SdfPath::EmptyPath(), nullptr);
        return;
    }

    if (isolate_from_str.empty() || apply_to_eval.IsEmpty())
    {
        isolate_si->set_args(SdfPath(isolate_from_str), nullptr);
        return;
    }

    const auto isolate_from_path = SdfPath(isolate_from_str);
    isolate_si->set_args(isolate_from_path, [apply_to_eval, isolate_from_path](const SdfPath& path) {
        if (path.HasPrefix(isolate_from_path) && path != isolate_from_path)
        {
            return apply_to_eval.Match(path, [](const SdfPath& p) { return p; }).GetValue();
        }

        return false;
    });
}

OPENDCC_NAMESPACE_CLOSE
