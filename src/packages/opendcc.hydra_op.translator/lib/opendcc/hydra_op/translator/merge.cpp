// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "merge.h"
#include <pxr/usdImaging/usdImaging/sceneIndices.h>
#include <pxr/usdImaging/usdImaging/stageSceneIndex.h>
#include <pxr/imaging/hd/mergingSceneIndex.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

HydraOpNodeTranslator::DirtyTypeFlags MergeTranslator::get_dirty_flags_impl(const PXR_NS::UsdHydraOpMerge& prim, const PXR_NS::TfToken& property_name)
{
    if (property_name == UsdHydraOpTokens->inputsIn)
    {
        return DirtyType::DirtyInput;
    }
    return DirtyType::Clean;
}

PXR_NS::HdSceneIndexBaseRefPtr MergeTranslator::populate_impl(const PXR_NS::UsdHydraOpMerge& prim,
                                                              const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& inputs)
{
    auto merging = HdMergingSceneIndex::New();
    for (const auto& in : inputs)
    {
        merging->AddInputScene(in, SdfPath::AbsoluteRootPath());
    }
    return merging;
}

void MergeTranslator::process_args_change_impl(const PXR_NS::UsdHydraOpMerge& prim, const PXR_NS::TfTokenVector& property_names,
                                               const PXR_NS::HdSceneIndexBaseRefPtr& scene_index)
{
}

OPENDCC_NAMESPACE_CLOSE
