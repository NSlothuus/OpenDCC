// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_refine_manager.h"

#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"
#include "opendcc/app/core/settings.h"

OPENDCC_NAMESPACE_OPEN

void UsdViewportRefineManager::clear_refine_level(const PXR_NS::UsdStageRefPtr& stage, const PXR_NS::SdfPath& prim_path)
{
    clear_refine_level(Application::instance().get_session()->get_stage_id(stage), prim_path);
}

void UsdViewportRefineManager::clear_stage(const PXR_NS::UsdStageRefPtr& stage)
{
    clear_stage(Application::instance().get_session()->get_stage_id(stage));
}

int UsdViewportRefineManager::get_refine_level(const PXR_NS::UsdStageRefPtr& stage, const PXR_NS::SdfPath& prim_path) const
{
    return get_refine_level(Application::instance().get_session()->get_stage_id(stage), prim_path);
}

void UsdViewportRefineManager::set_refine_level(const PXR_NS::UsdStageRefPtr& stage, const PXR_NS::SdfPath& prim_path, int refine_level)
{
    set_refine_level(Application::instance().get_session()->get_stage_id(stage), prim_path, refine_level);
}

void UsdViewportRefineManager::set_refine_level(const PXR_NS::UsdStageCache::Id& stage_id, const PXR_NS::SdfPath& prim_path, int refine_level)
{
    Lock lock(m_mutex);
    if (!stage_id.IsValid() || refine_level < 0 || refine_level > 8)
    {
        return;
    }

    m_refines[stage_id][prim_path] = refine_level;
    m_refine_level_dispatcher.dispatch(EventType::REFINE_LEVEL_CHANGED, stage_id, prim_path, refine_level);
}

int UsdViewportRefineManager::get_refine_level(const PXR_NS::UsdStageCache::Id& stage_id, const PXR_NS::SdfPath& prim_path) const
{
    if (!stage_id.IsValid())
    {
        return 0;
    }
    const auto stage_iter = m_refines.find(stage_id);
    if (stage_iter != m_refines.end())
    {
        const auto& per_stage_map = stage_iter->second;
        auto prim_iter = per_stage_map.find(prim_path);
        if (prim_iter != per_stage_map.end())
        {
            return prim_iter->second;
        }
        else
        {
            auto parent = prim_path;
            while (!parent.IsEmpty())
            {
                parent = parent.GetParentPath();
                const auto found_iter = per_stage_map.find(parent);
                if (found_iter != per_stage_map.end())
                {
                    return found_iter->second;
                }
            }
        }
    }

    return 0;
}

void UsdViewportRefineManager::clear_stage(const PXR_NS::UsdStageCache::Id& stage_id)
{
    Lock lock(m_mutex);
    if (!stage_id.IsValid())
    {
        return;
    }

    auto iter = m_refines.find(stage_id);
    if (iter != m_refines.end())
    {
        m_refines.erase(iter);
        m_stage_cleared_dispatcher.dispatch(EventType::STAGE_CLEARED, iter->first);
    }
}

void UsdViewportRefineManager::clear_all()
{
    Lock lock(m_mutex);
    for (auto iter : m_refines)
    {
        m_refines.erase(iter.first);
        m_stage_cleared_dispatcher.dispatch(EventType::STAGE_CLEARED, iter.first);
    }
}

void UsdViewportRefineManager::clear_refine_level(const PXR_NS::UsdStageCache::Id& stage_id, const PXR_NS::SdfPath& prim_path)
{
    Lock lock(m_mutex);
    if (!stage_id.IsValid())
    {
        return;
    }
    auto stage_iter = m_refines.find(stage_id);
    if (stage_iter != m_refines.end())
    {
        auto& per_stage_map = stage_iter->second;
        per_stage_map.erase(prim_path);
        m_refine_level_dispatcher.dispatch(EventType::REFINE_LEVEL_CHANGED, stage_id, prim_path, 0);
    }
}

UsdViewportRefineManager& UsdViewportRefineManager::instance()
{
    static UsdViewportRefineManager instance;
    return instance;
}
OPENDCC_NAMESPACE_CLOSE
