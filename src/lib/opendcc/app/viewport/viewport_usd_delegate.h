/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/viewport/visibility_mask.h"
#include "opendcc/app/viewport/prim_material_override.h"
#include "opendcc/app/viewport/viewport_scene_delegate.h"
#include "opendcc/app/viewport/viewport_refine_manager.h"

#include <pxr/pxr.h>
#include <pxr/usdImaging/usdImaging/delegate.h>
#include "viewport_locator_delegate.h"

OPENDCC_NAMESPACE_OPEN

class SelectionList;

class ViewportUsdDelegate
    : public ViewportSceneDelegate
    , public PXR_NS::UsdImagingDelegate
{
public:
    using OnGetInstancerIdCallback = std::function<std::tuple<PXR_NS::SdfPath, bool>(const PXR_NS::SdfPath&)>;
    using OnGetInstancerTransformCallback = std::function<std::tuple<PXR_NS::GfMatrix4d, bool>(const PXR_NS::SdfPath&)>;

    ViewportUsdDelegate(PXR_NS::HdRenderIndex* render_index, const PXR_NS::SdfPath& delegate_id);
    virtual ~ViewportUsdDelegate();

    virtual void update(const ViewportHydraEngineParams& engine_params) override;
    virtual void populate_selection(const SelectionList& selection_list, const PXR_NS::HdSelectionSharedPtr& result) override;

    virtual PXR_NS::GfRange3d GetExtent(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::GfMatrix4d GetTransform(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::HdReprSelector GetReprSelector(PXR_NS::SdfPath const& id) override;
#if PXR_VERSION < 2005
    virtual PXR_NS::SdfPath GetPathForInstanceIndex(const PXR_NS::SdfPath& protoRprimId, int protoIndex, int* instancerIndex,
                                                    PXR_NS::SdfPath* masterCachePath = nullptr,
                                                    PXR_NS::SdfPathVector* instanceContext = nullptr) override;
#else
    virtual PXR_NS::SdfPath GetScenePrimPath(PXR_NS::SdfPath const& rprimId, int instanceIndex,
                                             PXR_NS::HdInstancerContext* instancerContext = nullptr) override;
#endif
    virtual void Sync(PXR_NS::HdSyncRequestVector* request) override;
    virtual void PostSyncCleanup() override;
    virtual bool IsEnabled(PXR_NS::TfToken const& option) const override;
    virtual PXR_NS::HdMeshTopology GetMeshTopology(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::HdBasisCurvesTopology GetBasisCurvesTopology(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::PxOsdSubdivTags GetSubdivTags(PXR_NS::SdfPath const& id) override;
    virtual bool GetVisible(PXR_NS::SdfPath const& id) override;
    virtual bool GetDoubleSided(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::HdCullStyle GetCullStyle(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::VtValue GetShadingStyle(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::HdDisplayStyle GetDisplayStyle(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::VtValue Get(PXR_NS::SdfPath const& id, PXR_NS::TfToken const& key) override;
    virtual PXR_NS::TfToken GetRenderTag(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::VtArray<PXR_NS::TfToken> GetCategories(PXR_NS::SdfPath const& id) override;
    virtual std::vector<PXR_NS::VtArray<PXR_NS::TfToken>> GetInstanceCategories(PXR_NS::SdfPath const& instancerId) override;
    virtual PXR_NS::HdIdVectorSharedPtr GetCoordSysBindings(PXR_NS::SdfPath const& id) override;
    virtual size_t SampleTransform(PXR_NS::SdfPath const& id, size_t maxSampleCount, float* sampleTimes, PXR_NS::GfMatrix4d* sampleValues) override;
    virtual size_t SampleInstancerTransform(PXR_NS::SdfPath const& instancerId, size_t maxSampleCount, float* sampleTimes,
                                            PXR_NS::GfMatrix4d* sampleValues) override;
    virtual size_t SamplePrimvar(PXR_NS::SdfPath const& id, PXR_NS::TfToken const& key, size_t maxSampleCount, float* sampleTimes,
                                 PXR_NS::VtValue* sampleValues) override;
    virtual PXR_NS::VtIntArray GetInstanceIndices(PXR_NS::SdfPath const& instancerId, PXR_NS::SdfPath const& prototypeId) override;
    virtual PXR_NS::GfMatrix4d GetInstancerTransform(PXR_NS::SdfPath const& instancerId) override;
    virtual PXR_NS::SdfPath GetMaterialId(PXR_NS::SdfPath const& rprimId) override;
#if PXR_VERSION < 2002
    virtual std::string GetSurfaceShaderSource(PXR_NS::SdfPath const& materialId) override;
    virtual std::string GetDisplacementShaderSource(PXR_NS::SdfPath const& materialId) override;
    virtual PXR_NS::VtValue GetMaterialParamValue(PXR_NS::SdfPath const& materialId, PXR_NS::TfToken const& paramName) override;
    virtual PXR_NS::HdMaterialParamVector GetMaterialParams(PXR_NS::SdfPath const& materialId) override;
    virtual PXR_NS::VtDictionary GetMaterialMetadata(PXR_NS::SdfPath const& materialId) override;
#else
    virtual PXR_NS::VtValue GetMaterialResource(PXR_NS::SdfPath const& materialId) override;
#endif
#if PXR_VERSION < 2108
    virtual PXR_NS::HdTextureResource::ID GetTextureResourceID(PXR_NS::SdfPath const& textureId) override;
    virtual PXR_NS::HdTextureResourceSharedPtr GetTextureResource(PXR_NS::SdfPath const& textureId) override;
#endif
    virtual PXR_NS::HdRenderBufferDescriptor GetRenderBufferDescriptor(PXR_NS::SdfPath const& id) override;
    virtual PXR_NS::VtValue GetLightParamValue(PXR_NS::SdfPath const& id, PXR_NS::TfToken const& paramName) override;
    virtual PXR_NS::VtValue GetCameraParamValue(PXR_NS::SdfPath const& cameraId, PXR_NS::TfToken const& paramName) override;
    virtual PXR_NS::HdVolumeFieldDescriptorVector GetVolumeFieldDescriptors(PXR_NS::SdfPath const& volumeId) override;
    virtual PXR_NS::TfTokenVector GetExtComputationSceneInputNames(PXR_NS::SdfPath const& computationId) override;
    virtual PXR_NS::HdExtComputationInputDescriptorVector GetExtComputationInputDescriptors(PXR_NS::SdfPath const& computationId) override;
    virtual PXR_NS::HdExtComputationOutputDescriptorVector GetExtComputationOutputDescriptors(PXR_NS::SdfPath const& computationId) override;
    virtual PXR_NS::HdExtComputationPrimvarDescriptorVector GetExtComputationPrimvarDescriptors(PXR_NS::SdfPath const& id,
                                                                                                PXR_NS::HdInterpolation interpolationMode) override;
    virtual PXR_NS::VtValue GetExtComputationInput(PXR_NS::SdfPath const& computationId, PXR_NS::TfToken const& input) override;
    virtual std::string GetExtComputationKernel(PXR_NS::SdfPath const& computationId) override;
    virtual void InvokeExtComputation(PXR_NS::SdfPath const& computationId, PXR_NS::HdExtComputationContext* context) override;
    virtual PXR_NS::HdPrimvarDescriptorVector GetPrimvarDescriptors(PXR_NS::SdfPath const& id, PXR_NS::HdInterpolation interpolation) override;
    virtual PXR_NS::TfTokenVector GetTaskRenderTags(PXR_NS::SdfPath const& taskId) override;
#ifdef OPENDCC_HOUDINI_SUPPORT
#if PXR_VERSION >= 2005
    virtual PXR_NS::SdfPath GetDataSharingId(PXR_NS::SdfPath const& primId) override;
#endif
#endif
#if PXR_VERSION >= 2108
    virtual PXR_NS::SdfPath GetInstancerId(const PXR_NS::SdfPath& primId) override;
    OPENDCC_API void set_instancer_id_callback(OnGetInstancerIdCallback callback);
    OPENDCC_API void set_instancer_transform_callback(OnGetInstancerTransformCallback callback);
#endif
private:
    void initialize(const ViewportHydraEngineParams& engine_params);
    void prepare_batch(const ViewportHydraEngineParams& params);
    // Returns stage paths
    PXR_NS::SdfPathVector compute_exclude_paths(PXR_NS::UsdStageRefPtr stage, const PXR_NS::SdfPathVector& populated_paths) const;
    void update_repr_paths(const std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash>& repr_paths);

    // override
    using OverrideMap = std::unordered_map<PXR_NS::SdfPath, PrimMaterialDescriptor, PXR_NS::SdfPath::Hash>;
    using OverrideAssignments = std::unordered_map<PXR_NS::SdfPath, PXR_NS::SdfPath, PXR_NS::SdfPath::Hash>;

    void on_material_changed(const size_t material_id, const PrimMaterialDescriptor& descr, const PrimMaterialOverride::Status status,
                             OverrideMap& material_map, OverrideAssignments& assignments);
    void on_material_assignment_changed(const size_t material_id, const PXR_NS::SdfPath& assignment, const PrimMaterialOverride::Status status,
                                        OverrideMap& material_map, OverrideAssignments& assignments);
    void on_material_resource_changed(const PXR_NS::SdfPath& mat_path, const PrimMaterialDescriptor& descr, const PrimMaterialOverride::Status status,
                                      OverrideMap& material_map, OverrideMap& resource_map, OverrideAssignments& assignments);
    void clear_tool_overrides();

    Application::CallbackHandle m_tool_changed_handle;

    PrimMaterialOverride::MaterialDispatcherHandle m_tool_material_handle;
    PrimMaterialOverride::AssignmentDispatcherHandle m_tool_assignment_handle;
    PrimMaterialOverride::MaterialResourceDispatcherHandle m_tool_material_resource_handle;

    OverrideMap m_tool_material_overrides;
    OverrideMap m_tool_material_resource_overrides;
    OverrideAssignments m_tool_material_assignments;

    PrimMaterialOverride::MaterialDispatcherHandle m_persistent_material_handle;
    PrimMaterialOverride::AssignmentDispatcherHandle m_persistent_assignment_handle;
    PrimMaterialOverride::MaterialResourceDispatcherHandle m_persistent_material_resource_handle;

    OverrideMap m_persistent_material_overrides;
    OverrideMap m_persistent_material_resource_overrides;
    OverrideAssignments m_persistent_material_assignments;

    //
    UsdRefineHandle m_usd_refine_handle;
    UsdStageClearedHandle m_usd_stage_cleared_handle;
    PXR_NS::HdReprSelector m_repr_selector;
    std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash> m_repr_paths;
    PXR_NS::UsdStageRefPtr m_stage;
    VisibilityMask m_visibility_mask;
#if PXR_VERSION >= 2108
    OnGetInstancerIdCallback m_on_get_instancer_id;
    OnGetInstancerTransformCallback m_on_get_instancer_transform;
#endif
    Application::SelectionMode m_last_mode = Application::SelectionMode::COUNT;
};

OPENDCC_NAMESPACE_CLOSE
