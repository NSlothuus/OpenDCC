/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include "usd/usd_fallback_proxy/utils/api.h"
#include "usd/usd_fallback_proxy/core/property_gatherer.h"
#include "usd/usd_fallback_proxy/core/usd_property_source.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdRender/var.h>
#include <pxr/usd/usdRender/product.h>

OPENDCC_NAMESPACE_OPEN

namespace usd_fallback_proxy
{
    namespace utils
    {
        struct FALLBACK_PROXY_UTILS_API PropertyInfo
        {
            PXR_NS::SdfSpecType type;
            PXR_NS::UsdMetadataValueMap metadata;
        };
        using PropertyMap = std::unordered_map<PXR_NS::TfToken, PropertyInfo, PXR_NS::TfToken::HashFunctor>;
        using PropertyPair = PropertyMap::value_type;

        PXR_NS::TfToken FALLBACK_PROXY_UTILS_API get_current_render_delegate_name(PXR_NS::UsdStageWeakPtr stage);

        void FALLBACK_PROXY_UTILS_API try_insert_property_pair(const PropertyPair& property_pair, const PXR_NS::UsdPrim& prim,
                                                               PropertyGatherer& property_gatherer, const UsdPropertySource& source);

        bool FALLBACK_PROXY_UTILS_API is_connect_to_render_settings(const PXR_NS::UsdRenderProduct& product);
        bool FALLBACK_PROXY_UTILS_API is_connect_to_render_settings(const PXR_NS::UsdRenderVar& var);
    }
}

OPENDCC_NAMESPACE_CLOSE
