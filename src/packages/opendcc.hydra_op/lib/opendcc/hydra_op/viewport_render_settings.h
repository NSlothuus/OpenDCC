/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/app/viewport/hydra_render_settings.h"
#include <pxr/usd/usd/timeCode.h>
#include <pxr/imaging/hd/sceneIndex.h>

OPENDCC_NAMESPACE_OPEN

class HydraOpViewportRenderSettings final : public HydraRenderSettings
{
public:
    static std::shared_ptr<HydraOpViewportRenderSettings> create(PXR_NS::HdSceneIndexBaseRefPtr si);

    PXR_NS::GfVec2i get_resolution() const override;
    PXR_NS::SdfPath get_camera_path() const override;
    PXR_NS::GfCamera get_camera() const override;
    std::vector<AOV> get_aovs() const override;

    PXR_NS::HdAovSettingsMap get_settings() const override;
    PXR_NS::TfToken get_render_delegate() const override;
    std::vector<RenderProduct> get_render_products() const override;
    const PXR_NS::SdfPath get_settings_path() const;

private:
    HydraOpViewportRenderSettings(const PXR_NS::SdfPath& settings_path, const PXR_NS::HdAovSettingsMap& settings, const std::vector<AOV>& aovs,
                                  const std::vector<RenderProduct>& render_products, const PXR_NS::SdfPath& camera_path, const PXR_NS::GfCamera& cam,
                                  const PXR_NS::TfToken& render_delegate);

    std::vector<AOV> m_aovs;
    std::vector<RenderProduct> m_render_products;

    PXR_NS::SdfPath m_camera_path;
    PXR_NS::GfCamera m_camera;
    PXR_NS::HdAovSettingsMap m_settings;
    PXR_NS::TfToken m_render_delegate;
    PXR_NS::SdfPath m_settings_path;
};

OPENDCC_NAMESPACE_CLOSE
