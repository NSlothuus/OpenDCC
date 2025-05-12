/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/imaging/hd/aov.h>
#include <pxr/usd/usdRender/settings.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdRender/product.h>
#include <pxr/usd/usdRender/var.h>

OPENDCC_NAMESPACE_OPEN

class HydraRenderSettings
{
public:
    struct AOV
    {
        PXR_NS::TfToken name;
        PXR_NS::TfToken product_name;
        PXR_NS::HdAovDescriptor descriptor;
    };

    struct RenderVar
    {
        PXR_NS::TfToken name;
        PXR_NS::HdAovDescriptor descriptor;
    };

    struct RenderProduct
    {
        PXR_NS::TfToken name;
        std::vector<RenderVar> render_vars;
        PXR_NS::HdAovSettingsMap settings;
    };

    virtual ~HydraRenderSettings() {}

    virtual PXR_NS::GfVec2i get_resolution() const = 0;
    virtual PXR_NS::SdfPath get_camera_path() const = 0;
    virtual PXR_NS::GfCamera get_camera() const = 0;
    virtual std::vector<AOV> get_aovs() const = 0;
    virtual PXR_NS::HdAovSettingsMap get_settings() const = 0;
    virtual std::vector<RenderProduct> get_render_products() const = 0;
    virtual PXR_NS::TfToken get_render_delegate() const = 0;
};

class OPENDCC_API UsdHydraRenderSettings final : public HydraRenderSettings
{
public:
    static std::shared_ptr<UsdHydraRenderSettings> create(PXR_NS::UsdStageRefPtr stage, PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default(),
                                                          PXR_NS::SdfPath settings_path = PXR_NS::SdfPath());

    PXR_NS::GfVec2i get_resolution() const override;
    PXR_NS::SdfPath get_camera_path() const override;
    PXR_NS::GfCamera get_camera() const override;
    std::vector<AOV> get_aovs() const;
    void clear();
    bool has_setting(PXR_NS::SdfPath path) const;

    PXR_NS::HdAovSettingsMap get_settings() const override;
    std::vector<RenderProduct> get_render_products() const override;

    PXR_NS::UsdTimeCode get_time() const { return m_time; }
    PXR_NS::TfToken get_render_delegate() const override;

private:
    UsdHydraRenderSettings();
    bool build_render_settings(PXR_NS::UsdRenderSettings render_settings, PXR_NS::UsdTimeCode time);

    std::vector<AOV> m_aovs;
    std::vector<RenderProduct> m_render_products;

    PXR_NS::UsdRelationship m_camera_rel;
    PXR_NS::GfVec2i m_resolution = PXR_NS::GfVec2i(800, 600);
    float m_pixel_aspect_ratio = 1;
    PXR_NS::GfVec4f m_data_window_ndc = PXR_NS::GfVec4f(0, 0, 1, 1);
    PXR_NS::VtTokenArray m_included_purposes;
    std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash> m_settings_paths;
    PXR_NS::HdAovSettingsMap m_settings;
    PXR_NS::UsdTimeCode m_time;
    PXR_NS::TfToken m_render_delegate;
};

OPENDCC_NAMESPACE_CLOSE
