/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/base/tf/type.h>
#include <pxr/usd/usd/property.h>

#include <unordered_map>

OPENDCC_NAMESPACE_OPEN

class RenderSettingsRegistry
{
private:
    RenderSettingsRegistry();
    RenderSettingsRegistry(const RenderSettingsRegistry&) = delete;
    RenderSettingsRegistry(RenderSettingsRegistry&&) = delete;
    RenderSettingsRegistry& operator=(const RenderSettingsRegistry&) = delete;
    RenderSettingsRegistry& operator=(RenderSettingsRegistry&&) = delete;

public:
    static RenderSettingsRegistry& instance();

    PXR_NS::UsdProperty get_property(const std::string& render_delegate, const PXR_NS::UsdPrim& prim, const PXR_NS::TfToken& property_name) const;
    std::vector<PXR_NS::UsdProperty> get_properties(const std::string& render_delegate, const PXR_NS::UsdPrim& prim) const;

private:
    PXR_NS::TfType get_type();
    void initialize_stages();
    std::unordered_map<std::string /* render_delegate name */, PXR_NS::UsdStageRefPtr> m_stages;
};

OPENDCC_NAMESPACE_CLOSE
