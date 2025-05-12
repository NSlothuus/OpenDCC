/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/imaging/hd/api.h>
#include <pxr/imaging/hd/schema.h>
#include <pxr/imaging/hd/vectorSchema.h>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API HydraExtRenderSessionAPISchema final : public PXR_NS::HdSchema
{
public:
    HydraExtRenderSessionAPISchema(PXR_NS::HdContainerDataSourceHandle container)
        : PXR_NS::HdSchema(container)
    {
    }

    static HydraExtRenderSessionAPISchema get_from_parent(const PXR_NS::HdContainerDataSourceHandle& from_parent_container);

    PXR_NS::HdTokenDataSourceHandle get_render_delegate() const;

    static const PXR_NS::TfToken& get_schema_token();
    static const PXR_NS::HdDataSourceLocator& get_default_locator();
};

OPENDCC_NAMESPACE_CLOSE
