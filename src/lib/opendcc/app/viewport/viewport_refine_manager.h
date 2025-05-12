/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/pxr.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/common.h>
#include <pxr/usd/usd/stageCache.h>
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"

#include <unordered_map>
#include <mutex>

OPENDCC_NAMESPACE_OPEN

template <typename RefineLevelCallbackType, typename StageClearedCallbackType>
class ViewportRefineManager
{
public:
    enum class EventType
    {
        REFINE_LEVEL_CHANGED,
        STAGE_CLEARED
    };

    using RefineLevelDispatcher = eventpp::EventDispatcher<EventType, RefineLevelCallbackType>;
    using RefineLevelDispatcherHandle = typename RefineLevelDispatcher::Handle;
    using StageClearedDispatcher = eventpp::EventDispatcher<EventType, StageClearedCallbackType>;
    using StageClearedDispatcherHandle = typename StageClearedDispatcher::Handle;

    virtual ~ViewportRefineManager() = default;

    RefineLevelDispatcherHandle register_refine_level_changed_callback(const std::function<RefineLevelCallbackType>& callback)
    {
        Lock lock(m_mutex);
        return m_refine_level_dispatcher.appendListener(EventType::REFINE_LEVEL_CHANGED, callback);
    }

    StageClearedDispatcherHandle register_stage_cleared_callback(const std::function<StageClearedCallbackType>& callback)
    {
        Lock lock(m_mutex);
        return m_stage_cleared_dispatcher.appendListener(EventType::STAGE_CLEARED, callback);
    }

    void unregister_refine_level_changed_callback(const RefineLevelDispatcherHandle& handle)
    {
        Lock lock(m_mutex);
        m_refine_level_dispatcher.removeListener(EventType::REFINE_LEVEL_CHANGED, handle);
    }

    void unregister_stage_cleared_callback(const StageClearedDispatcherHandle& handle)
    {
        Lock lock(m_mutex);
        m_stage_cleared_dispatcher.removeListener(EventType::STAGE_CLEARED, handle);
    }

protected:
    using Lock = std::lock_guard<std::recursive_mutex>;

    ViewportRefineManager() = default;

    mutable std::recursive_mutex m_mutex;

    RefineLevelDispatcher m_refine_level_dispatcher;
    StageClearedDispatcher m_stage_cleared_dispatcher;
};

class UsdViewportRefineManager
    : public ViewportRefineManager<void(const PXR_NS::UsdStageCache::Id&, const PXR_NS::SdfPath&, int), void(const PXR_NS::UsdStageCache::Id&)>
{
public:
    OPENDCC_API static UsdViewportRefineManager& instance();

    OPENDCC_API void set_refine_level(const PXR_NS::UsdStageRefPtr& stage, const PXR_NS::SdfPath& prim_path, int refine_level);
    OPENDCC_API void set_refine_level(const PXR_NS::UsdStageCache::Id& stage_id, const PXR_NS::SdfPath& prim_path, int refine_level);
    OPENDCC_API int get_refine_level(const PXR_NS::UsdStageRefPtr& stage, const PXR_NS::SdfPath& prim_path) const;
    OPENDCC_API int get_refine_level(const PXR_NS::UsdStageCache::Id& stage_id, const PXR_NS::SdfPath& prim_path) const;
    OPENDCC_API void clear_all();
    OPENDCC_API void clear_stage(const PXR_NS::UsdStageRefPtr& stage);
    OPENDCC_API void clear_stage(const PXR_NS::UsdStageCache::Id& stage_id);
    OPENDCC_API void clear_refine_level(const PXR_NS::UsdStageRefPtr& stage, const PXR_NS::SdfPath& prim_path);
    OPENDCC_API void clear_refine_level(const PXR_NS::UsdStageCache::Id& stage_id, const PXR_NS::SdfPath& prim_path);

    ~UsdViewportRefineManager() = default;

private:
    using PerPrimitiveRefines = std::unordered_map<PXR_NS::SdfPath, int, PXR_NS::SdfPath::Hash>;
    using PerStageRefines = std::unordered_map<PXR_NS::UsdStageCache::Id, PerPrimitiveRefines, PXR_NS::TfHash>;

    UsdViewportRefineManager() = default;

    PerStageRefines m_refines;
};

using UsdViewportRefineManagerBase =
    ViewportRefineManager<void(const PXR_NS::UsdStageCache::Id&, const PXR_NS::SdfPath&, int), void(const PXR_NS::UsdStageCache::Id&)>;

using UsdRefineHandle = UsdViewportRefineManagerBase::RefineLevelDispatcherHandle;
using UsdStageClearedHandle = UsdViewportRefineManagerBase::StageClearedDispatcherHandle;

OPENDCC_NAMESPACE_CLOSE
