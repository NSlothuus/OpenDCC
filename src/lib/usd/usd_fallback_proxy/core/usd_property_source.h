/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <usd/usd_fallback_proxy/core/api.h>
#include <pxr/usd/usd/property.h>

OPENDCC_NAMESPACE_OPEN

class UsdPropertySource
{
public:
    UsdPropertySource() = default;
    UsdPropertySource(const UsdPropertySource&) = default;
    UsdPropertySource(UsdPropertySource&&) = default;
    UsdPropertySource(const PXR_NS::TfToken& source_group, const PXR_NS::TfType& source_plugin)
        : m_source_group(source_group)
        , m_source_plugin(source_plugin)
    {
    }
    ~UsdPropertySource() = default;

    UsdPropertySource& operator=(const UsdPropertySource&) = default;
    UsdPropertySource& operator=(UsdPropertySource&&) = default;

    PXR_NS::TfToken get_source_group() const { return m_source_group; }
    PXR_NS::TfType get_source_plugin() const { return m_source_plugin; }

private:
    PXR_NS::TfToken m_source_group;
    PXR_NS::TfType m_source_plugin;
};

OPENDCC_NAMESPACE_CLOSE
