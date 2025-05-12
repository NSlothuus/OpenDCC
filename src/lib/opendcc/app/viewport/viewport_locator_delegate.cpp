// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_locator_delegate.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"
#include "opendcc/app/viewport/shadow_matrix.h"

#include <pxr/usd/sdf/path.h>
#include <pxr/imaging/pxOsd/tokens.h>
#include <pxr/usdImaging/usdImaging/tokens.h>
#include <pxr/imaging/hd/primGather.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/modelAPI.h>
#include <pxr/usd/usdLux/rectLight.h>
#include <pxr/imaging/hd/sceneDelegate.h>
#include <pxr/usd/usdLux/distantLight.h>
#include <pxr/imaging/hdx/simpleLightTask.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/usd/usdLux/sphereLight.h>
#include <pxr/usd/usdLux/cylinderLight.h>
#include <pxr/usd/usdLux/diskLight.h>
#include <pxr/usd/usdLux/domeLight.h>
#include <pxr/imaging/hio/glslfx.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#if PXR_VERSION < 2108
#include <pxr/imaging/glf/textureRegistry.h>
#include <pxr/imaging/glf/image.h>
#include <pxr/imaging/hdSt/textureResource.h>
#include <pxr/imaging/glf/textureHandle.h>
#endif
#include <pxr/imaging/hd/light.h>
#if PXR_VERSION >= 2002
#include <pxr/imaging/hd/material.h>
#endif
#include <type_traits>
#include "opendcc/app/viewport/viewport_hydra_engine.h"
#include "opendcc/app/viewport/istage_resolver.h"
#include "opendcc/app/viewport/shadow_matrix.h"
#include "pxr/usd/usdGeom/camera.h"
#include "pxr/usd/usdLux/lightFilter.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE;

OPENDCC_REGISTER_SCENE_DELEGATE(ViewportLocatorDelegate, TfToken("USD"))

ViewportLocatorDelegate::ViewportLocatorDelegate(HdRenderIndex* render_index, const SdfPath& delegate_id)
    : ViewportSceneDelegate(render_index, delegate_id)
{
}

ViewportLocatorDelegate::~ViewportLocatorDelegate()
{
    m_prim_ids.Clear();
    m_locators.clear();
}

void ViewportLocatorDelegate::initialize(const ViewportHydraEngineParams& engine_params)
{
    if (!engine_params.is_hd_st)
        return;

    m_invised_paths = engine_params.invised_paths;
    m_main_render_index = engine_params.main_render_index;

    initialize_locators(m_cur_stage);
    m_watcher = std::make_shared<StageObjectChangedWatcher>(m_cur_stage, [this](PXR_NS::UsdNotice::ObjectsChanged const& notice) {
        if (!notice.GetResyncedPaths().empty())
        {
            for (auto locator : m_locators)
            {
                const auto path = SdfPath(locator.first);
                auto prim = m_cur_stage->GetPrimAtPath(path);
                if (!prim)
                {
                    m_tasks.push([this, path]() { m_locators_data->remove_locator(path); });
                }
                else
                {
                    m_locators_data->update(path);
                    const auto& locator_path = prim.GetPrimPath();
                    m_tasks.push([this, locator_path]() { m_locators_data->mark_locator_dirty(locator_path, HdChangeTracker::AllDirty); });
                }
            }

            m_tasks.push([this]() { initialize_locators(m_cur_stage); });
        }

        for (auto& item : notice.GetChangedInfoOnlyPaths())
        {
            if (auto prim = m_cur_stage->GetPrimAtPath(item.GetPrimPath()))
            {
                if (!m_locators_data->is_locator(prim.GetPrimPath()))
                {
                    for (auto locator : m_locators)
                    {
                        if (locator.first.GetString().find(item.GetPrimPath().GetString()) != std::string::npos)
                        {
                            auto locator_path = SdfPath(locator.first);
                            m_tasks.push([this, locator_path]() { m_locators_data->mark_locator_dirty(locator_path, HdChangeTracker::AllDirty); });
                        }
                    }

                    continue;
                }

                const auto& locator_path = item.GetPrimPath();
                auto locator_it = m_locators.find(item.GetPrimPath());
                if (m_locators.find(item.GetPrimPath()) == m_locators.end())
                {
                    m_locators_data->insert_locator(locator_path, m_time);
                }
                else
                {
                    auto attr_name = item.GetNameToken();
                    m_locators_data->update(item.GetPrimPath());
                    m_tasks.push([this, locator_path, attr_name]() {
                        if (m_cur_stage)
                        {
                            if (auto prim = m_cur_stage->GetPrimAtPath(locator_path))
                            {
                                if (auto attr = prim.GetAttribute(attr_name))
                                {
                                    if (attr.ValueMightBeTimeVarying())
                                        m_time_varying_locators.insert(locator_path);
                                }
                            }
                        }
                        m_locators_data->mark_locator_dirty(locator_path, HdChangeTracker::AllDirty);
                    });
                }
            }
            else if (m_locators.find(item.GetPrimPath()) != m_locators.end())
            {

                auto locator_path = item.GetPrimPath();
                m_tasks.push([this, locator_path]() { m_locators_data->remove_locator(locator_path); });
                m_locators.erase(item.GetPrimPath());
            }
        }
    });
    m_is_init = true;
}

void ViewportLocatorDelegate::populate_selection(const SelectionList& selection_list, const PXR_NS::HdSelectionSharedPtr& result)
{
    for (const auto& selection_entry : selection_list)
    {
        SdfPathVector affectedPrims;
        HdPrimGather gather;
        gather.Subtree(m_prim_ids.GetIds(), selection_entry.first, &affectedPrims);

        for (auto& prim_path : affectedPrims)
        {
            result->AddRprim(m_selection_mode, convert_stage_path_to_index_path(prim_path));
        }
    }
}

PXR_NS::GfRange3d ViewportLocatorDelegate::GetExtent(PXR_NS::SdfPath const& id)
{
    auto converted = convert_index_path_to_stage_path(id);
    auto prim = m_cur_stage->GetPrimAtPath(converted);
    if (!prim)
        return HdSceneDelegate::GetExtent(converted);

    auto locator_it = m_locators.find(converted.GetPrimPath());
    if (locator_it != m_locators.end())
    {
        return locator_it->second->get_locator_item()->bbox();
    }

    return HdSceneDelegate::GetExtent(converted);
}

GfMatrix4d ViewportLocatorDelegate::GetTransform(PXR_NS::SdfPath const& id)
{
    auto converted = convert_index_path_to_stage_path(id);
    UsdPrim locator;
    if (!m_cur_stage || !(locator = m_cur_stage->GetPrimAtPath(converted)))
        return GfMatrix4d().SetIdentity();

    UsdGeomXformCache cache(m_time);
    auto locator_it = m_locators.find(converted.GetPrimPath());
    if (locator_it != m_locators.end())
        return UsdGeomXformCache(m_time).GetLocalToWorldTransform(locator);
    return GfMatrix4d(1);
}

bool ViewportLocatorDelegate::GetVisible(PXR_NS::SdfPath const& id)
{
    if (!m_cur_stage)
        return false;

    auto converted = convert_index_path_to_stage_path(id);
    if (auto prim = m_cur_stage->GetPrimAtPath(converted))
    {
        if (UsdGeomCamera(prim) && !m_visibility_mask.is_visible(PrimVisibilityTypes->camera))
            return false;
#if PXR_VERSION <= 2108
        if (UsdLuxLight(prim) && !m_visibility_mask.is_visible(PrimVisibilityTypes->light))
            return false;
#else
        if ((UsdLuxBoundableLightBase(prim) || UsdLuxNonboundableLightBase(prim)) && !m_visibility_mask.is_visible(PrimVisibilityTypes->light))
            return false;
#endif
        auto token = TfToken("");

        if (m_invised_paths.find(converted) != m_invised_paths.end())
        {
            return false;
        }

        while (!prim.IsPseudoRoot())
        {
            if (prim.HasAttribute(HdTokens->visibility))
                prim.GetAttribute(HdTokens->visibility).Get(&token);

            if (token == "invisible" || !prim.IsActive())
            {
                return false;
            }
            prim = prim.GetParent();
        }
    }

    return true;
}

PXR_NS::HdPrimvarDescriptorVector ViewportLocatorDelegate::GetPrimvarDescriptors(PXR_NS::SdfPath const& id, PXR_NS::HdInterpolation interpolation)
{
    HdPrimvarDescriptorVector primvars;
    if (interpolation == HdInterpolationVertex)
    {
        primvars.emplace_back(HdTokens->points, interpolation, HdPrimvarRoleTokens->point);
    }
    else if (interpolation == HdInterpolation::HdInterpolationConstant)
    {
        primvars.emplace_back(HdTokens->displayColor, interpolation, HdPrimvarRoleTokens->color);
    }

    return primvars;
}

SdfPath ViewportLocatorDelegate::GetMaterialId(PXR_NS::SdfPath const& rprimId)
{
    auto iter = m_locators.find(convert_index_path_to_stage_path(rprimId));
    if (iter != m_locators.end())
    {
        return iter->second->get_material_id();
    }

    return HdSceneDelegate::GetMaterialId(rprimId);
}

#if PXR_VERSION >= 2002
VtValue ViewportLocatorDelegate::GetMaterialResource(const SdfPath& materialId)
{
    auto iter = m_locators.find(convert_index_path_to_stage_path(materialId).GetPrimPath());
    if (iter != m_locators.end())
        return VtValue(iter->second->get_material_resource());
    return HdSceneDelegate::GetMaterialResource(materialId);
}

#else
HdMaterialParamVector ViewportLocatorDelegate::GetMaterialParams(const SdfPath& materialId)
{
    auto iter = m_locators.find(convert_index_path_to_stage_path(materialId).GetPrimPath());
    if (iter != m_locators.end())
    {
        return iter->second->get_material_params();
    }
    return HdSceneDelegate::GetMaterialParams(materialId);
}

std::string ViewportLocatorDelegate::GetSurfaceShaderSource(const SdfPath& materialId)
{
    auto iter = m_locators.find(convert_index_path_to_stage_path(materialId).GetPrimPath());
    if (iter != m_locators.end())
    {
        return iter->second->get_surface_shader_source();
    }
    return HdSceneDelegate::GetSurfaceShaderSource(materialId);
}
#endif
#if PXR_VERSION < 2008
PXR_NS::HdTextureResource::ID ViewportLocatorDelegate::GetTextureResourceID(const PXR_NS::SdfPath& textureId)
{
    auto converted = convert_index_path_to_stage_path(textureId);
    auto iter = m_locators.find(converted.GetPrimPath());
    if (iter != m_locators.end())
        return iter->second->get_texture_resource_id(converted);

    return HdSceneDelegate::GetTextureResourceID(textureId);
}

PXR_NS::HdTextureResourceSharedPtr ViewportLocatorDelegate::GetTextureResource(const PXR_NS::SdfPath& textureId)
{
    auto converted = convert_index_path_to_stage_path(textureId);
    auto iter = m_locators.find(converted.GetPrimPath());
    if (iter != m_locators.end())
        return iter->second->get_texture_resource(converted);

    return HdSceneDelegate::GetTextureResource(textureId);
}
#endif

void ViewportLocatorDelegate::set_invised_paths(const std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash>& invised_paths)
{
    if (invised_paths == m_invised_paths)
    {
        return;
    }

    SdfPathVector changed_paths;
    std::set<SdfPath> new_sorted_invised_paths(invised_paths.begin(), invised_paths.end());
    std::set<SdfPath> old_sorted_invised_paths(m_invised_paths.begin(), m_invised_paths.end());
    std::set_symmetric_difference(new_sorted_invised_paths.begin(), new_sorted_invised_paths.end(), old_sorted_invised_paths.begin(),
                                  old_sorted_invised_paths.end(), std::back_inserter(changed_paths));

    SdfPath::RemoveDescendentPaths(&changed_paths);
    m_invised_paths = invised_paths;
    for (const auto& subtree : changed_paths)
    {
        SdfPathVector affected_paths;
        HdPrimGather gather;
        gather.Subtree(m_prim_ids.GetIds(), subtree, &affected_paths);
        for (const auto& path : affected_paths)
        {
            GetRenderIndex().GetChangeTracker().MarkRprimDirty(convert_stage_path_to_index_path(path), HdChangeTracker::DirtyVisibility);
        }
    }
}

PXR_NS::VtValue ViewportLocatorDelegate::Get(PXR_NS::SdfPath const& id, PXR_NS::TfToken const& key)
{
    auto converted = convert_index_path_to_stage_path(id);
    if (key == HdTokens->points)
    {
        auto locator_it = m_locators.find(converted.GetPrimPath());
        if (locator_it != m_locators.end())
        {
            return VtValue(locator_it->second->get_locator_item()->vertex_positions());
        }
    }
    else if (key == HdTokens->displayColor)
    {
        const auto& prim = m_cur_stage->GetPrimAtPath(converted);
        if (prim.HasAttribute(TfToken("color")))
        {
            GfVec3f color;
            prim.GetAttribute(TfToken("color")).Get(&color);
            return VtValue(GfVec4f(color[0], color[1], color[2], 1));
        }
        else
        {
            return VtValue(GfVec4f(1, 1, 1, 1));
        }
    }

    return ViewportSceneDelegate::Get(id, key);
}

PXR_INTERNAL_NS::HdMeshTopology ViewportLocatorDelegate::GetMeshTopology(SdfPath const& id)
{
    auto locator_it = m_locators.find(convert_index_path_to_stage_path(id).GetPrimPath());
    if (locator_it != m_locators.end())
    {
        return HdMeshTopology(HdTokens->linear, HdTokens->leftHanded, locator_it->second->get_locator_item()->vertex_per_curve(),
                              locator_it->second->get_locator_item()->vertex_indexes());
    }
    return HdMeshTopology();
}

PXR_NS::HdBasisCurvesTopology ViewportLocatorDelegate::GetBasisCurvesTopology(PXR_NS::SdfPath const& id)
{
    auto locator_it = m_locators.find(convert_index_path_to_stage_path(id).GetPrimPath());
    if (locator_it != m_locators.end())
    {
        return HdBasisCurvesTopology(HdTokens->linear, HdTokens->linear, locator_it->second->get_locator_item()->topology(),
                                     locator_it->second->get_locator_item()->vertex_per_curve(),
                                     locator_it->second->get_locator_item()->vertex_indexes());
    }
    return HdBasisCurvesTopology();
}

void ViewportLocatorDelegate::update(const ViewportHydraEngineParams& engine_params)
{
    static std::mutex sync;
    std::lock_guard<std::mutex> lock(sync);
    m_main_render_index = engine_params.main_render_index;

    UsdStageRefPtr current_stage;
    UsdTimeCode time;

    if (engine_params.stage_resolver)
    {
        current_stage = engine_params.stage_resolver->get_stage(GetDelegateID().GetParentPath());
        time = engine_params.stage_resolver->resolve_time(GetDelegateID().GetParentPath(), engine_params.frame.GetValue());
    }
    else
    {
        current_stage = Application::instance().get_session()->get_current_stage();
        time = engine_params.frame;
    }

    if (!current_stage)
        return;

    if (m_time != time)
    {
        m_time = time;
        for (const auto& locator : m_time_varying_locators)
        {
            m_locators_data->update(locator, Application::instance().get_current_time());
            m_locators_data->mark_locator_dirty(locator, HdChangeTracker::AllDirty);
        }
    }
    if (m_cur_stage != current_stage)
    {
        m_prim_ids.Clear();
        m_cur_stage = current_stage;
        m_watcher = nullptr;
        m_locators.clear();
        m_locators_data = nullptr;
    }
    if (engine_params.visibility_mask.is_dirty())
    {
        m_visibility_mask = engine_params.visibility_mask;
        GetRenderIndex().GetChangeTracker().MarkAllRprimsDirty(HdChangeTracker::DirtyVisibility);
    }

    if (engine_params.invised_paths_dirty)
        set_invised_paths(engine_params.invised_paths);
    if (m_watcher)
    {
        while (!m_tasks.empty())
        {
            m_tasks.front()();
            m_tasks.pop();
        }
    }
    else
    {
        initialize(engine_params);
    }
}

void ViewportLocatorDelegate::initialize_locators(PXR_NS::UsdStageRefPtr current_stage)
{
    m_cur_stage = current_stage;

    if (!m_cur_stage)
    {
        m_locators_data = nullptr;
        return;
    }

    if (!m_locators_data)
    {
        m_locators_data = std::make_unique<UsdViewportLocatorData>(this);
    }
    m_locators_data->initialize_locators(m_time);
}

bool ViewportLocatorDelegate::GetDoubleSided(SdfPath const& id)
{
    auto locator_it = m_locators.find(convert_index_path_to_stage_path(id).GetPrimPath());
    if (locator_it != m_locators.end())
    {
        return locator_it->second->get_locator_item()->is_double_sided();
    }
    return false;
}

HdCullStyle ViewportLocatorDelegate::GetCullStyle(SdfPath const& id)
{
    return HdCullStyleBackUnlessDoubleSided;
}

HdReprSelector ViewportLocatorDelegate::GetReprSelector(SdfPath const& id)
{
    auto converted = convert_index_path_to_stage_path(id);
    return m_locators_data->contains_texture(converted) ? HdSceneDelegate::GetReprSelector(converted) : HdReprSelector(HdReprTokens->wire);
}

TfToken ViewportLocatorDelegate::GetRenderTag(SdfPath const& id)
{
    return TfToken("locator");
}

bool ViewportLocatorDelegate::is_locator(const PXR_NS::UsdPrim& prim) const
{
    return m_locators_data ? m_locators_data->is_locator(prim.GetPrimPath()) : false;
}

OPENDCC_NAMESPACE_CLOSE
