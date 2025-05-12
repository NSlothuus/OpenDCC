// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/hydra_ext_render_session_api_adapter.h"
#include "hydra_render_session_api/renderSessionAPI.h"
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/usdImaging/usdImaging/stageSceneIndex.h>
#include <pxr/usd/usdRender/settings.h>
#include <pxr/usd/usdRender/product.h>
#include "hydra_ext_render_session_api_schema.h"
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/usdImaging/usdImaging/dataSourceAttribute.h>
#include <pxr/usd/usdRender/var.h>

PXR_NAMESPACE_USING_DIRECTIVE

TF_REGISTRY_FUNCTION(TfType)
{
    using Adapter = OPENDCC_NAMESPACE::HydraExtRenderSessionAPIAdapter;
    TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter>>();
    t.SetFactory<UsdImagingAPISchemaAdapterFactory<Adapter>>();
}

OPENDCC_NAMESPACE_OPEN

namespace
{
    class AuthoredAttributesDataSource : public HdContainerDataSource
    {
    public:
        HD_DECLARE_DATASOURCE(AuthoredAttributesDataSource);

        TfTokenVector GetNames() override
        {
            const auto attrs = m_prim.GetAttributes();
            TfTokenVector result(attrs.size());
            std::transform(attrs.begin(), attrs.end(), result.begin(), [](const auto& attr) { return attr.GetName(); });
            return result;
        }

        HdDataSourceBaseHandle Get(const TfToken& name) override
        {
            const auto attrs = m_prim.GetAttributes();
            for (const auto& attr : attrs)
            {
                if (attr.GetName() == name)
                {
                    return UsdImagingDataSourceAttributeNew(attr, m_globals, m_prim.GetPath(),
                                                            HdDataSourceLocator(m_prim.GetPath().GetAsToken(), name));
                }
            }

            return nullptr;
        }

    private:
        AuthoredAttributesDataSource(const UsdPrim& prim, const UsdImagingDataSourceStageGlobals& globals)
            : m_prim(prim)
            , m_globals(globals)
        {
        }

        UsdPrim m_prim;
        const UsdImagingDataSourceStageGlobals& m_globals;
    };

    template <class T>
    class RelationshipsDataSource : public HdContainerDataSource
    {
    public:
        HD_DECLARE_DATASOURCE(RelationshipsDataSource);

        TfTokenVector GetNames() override
        {
            TfTokenVector result;
            if (const auto rel = m_prim.GetRelationship(m_relationship_name))
            {
                const auto targets = get_targets();
                std::transform(targets.begin(), targets.end(), std::back_inserter(result), [](const auto& target) { return target.GetToken(); });
            }
            return result;
        }

        HdDataSourceBaseHandle Get(const TfToken& name) override
        {
            for (const auto& v : GetNames())
            {
                if (v == name)
                {
                    return T::New(m_prim.GetStage()->GetPrimAtPath(SdfPath(v)), m_globals);
                }
            }
            return nullptr;
        }

    private:
        RelationshipsDataSource(const UsdPrim& prim, const TfToken& relationship_name, const UsdImagingDataSourceStageGlobals& globals)
            : m_prim(prim)
            , m_relationship_name(relationship_name)
            , m_globals(globals)
        {
        }

        SdfPathVector get_targets() const
        {
            SdfPathVector result;
            if (const auto rel = m_prim.GetRelationship(m_relationship_name))
            {
                SdfPathVector targets;
                rel.GetTargets(&targets);

                auto stage = m_prim.GetStage();
                for (const auto& t : targets)
                {
                    if (stage->GetPrimAtPath(t))
                    {
                        result.push_back(t);
                    }
                }
            }
            return result;
        }

        UsdPrim m_prim;
        TfToken m_relationship_name;
        const UsdImagingDataSourceStageGlobals& m_globals;
    };

    class HydraExtRenderProductDataSource : public HdContainerDataSource
    {
    public:
        HD_DECLARE_DATASOURCE(HydraExtRenderProductDataSource);

        TfTokenVector GetNames() override
        {
            static const TfTokenVector names = { TfToken("settings"), TfToken("render_vars") };
            return names;
        }

        HdDataSourceBaseHandle Get(const TfToken& name) override
        {
            if (name == TfToken("settings"))
            {
                return AuthoredAttributesDataSource::New(m_prim, m_globals);
            }
            if (name == TfToken("render_vars"))
            {
                return RelationshipsDataSource<AuthoredAttributesDataSource>::New(m_prim, UsdRenderTokens->orderedVars, m_globals);
            }

            return nullptr;
        }

    private:
        HydraExtRenderProductDataSource(const UsdPrim& prim, const UsdImagingDataSourceStageGlobals& globals)
            : m_prim(prim)
            , m_globals(globals)
        {
        }

        UsdPrim m_prim;
        const UsdImagingDataSourceStageGlobals& m_globals;
    };

    class HydraExtRenderSettingsDataSource : public HdContainerDataSource
    {
    public:
        HD_DECLARE_DATASOURCE(HydraExtRenderSettingsDataSource);

        TfTokenVector GetNames() override
        {
            static const TfTokenVector names = { UsdHydraExtTokens->render_delegate, TfToken("render_products") };
            return names;
        }

        HdDataSourceBaseHandle Get(const TfToken& name) override
        {
            if (name == UsdHydraExtTokens->render_delegate)
            {
                auto api = UsdHydraExtRenderSessionAPI(m_render_settings.GetPrim());
                if (!api)
                {
                    return nullptr;
                }

                TfToken render_delegate { "Storm" };
                api.GetRender_delegateAttr().Get(&render_delegate, m_globals.GetTime());
                return HdRetainedTypedSampledDataSource<TfToken>::New(render_delegate);
            }
            if (name == TfToken("render_products"))
            {
                return RelationshipsDataSource<HydraExtRenderProductDataSource>::New(m_render_settings.GetPrim(), UsdRenderTokens->products,
                                                                                     m_globals);
            }
            return nullptr;
        }

    private:
        HydraExtRenderSettingsDataSource(const UsdRenderSettings& render_settings, const UsdImagingDataSourceStageGlobals& globals)
            : m_render_settings(render_settings)
            , m_globals(globals)
        {
        }

        UsdRenderSettings m_render_settings;
        const UsdImagingDataSourceStageGlobals& m_globals;
    };
    HD_DECLARE_DATASOURCE_HANDLES(HydraExtRenderSettingsDataSource);

    class HydraExtRenderSessionAPIDataSource : public HdContainerDataSource
    {
    public:
        HD_DECLARE_DATASOURCE(HydraExtRenderSessionAPIDataSource);
        HydraExtRenderSessionAPIDataSource(const UsdRenderSettings& render_settings, const UsdImagingDataSourceStageGlobals& globals)
            : m_render_settings(render_settings)
            , m_globals(globals)
        {
        }

        // Inherited via HdContainerDataSource
        virtual TfTokenVector GetNames() override { return { HydraExtRenderSessionAPISchema::get_schema_token() }; }

        virtual HdDataSourceBaseHandle Get(const TfToken& name) override
        {
            if (name == HydraExtRenderSessionAPISchema::get_schema_token())
            {
                return HydraExtRenderSettingsDataSource::New(m_render_settings, m_globals);
            }
            return nullptr;
        }

    private:
        UsdRenderSettings m_render_settings;
        const UsdImagingDataSourceStageGlobals& m_globals;
    };
};

HdContainerDataSourceHandle HydraExtRenderSessionAPIAdapter::GetImagingSubprimData(const UsdPrim& prim, const TfToken& subprim,
                                                                                   const TfToken& appliedInstanceName,
                                                                                   const UsdImagingDataSourceStageGlobals& stageGlobals)
{
    if (!subprim.IsEmpty() || !appliedInstanceName.IsEmpty())
    {
        return nullptr;
    }

    auto api = UsdHydraExtRenderSessionAPI(prim);
    if (!api)
    {
        return nullptr;
    }

    TfToken render_delegate { "Storm" };
    api.GetRender_delegateAttr().Get(&render_delegate, stageGlobals.GetTime());

    if (auto render_settings = UsdRenderSettings(prim))
    {
        return HydraExtRenderSessionAPIDataSource::New(render_settings, stageGlobals);
    }
    return nullptr;
}

HdDataSourceLocatorSet HydraExtRenderSessionAPIAdapter::InvalidateImagingSubprim(const UsdPrim& prim, const TfToken& subprim,
                                                                                 const TfToken& appliedInstanceName, const TfTokenVector& properties,
                                                                                 UsdImagingPropertyInvalidationType invalidationType)
{
    if (appliedInstanceName.IsEmpty())
    {
        return HdDataSourceLocatorSet();
    }

    if (subprim.IsEmpty())
    {
        return HdDataSourceLocator { UsdHydraExtTokens->HydraRenderSessionAPI };
    }
    return HdDataSourceLocatorSet();
}

OPENDCC_NAMESPACE_CLOSE
