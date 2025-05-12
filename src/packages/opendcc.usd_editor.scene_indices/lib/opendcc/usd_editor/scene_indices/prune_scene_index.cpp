// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/scene_indices/prune_scene_index.h"
#include "opendcc/app/viewport/scene_indices/hydra_engine_scene_indices_notifier.h"
#include <opendcc/base/logging/logger.h>
#include "opendcc/usd_editor/scene_indices/utils.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    bool is_prunned_impl(const std::vector<SdfPath> &sorted_exclude_paths, const SdfPath &target)
    {
        auto rit = std::lower_bound(sorted_exclude_paths.rbegin(), sorted_exclude_paths.rend(), target, std::greater<SdfPath>());
        return rit != sorted_exclude_paths.rend() && target.HasPrefix(*rit);
    }
}

template <class T>
inline T PruneSceneIndex::prune_entries(const T &source_entries) const
{
    OPENDCC_ASSERT(m_predicate);
    T result;
    std::copy_if(source_entries.begin(), source_entries.end(), std::back_inserter(result),
                 [this](const auto &entry) { return !is_prunned_impl(m_prunned_prefixes, entry.primPath); });

    return result;
}

PruneSceneIndex::PruneSceneIndex(const PXR_NS::HdSceneIndexBaseRefPtr &input_scene_index)
    : HdSingleInputFilteringSceneIndexBase(input_scene_index)
{
}

bool PruneSceneIndex::is_prunned(const PXR_NS::SdfPath &prim_path) const
{
    return !m_prunned_prefixes.empty() && is_prunned_impl(m_prunned_prefixes, prim_path);
}

PXR_NS::HdSceneIndexPrim PruneSceneIndex::GetPrim(const PXR_NS::SdfPath &prim_path) const
{
    const auto prim = _GetInputSceneIndex()->GetPrim(prim_path);
    if (is_prunned(prim_path))
    {
        return { TfToken(), nullptr };
    }

    return prim;
}

PXR_NS::SdfPathVector PruneSceneIndex::GetChildPrimPaths(const PXR_NS::SdfPath &prim_path) const
{
    if (is_prunned(prim_path))
    {
        return {};
    }

    auto prunned_children = _GetInputSceneIndex()->GetChildPrimPaths(prim_path);

    if (m_predicate)
    {
        prunned_children.erase(std::remove_if(prunned_children.begin(), prunned_children.end(),
                                              [this](const SdfPath &p) { return is_prunned_impl(m_prunned_prefixes, p); }),
                               prunned_children.end());
    }
    return prunned_children;
}

void PruneSceneIndex::set_predicate(const PruneSceneIndex::PrunePredicate &predicate)
{
    const auto old_predicate = std::move(m_predicate);
    // Set new predicate before we call _SendPrimsAdded which might
    // make client scene indices pull on this scene index.
    m_predicate = predicate;
    m_prunned_prefixes.clear();

    HdSceneIndexObserver::AddedPrimEntries addedEntries;
    HdSceneIndexObserver::RemovedPrimEntries removedEntries;
    auto view = PrimView(_GetInputSceneIndex());
    for (auto it = view.begin(); it != view.end(); ++it)
    {
        const auto &prim_path = *it;

        const bool old_value = old_predicate && old_predicate(prim_path);
        const bool new_value = m_predicate && m_predicate(prim_path);
        if (old_value == new_value)
        {
            continue;
        }

        // if prim was created
        if (old_value)
        {
            for (const auto &subprim : PrimView(_GetInputSceneIndex(), prim_path))
            {
                const auto should_prune = m_predicate && m_predicate(subprim);
                if (!should_prune)
                {
                    TfToken primType = _GetInputSceneIndex()->GetPrim(subprim).primType;
                    addedEntries.emplace_back(subprim, primType);
                }
            }
        }
        else
        {
            removedEntries.emplace_back(prim_path);
            m_prunned_prefixes.emplace_back(prim_path);
        }
        it.SkipDescendants();
    }

    std::sort(m_prunned_prefixes.begin(), m_prunned_prefixes.end());

    if (!addedEntries.empty())
    {
        _SendPrimsAdded(addedEntries);
    }
    if (!removedEntries.empty())
    {
        _SendPrimsRemoved(removedEntries);
    }
}

void PruneSceneIndex::_PrimsAdded(const PXR_NS::HdSceneIndexBase &sender, const PXR_NS::HdSceneIndexObserver::AddedPrimEntries &entries)
{
    if (!_IsObserved())
    {
        return;
    }

    if (!m_predicate)
    {
        _SendPrimsAdded(entries);
        return;
    }

    _SendPrimsAdded(prune_entries(entries));
}

void PruneSceneIndex::_PrimsRemoved(const PXR_NS::HdSceneIndexBase &sender, const PXR_NS::HdSceneIndexObserver::RemovedPrimEntries &entries)
{
    if (!_IsObserved())
    {
        return;
    }

    if (!m_predicate)
    {
        _SendPrimsRemoved(entries);
        return;
    }

    _SendPrimsRemoved(prune_entries(entries));
}

void PruneSceneIndex::_PrimsDirtied(const PXR_NS::HdSceneIndexBase &sender, const PXR_NS::HdSceneIndexObserver::DirtiedPrimEntries &entries)
{
    if (!_IsObserved())
    {
        return;
    }

    if (!m_predicate)
    {
        _SendPrimsDirtied(entries);
        return;
    }

    _SendPrimsDirtied(prune_entries(entries));
}

OPENDCC_NAMESPACE_CLOSE
