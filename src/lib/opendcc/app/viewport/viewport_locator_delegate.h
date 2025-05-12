/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/imaging/hd/meshTopology.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/imaging/hd/selection.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>
#include <pxr/imaging/glf/simpleLight.h>
#include <pxr/usd/usd/collectionAPI.h>
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/stage_watcher.h"
#include <memory>
#include <queue>

#include "opendcc/app/viewport/usd_viewport_locator_data.h"
#include "opendcc/app/viewport/viewport_scene_delegate.h"
#include <pxr/base/gf/camera.h>
#include "opendcc/app/viewport/viewport_usd_locator.h"
#include "opendcc/app/viewport/visibility_mask.h"

OPENDCC_NAMESPACE_OPEN

class ViewportUsdDelegate;

class OPENDCC_API ViewportLocatorDelegate : public ViewportSceneDelegate
{
public:
    ViewportLocatorDelegate(PXR_NS::HdRenderIndex* renderIndex, const PXR_NS::SdfPath& delegate_id);
    virtual ~ViewportLocatorDelegate();

    virtual PXR_NS::GfRange3d GetExtent(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::GfMatrix4d GetTransform(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::VtValue Get(PXR_NS::SdfPath const& id, PXR_NS::TfToken const& key) override;

    virtual bool GetDoubleSided(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::HdCullStyle GetCullStyle(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::HdReprSelector GetReprSelector(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::TfToken GetRenderTag(PXR_NS::SdfPath const& id) override;

    virtual bool GetVisible(PXR_NS::SdfPath const& id) override;
    PXR_INTERNAL_NS::HdMeshTopology GetMeshTopology(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::HdBasisCurvesTopology GetBasisCurvesTopology(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::HdPrimvarDescriptorVector GetPrimvarDescriptors(PXR_NS::SdfPath const& id, PXR_NS::HdInterpolation interpolation) override;

    virtual PXR_NS::SdfPath GetMaterialId(PXR_NS::SdfPath const& rprimId) override;
#if PXR_VERSION >= 2002
    virtual PXR_NS::VtValue GetMaterialResource(const PXR_NS::SdfPath& materialId) override;
#else
    virtual PXR_NS::HdMaterialParamVector GetMaterialParams(const PXR_NS::SdfPath& materialId) override;
    virtual std::string GetSurfaceShaderSource(PXR_NS::SdfPath const& materialId) override;
    virtual PXR_NS::HdTextureResource::ID GetTextureResourceID(const PXR_NS::SdfPath& textureId) override;
    virtual PXR_NS::HdTextureResourceSharedPtr GetTextureResource(const PXR_NS::SdfPath& textureId) override;
#endif
    virtual void update(const ViewportHydraEngineParams& engine_params) override;
    virtual void populate_selection(const SelectionList& selection_list, const PXR_NS::HdSelectionSharedPtr& result) override;
    std::weak_ptr<PXR_NS::HdRenderIndex> get_main_render_index() { return m_main_render_index; }

private:
    friend class UsdViewportLocatorData;
    friend class ViewportUsdLightLocator;

    bool is_locator(const PXR_NS::UsdPrim& prim) const;
    void initialize_locators(PXR_NS::UsdStageRefPtr current_stage);
    void set_invised_paths(const std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash>& invised_paths);
    void initialize(const ViewportHydraEngineParams& engine_params);

    bool m_is_init = false;
    std::queue<std::function<void()>> m_tasks;
    std::shared_ptr<StageObjectChangedWatcher> m_watcher;
    PXR_NS::UsdStageRefPtr m_cur_stage;
    PXR_NS::Hd_SortedIds m_prim_ids;
    VisibilityMask m_visibility_mask;
    Application::CallbackHandle m_time_changed_event_handle;

    std::unique_ptr<UsdViewportLocatorData> m_locators_data;
    std::unordered_map<PXR_NS::SdfPath, ViewportUsdLocatorPtr, PXR_NS::SdfPath::Hash> m_locators;
    std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash> m_time_varying_locators;
    std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash> m_invised_paths;
#if PXR_VERSION < 2108
    std::unordered_map<std::string, PXR_NS::HdTextureResourceSharedPtr> m_texture_resources;
#endif
    std::weak_ptr<PXR_NS::HdRenderIndex> m_main_render_index;
    PXR_NS::UsdTimeCode m_time;
};

OPENDCC_NAMESPACE_CLOSE
