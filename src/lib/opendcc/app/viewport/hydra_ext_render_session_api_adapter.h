/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include <pxr/usdImaging/usdImaging/apiSchemaAdapter.h>

OPENDCC_NAMESPACE_OPEN

class HydraExtRenderSessionAPIAdapter final : public PXR_NS::UsdImagingAPISchemaAdapter
{
public:
    using BaseAdapter = UsdImagingAPISchemaAdapter;

    PXR_NS::HdContainerDataSourceHandle GetImagingSubprimData(const PXR_NS::UsdPrim& prim, const PXR_NS::TfToken& subprim,
                                                              const PXR_NS::TfToken& appliedInstanceName,
                                                              const PXR_NS::UsdImagingDataSourceStageGlobals& stageGlobals) override;

    PXR_NS::HdDataSourceLocatorSet InvalidateImagingSubprim(const PXR_NS::UsdPrim& prim, const PXR_NS::TfToken& subprim,
                                                            const PXR_NS::TfToken& appliedInstanceName, const PXR_NS::TfTokenVector& properties,
                                                            PXR_NS::UsdImagingPropertyInvalidationType invalidationType) override;
};

OPENDCC_NAMESPACE_CLOSE
