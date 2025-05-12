/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/hydra_op/translator/node_translator.h"
#include <pxr/usdImaging/usdImaging/sceneIndices.h>
#include <pxr/imaging/hd/mergingSceneIndex.h>
#include "opendcc/hydra_op/schema/translateAPI.h"

OPENDCC_NAMESPACE_OPEN

class TranslateAPITranslator final : public HydraOpNodeTranslatorTyped<PXR_NS::UsdHydraOpTranslateAPI>
{
protected:
    DirtyTypeFlags get_dirty_flags_impl(const PXR_NS::UsdHydraOpTranslateAPI& prim, const PXR_NS::TfToken& property_name) override;
    PXR_NS::HdSceneIndexBaseRefPtr populate_impl(const PXR_NS::UsdHydraOpTranslateAPI& prim,
                                                 const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& inputs) override;
    void process_args_change_impl(const PXR_NS::UsdHydraOpTranslateAPI& prim, const PXR_NS::TfTokenVector& property_names,
                                  const PXR_NS::HdSceneIndexBaseRefPtr& scene_index) override;
    bool is_time_dependent() const override { return true; }
    void on_time_changed_impl(const PXR_NS::UsdHydraOpTranslateAPI& prim, const PXR_NS::HdSceneIndexBaseRefPtr& scene_index,
                              PXR_NS::UsdTimeCode time) override;

private:
    PXR_NS::UsdImagingSceneIndices m_indices;
    PXR_NS::UsdTimeCode m_time;
};

OPENDCC_NAMESPACE_CLOSE
