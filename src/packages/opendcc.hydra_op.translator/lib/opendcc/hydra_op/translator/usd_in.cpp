// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "usd_in.h"
#include <pxr/usdImaging/usdImaging/sceneIndices.h>
#include <pxr/usdImaging/usdImaging/stageSceneIndex.h>
#include <opendcc/base/logging/logger.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

HydraOpNodeTranslator::DirtyTypeFlags UsdInTranslator::get_dirty_flags_impl(const PXR_NS::UsdHydraOpUsdIn& prim, const PXR_NS::TfToken& property_name)
{
    if (property_name == UsdHydraOpTokens->inputsFilePath)
    {
        return DirtyType::DirtyNode;
    }

    if (property_name == UsdHydraOpTokens->inputsRootPrim || property_name == UsdHydraOpTokens->inputsStagePrefix)
    {
        return DirtyType::DirtyArgs;
    }
    return DirtyType::Clean;
}

PXR_NS::HdSceneIndexBaseRefPtr UsdInTranslator::populate_impl(const PXR_NS::UsdHydraOpUsdIn& prim,
                                                              const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& inputs)
{
    SdfAssetPath asset;
    prim.GetInputsFilePathAttr().Get<SdfAssetPath>(&asset);
    UsdImagingCreateSceneIndicesInfo info;

    if (const auto asset_path = asset.GetAssetPath(); !asset_path.empty())
    {
        info.stage = UsdStage::Open(asset.GetAssetPath());
    }

    m_indices = UsdImagingCreateSceneIndices(info);
    if (m_indices.stageSceneIndex)
    {
        m_indices.stageSceneIndex->SetTime(m_time);
    }
    return m_indices.finalSceneIndex;
}

void UsdInTranslator::process_args_change_impl(const PXR_NS::UsdHydraOpUsdIn& prim, const PXR_NS::TfTokenVector& property_names,
                                               const PXR_NS::HdSceneIndexBaseRefPtr& scene_index)
{
    OPENDCC_ASSERT(m_indices.finalSceneIndex == scene_index);

    for (const auto& name : property_names)
    {
        // In theory, UsdImagingStageSceneIndex has method SetStage, but it is very unstable and can lead to crashes
        // So in case of inputsFilePath change it is safer to completely repopulate index chain
        /*if (name == UsdHydraOpTokens->inputsFilePath)
        {
            SdfAssetPath asset;
            prim.GetInputsFilePathAttr().Get<SdfAssetPath>(&asset);

            const auto& stage_path = asset.GetAssetPath();
            auto stage = stage_path.empty() ? nullptr : UsdStage::Open(asset.GetAssetPath());
            m_indices.stageSceneIndex->SetStage(stage);
        }*/
    }
}

void UsdInTranslator::on_time_changed_impl(const PXR_NS::UsdHydraOpUsdIn& prim, const PXR_NS::HdSceneIndexBaseRefPtr& scene_index,
                                           PXR_NS::UsdTimeCode time)
{
    OPENDCC_ASSERT(m_indices.finalSceneIndex == scene_index);
    m_time = time;
    if (m_indices.stageSceneIndex)
    {
        m_indices.stageSceneIndex->SetTime(m_time);
    }
}

OPENDCC_NAMESPACE_CLOSE
