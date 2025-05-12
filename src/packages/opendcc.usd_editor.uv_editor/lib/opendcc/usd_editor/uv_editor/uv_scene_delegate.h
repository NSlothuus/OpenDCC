/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/app/viewport/viewport_scene_delegate.h"
#include "opendcc/usd_editor/uv_editor/prim_info.h"
#include "opendcc/app/core/stage_watcher.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/imaging/hd/material.h>

#include <memory>

OPENDCC_NAMESPACE_OPEN

class UVSceneDelegate : public ViewportSceneDelegate
{
public:
    UVSceneDelegate(PXR_NS::HdRenderIndex* render_index, const PXR_NS::SdfPath& delegate_id);
    ~UVSceneDelegate() override;

    void update(const ViewportHydraEngineParams& engine_params) override;
    void populate_selection(const SelectionList& selection_list, const PXR_NS::HdSelectionSharedPtr& result) override;
    PXR_NS::HdMeshTopology GetMeshTopology(PXR_NS::SdfPath const& id) override;
    PXR_NS::GfRange3d GetExtent(PXR_NS::SdfPath const& id) override;
    PXR_NS::GfMatrix4d GetTransform(PXR_NS::SdfPath const& id) override;
    bool GetVisible(PXR_NS::SdfPath const& id) override;
    bool GetDoubleSided(PXR_NS::SdfPath const& id) override;
    PXR_NS::HdCullStyle GetCullStyle(PXR_NS::SdfPath const& id) override;
    PXR_NS::HdDisplayStyle GetDisplayStyle(PXR_NS::SdfPath const& id) override;
    PXR_NS::VtValue Get(PXR_NS::SdfPath const& id, PXR_NS::TfToken const& key) override;
    PXR_NS::HdPrimvarDescriptorVector GetPrimvarDescriptors(PXR_NS::SdfPath const& id, PXR_NS::HdInterpolation interpolation) override;
    PXR_NS::HdReprSelector GetReprSelector(PXR_NS::SdfPath const& id) override;
    PXR_NS::SdfPath GetMaterialId(PXR_NS::SdfPath const& rprimId) override;
    PXR_NS::VtValue GetMaterialResource(PXR_NS::SdfPath const& materialId) override;

private:
    bool initialize(const PXR_NS::SdfPathVector& highlighted, const PXR_NS::TfToken& uv_primvar);
    void repopulate_geometry(const PXR_NS::SdfPathVector& highlighted);
    void update_background_texture(const std::string& new_texture);
    void reload_current_texture();
    PXR_NS::SdfPath get_background_texture_prim() const;
    PXR_NS::SdfPath get_background_texture_material() const;
    PXR_NS::TfToken get_texture_node_id() const;
    PXR_NS::HdMaterialNetworkMap get_mesh_material_network(PXR_NS::SdfPath prim_path);

    enum class TilingMode
    {
        NONE,
        UDIM
    };

    bool m_is_initialized = false;
    PXR_NS::UsdStageRefPtr m_stage;
    PXR_NS::UsdTimeCode m_time;
    std::string m_texture_file;
    PXR_NS::TfToken m_uv_primvar;
    PXR_NS::SdfPathVector m_highlighted_paths;
    TilingMode m_tiling_mode = TilingMode::NONE;
    bool m_show_texture = false;

    PrimInfoMap m_prims_info;

    std::unique_ptr<StageObjectChangedWatcher> m_watcher;

    enum class RenderMode
    {
        Hull,
        Wire,
    };
    RenderMode m_mode = RenderMode::Hull;
};

OPENDCC_NAMESPACE_CLOSE
