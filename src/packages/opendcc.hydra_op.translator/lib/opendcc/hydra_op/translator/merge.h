/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include "opendcc/hydra_op/translator/network.h"
#include <pxr/usdImaging/usdImaging/sceneIndices.h>
#include "opendcc/hydra_op/schema/merge.h"

OPENDCC_NAMESPACE_OPEN

class MergeTranslator final : public HydraOpNodeTranslatorTyped<PXR_NS::UsdHydraOpMerge>
{
protected:
    DirtyTypeFlags get_dirty_flags_impl(const PXR_NS::UsdHydraOpMerge& prim, const PXR_NS::TfToken& property_name) override;
    PXR_NS::HdSceneIndexBaseRefPtr populate_impl(const PXR_NS::UsdHydraOpMerge& prim,
                                                 const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& inputs) override;
    void process_args_change_impl(const PXR_NS::UsdHydraOpMerge& prim, const PXR_NS::TfTokenVector& property_names,
                                  const PXR_NS::HdSceneIndexBaseRefPtr& scene_index) override;
};
OPENDCC_NAMESPACE_CLOSE
