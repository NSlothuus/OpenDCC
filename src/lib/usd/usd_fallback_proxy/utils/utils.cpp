// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "usd/usd_fallback_proxy/utils/utils.h"

#include <pxr/usd/usdRender/tokens.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usdRender/settings.h>
#include <pxr/usd/usdRender/product.h>
#include <pxr/usd/usdRender/var.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

namespace usd_fallback_proxy
{
    namespace utils
    {

        PXR_NS::TfToken get_current_render_delegate_name(PXR_NS::UsdStageWeakPtr stage)
        {
            TfToken render_delegate;

            if (!stage)
            {
                return render_delegate;
            }

            std::string render_settings_prim_path;
            if (!stage->GetMetadata(UsdRenderTokens->renderSettingsPrimPath, &render_settings_prim_path))
            {
                return render_delegate;
            }

            const auto render_settings = stage->GetPrimAtPath(SdfPath(render_settings_prim_path));
            if (!render_settings)
            {
                return render_delegate;
            }

            const auto render_delegate_attr = render_settings.GetAttribute(TfToken("render_delegate"));
            if (render_delegate_attr)
            {
                if (!render_delegate_attr.Get<TfToken>(&render_delegate))
                {
                    return render_delegate;
                }
            }
            else
            {
                render_delegate = TfToken("GL");
            }

            return render_delegate;
        }

        void try_insert_property_pair(const PropertyPair& property_pair, const UsdPrim& prim, PropertyGatherer& property_gatherer,
                                      const UsdPropertySource& source)
        {
            property_gatherer.try_insert_property(property_pair.second.type, property_pair.first, prim, property_pair.second.metadata, source);
        }

        static bool is_connect_to_render_settings(const UsdPrim& prim)
        {
            if (!prim)
            {
                return false;
            }

            const auto stage = prim.GetStage();
            if (!stage)
            {
                return false;
            }

            const auto settings = UsdRenderSettings::GetStageRenderSettings(stage);
            if (!settings)
            {
                return false;
            }

            auto products_rel = settings.GetProductsRel();
            if (!products_rel)
            {
                return false;
            }

            SdfPathVector product_paths;
            products_rel.GetTargets(&product_paths);

            for (const auto& product_path : product_paths)
            {
                auto product = UsdRenderProduct::Get(stage, product_path);
                if (!product)
                {
                    continue;
                }

                if (product.GetPrim() == prim)
                {
                    return true;
                }

                auto vars = product.GetOrderedVarsRel();
                if (!vars)
                {
                    continue;
                }

                SdfPathVector var_paths;
                vars.GetTargets(&var_paths);

                for (const auto& var_path : var_paths)
                {
                    auto var = UsdRenderVar::Get(stage, var_path);

                    if (var && var.GetPrim() == prim)
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        bool FALLBACK_PROXY_UTILS_API is_connect_to_render_settings(const PXR_NS::UsdRenderProduct& product)
        {
            return is_connect_to_render_settings(product.GetPrim());
        }

        bool FALLBACK_PROXY_UTILS_API is_connect_to_render_settings(const PXR_NS::UsdRenderVar& var)
        {
            return is_connect_to_render_settings(var.GetPrim());
        }

    }
}

OPENDCC_NAMESPACE_CLOSE
