/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <usd_fallback_proxy/core/api.h>
#include <pxr/usd/usd/notice.h>
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"
#include <pxr/usd/usdUtils/stageCache.h>

OPENDCC_NAMESPACE_OPEN

class UsdPrimFallbackProxy;

class FALLBACK_PROXY_API UsdFallbackProxyWatcher : public PXR_NS::TfWeakBase
{
private:
    struct HandleType;

public:
    using PrimFallbackProxyChangedHandle = std::shared_ptr<HandleType>;
    using InvalidProxyDispatcher = eventpp::EventDispatcher<std::string, void(const std::vector<UsdPrimFallbackProxy>&)>;
    using InvalidProxyDispatcherHandle = typename InvalidProxyDispatcher::Handle;

    static PrimFallbackProxyChangedHandle register_prim_fallback_proxy(const PXR_NS::UsdPrim& usd_prim);
    static InvalidProxyDispatcherHandle register_invalid_proxy_callback(
        const std::function<void(const std::vector<UsdPrimFallbackProxy>&)>& callback);

    static void unregister_invalid_proxy_callback(const InvalidProxyDispatcherHandle& handle);

private:
    using PerPrimCounter = std::unordered_map<PXR_NS::SdfPath, std::weak_ptr<HandleType>, PXR_NS::SdfPath::Hash>;
    using PerStagePrims = std::unordered_map<PXR_NS::UsdStageCache::Id, PerPrimCounter, PXR_NS::TfHash>;
    using PerStageNotice = std::unordered_map<PXR_NS::UsdStageCache::Id, PXR_NS::TfNotice::Key, PXR_NS::TfHash>;

    UsdFallbackProxyWatcher() = default;
    UsdFallbackProxyWatcher(const UsdFallbackProxyWatcher&) = delete;
    UsdFallbackProxyWatcher& operator=(const UsdFallbackProxyWatcher&) = delete;

    void on_prim_changed(const PXR_NS::UsdNotice::ObjectsChanged& notice, const PXR_NS::UsdStageWeakPtr& sender);
    void unregister_stage_notice(PXR_NS::UsdStageCache::Id stage_id);

    PrimFallbackProxyChangedHandle make_handle(const PXR_NS::SdfPath& prim_path_path, PXR_NS::UsdStageCache::Id stage_id);
    static UsdFallbackProxyWatcher& get_instance();

    PerStageNotice m_per_stage_notice_keys;
    PerStagePrims m_per_stage_prims;

    InvalidProxyDispatcher m_invalid_proxy_dispatcher;
};
OPENDCC_NAMESPACE_CLOSE
