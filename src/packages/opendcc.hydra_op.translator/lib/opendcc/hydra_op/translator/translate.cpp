// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/hydra_op/translator/translate.h"
#include "opendcc/hydra_op/schema/translateAPI.h"
#include <pxr/usdImaging/usdImaging/rerootingSceneIndex.h>
#include <pxr/imaging/hd/mergingSceneIndex.h>
#include <pxr/usdImaging/usdImaging/stageSceneIndex.h>
#include <opendcc/base/logging/logger.h>

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

TranslateAPITranslator::DirtyTypeFlags TranslateAPITranslator::get_dirty_flags_impl(const PXR_NS::UsdHydraOpTranslateAPI& prim,
                                                                                    const TfToken& property_name)
{
    DirtyTypeFlags result { DirtyType::Clean };
    if (property_name == UsdHydraOpTokens->hydraOpIn)
    {
        result |= DirtyType::DirtyInput;
    }
    if (property_name == UsdHydraOpTokens->hydraOpPath)
    {
        result |= DirtyType::DirtyNode;
    }

    return result;
}

HdSceneIndexBaseRefPtr TranslateAPITranslator::populate_impl(const PXR_NS::UsdHydraOpTranslateAPI& prim,
                                                             const std::vector<HdSceneIndexBaseRefPtr>& inputs)
{
    std::string hydra_op_path;
    prim.GetHydraOpPathAttr().Get(&hydra_op_path);
    UsdImagingCreateSceneIndicesInfo info;
    info.stage = prim.GetPrim().GetStage();
    info.overridesSceneIndexCallback = [usd_path = prim.GetPath(), hydra_op_path](const HdSceneIndexBaseRefPtr& input) {
        const auto dst_path = hydra_op_path.empty() ? usd_path : SdfPath(hydra_op_path);
        return UsdImagingRerootingSceneIndex::New(input, usd_path, dst_path);
    };

    m_indices = UsdImagingCreateSceneIndices(info);
    if (m_indices.stageSceneIndex)
    {
        m_indices.stageSceneIndex->SetTime(m_time);
    }
    if (inputs.empty())
    {
        return m_indices.finalSceneIndex;
    }

    auto merge = HdMergingSceneIndex::New();
    merge->AddInputScene(m_indices.finalSceneIndex, SdfPath::AbsoluteRootPath());
    for (const auto& inp : inputs)
    {
        merge->AddInputScene(inp, SdfPath::AbsoluteRootPath());
    }
    return merge;
}

void TranslateAPITranslator::process_args_change_impl(const PXR_NS::UsdHydraOpTranslateAPI& prim, const TfTokenVector& property_names,
                                                      const HdSceneIndexBaseRefPtr& scene_index)
{
}

void TranslateAPITranslator::on_time_changed_impl(const PXR_NS::UsdHydraOpTranslateAPI& prim, const PXR_NS::HdSceneIndexBaseRefPtr& scene_index,
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
