// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <usd_fallback_proxy/core/usd_fallback_proxy_watcher.h>
#include <usd_fallback_proxy/core/usd_prim_fallback_proxy.h>
#include <usd_fallback_proxy/core/source_registry.h>
#include <pxr/usd/usd/stage.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

static const std::string INVALID_PROXY = "invalid_proxy";

struct UsdFallbackProxyWatcher::HandleType
{
    SdfPath m_prim_path;
    UsdStageCache::Id m_stage_id;
    explicit HandleType(const SdfPath& prim_path, UsdStageCache::Id stage_id)
        : m_prim_path(prim_path)
        , m_stage_id(stage_id)
    {
    }
};

UsdFallbackProxyWatcher::PrimFallbackProxyChangedHandle UsdFallbackProxyWatcher::register_prim_fallback_proxy(const UsdPrim& usd_prim)
{
    if (!usd_prim)
        return nullptr;

    auto stage = usd_prim.GetStage();
    if (!stage)
        return nullptr;

    auto& stage_cache = UsdUtilsStageCache::Get();

    auto& instance = get_instance();
    auto stage_id = stage_cache.GetId(stage);
    auto prim_path = usd_prim.GetPath();
    if (instance.m_per_stage_notice_keys.find(stage_id) == instance.m_per_stage_notice_keys.end())
    {
        instance.m_per_stage_notice_keys[stage_id] =
            TfNotice::Register(TfWeakPtr<UsdFallbackProxyWatcher>(&instance), &UsdFallbackProxyWatcher::on_prim_changed, stage);
    }

    auto& counter = instance.m_per_stage_prims[stage_id][prim_path];
    if (counter.expired())
    {
        auto handle = instance.make_handle(prim_path, stage_id);
        counter = handle;
        return handle;
    }

    return counter.lock();
}

UsdFallbackProxyWatcher::InvalidProxyDispatcherHandle UsdFallbackProxyWatcher::register_invalid_proxy_callback(
    const std::function<void(const std::vector<UsdPrimFallbackProxy>&)>& callback)
{
    return get_instance().m_invalid_proxy_dispatcher.appendListener(INVALID_PROXY, callback);
}

void UsdFallbackProxyWatcher::unregister_invalid_proxy_callback(const InvalidProxyDispatcherHandle& handle)
{
    get_instance().m_invalid_proxy_dispatcher.removeListener(INVALID_PROXY, handle);
}

void UsdFallbackProxyWatcher::on_prim_changed(const UsdNotice::ObjectsChanged& notice, const UsdStageWeakPtr& sender)
{
    auto& instance = get_instance();
    auto& stage_cache = UsdUtilsStageCache::Get();
    auto stage_id = stage_cache.GetId(sender);
    auto stage_iter = instance.m_per_stage_prims.find(stage_id);

    if (stage_iter == m_per_stage_prims.end())
    {
        unregister_stage_notice(stage_id);
        return;
    }

    struct ChangedInfo
    {
        TfTokenVector resynced_properties;
        TfTokenVector changed_properties;
    };

    std::unordered_map<SdfPath, ChangedInfo, SdfPath::Hash> changed_info;
    for (const auto& resynced_path : notice.GetResyncedPaths())
    {
        if (!resynced_path.IsPropertyPath())
            continue;
        auto resynced_prim_path = resynced_path.GetPrimPath();
        const auto& resynced_property_name = resynced_path.GetNameToken();
        if (stage_iter->second.find(resynced_prim_path) != stage_iter->second.end())
        {
            changed_info[resynced_prim_path].resynced_properties.push_back(resynced_property_name);
        }
    }

    for (const auto& changed_field_path : notice.GetChangedInfoOnlyPaths())
    {
        if (!changed_field_path.IsPropertyPath())
            continue;

        auto changed_prim_path = changed_field_path.GetPrimPath();
        const auto& changed_property_name = changed_field_path.GetNameToken();
        if (stage_iter->second.find(changed_prim_path) != stage_iter->second.end())
        {
            changed_info[changed_prim_path].changed_properties.push_back(changed_property_name);
        }
    }

    std::vector<UsdPrimFallbackProxy> invalid_prim_proxies;
    for (auto& prim_info : changed_info)
    {
        auto prim = sender->GetPrimAtPath(prim_info.first);
        auto props = SourceRegistry::get_property_proxies(prim);
        auto is_invalid = false;
        for (const auto& resync : prim_info.second.resynced_properties)
        {
            if (std::find_if_not(props.begin(), props.end(),
                                 [resync](const UsdPropertyProxyPtr& prop) { return prop->get_name_token() == resync; }) == props.end())
            {
                invalid_prim_proxies.emplace_back(prim);
                is_invalid = true;
                break;
            }
        }
        if (is_invalid)
            continue;

        if (SourceRegistry::is_prim_proxy_outdated(prim, prim_info.second.resynced_properties, prim_info.second.changed_properties))
        {
            invalid_prim_proxies.emplace_back(prim);
        }
    }

    if (invalid_prim_proxies.empty())
        return;

    m_invalid_proxy_dispatcher.dispatch(INVALID_PROXY, invalid_prim_proxies);
}

void UsdFallbackProxyWatcher::unregister_stage_notice(UsdStageCache::Id stage_id)
{
    auto notice_key_iter = m_per_stage_notice_keys.find(stage_id);
    if (notice_key_iter != m_per_stage_notice_keys.end())
    {
        TfNotice::Revoke(notice_key_iter->second);
        m_per_stage_notice_keys.erase(notice_key_iter);
    }
}

UsdFallbackProxyWatcher& UsdFallbackProxyWatcher::get_instance()
{
    static UsdFallbackProxyWatcher instance;
    return instance;
}

UsdFallbackProxyWatcher::PrimFallbackProxyChangedHandle UsdFallbackProxyWatcher::make_handle(const SdfPath& prim_path, UsdStageCache::Id stage_id)
{
    return std::shared_ptr<HandleType>(new HandleType(prim_path, stage_id), [this](HandleType* handle) {
        auto& stage_cache = UsdUtilsStageCache::Get();
        auto stage_id = handle->m_stage_id;
        auto stage = stage_cache.Find(stage_id);
        if (stage)
        {
            auto stage_iter = m_per_stage_prims.find(stage_id);
            if (stage_iter != m_per_stage_prims.end())
            {
                auto prim_iter = stage_iter->second.find(handle->m_prim_path);
                if (prim_iter != stage_iter->second.end() && prim_iter->second.expired())
                {
                    stage_iter->second.erase(prim_iter);
                    if (stage_iter->second.empty())
                    {
                        unregister_stage_notice(stage_id);
                        m_per_stage_prims.erase(stage_iter);
                    }
                }
            }
            else
            {
                unregister_stage_notice(stage_id);
            }
        }
        else
        {
            unregister_stage_notice(stage_id);
            m_per_stage_prims.erase(stage_id);
        }

        delete handle;
    });
}

OPENDCC_NAMESPACE_CLOSE