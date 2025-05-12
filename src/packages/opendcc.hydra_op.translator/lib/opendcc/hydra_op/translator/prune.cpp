// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "prune.h"
#include <pxr/base/tf/refPtr.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/meshSchema.h>
#include <pxr/imaging/hd/collectionExpressionEvaluator.h>
#include "opendcc/base/utils/string_utils.h"
#include "opendcc/usd_editor/scene_indices/prune_scene_index.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

PruneTranslator::DirtyTypeFlags PruneTranslator::get_dirty_flags_impl(const PXR_NS::UsdHydraOpPrune& prim, const PXR_NS::TfToken& property_name)
{
    DirtyTypeFlags result { DirtyType::Clean };
    if (property_name == UsdHydraOpTokens->inputsIn)
    {
        result |= DirtyType::DirtyInput;
    }
    else if (property_name == UsdHydraOpTokens->inputsApplyTo)
    {
        result |= DirtyType::DirtyArgs;
    }

    return result;
}

PXR_NS::HdSceneIndexBaseRefPtr PruneTranslator::populate_impl(const PXR_NS::UsdHydraOpPrune& prim,
                                                              const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& inputs)
{
    auto result = HdSceneIndexBaseRefPtr(new PruneSceneIndex((inputs.empty() ? nullptr : inputs[0])));
    process_args_change_impl(prim, { UsdHydraOpTokens->inputsApplyTo }, result);
    return result;
}

void PruneTranslator::process_args_change_impl(const PXR_NS::UsdHydraOpPrune& prim, const PXR_NS::TfTokenVector& property_names,
                                               const PXR_NS::HdSceneIndexBaseRefPtr& scene_index)
{
    auto prune_si = PXR_NS::TfDynamic_cast<TfRefPtr<PruneSceneIndex>>(scene_index);
    SdfPathExpression expr;
    if (prim.GetInputsApplyToAttr().Get(&expr))
    {
        auto eval = SdfMakePathExpressionEval<const SdfPath&>(expr, SdfPredicateLibrary<const SdfPath&>());
        prune_si->set_predicate([eval](const SdfPath& path) { return eval.Match(path, [](const SdfPath& p) { return p; }); });
    }
}

OPENDCC_NAMESPACE_CLOSE
