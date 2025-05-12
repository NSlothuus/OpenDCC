// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/scene_indices/isolate_scene_index.h"
#include "opendcc/usd_editor/scene_indices/utils.h"
#include "opendcc/app/viewport/scene_indices/hydra_engine_scene_indices_notifier.h"
#include <opendcc/base/logging/logger.h>

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
inline T IsolateSceneIndex::prune_entries(const T &source_entries) const
{
    OPENDCC_ASSERT(m_predicate);
    T result;
    std::copy_if(source_entries.begin(), source_entries.end(), std::back_inserter(result),
                 [this](const auto &entry) { return !is_prunned_impl(m_prunned_prefixes, entry.primPath); });

    return result;
}

IsolateSceneIndex::IsolateSceneIndex(const PXR_NS::HdSceneIndexBaseRefPtr &input_scene_index)
    : HdSingleInputFilteringSceneIndexBase(input_scene_index)
{
}

bool IsolateSceneIndex::is_prunned(const PXR_NS::SdfPath &prim_path) const
{
    return !m_prunned_prefixes.empty() && is_prunned_impl(m_prunned_prefixes, prim_path);
}

void IsolateSceneIndex::update_prunned_prefixes(const std::optional<PXR_NS::SdfPath> &isolate_from, const std::optional<IsolatePredicate> &predicate)
{
    HdSceneIndexObserver::AddedPrimEntries added_entries;
    HdSceneIndexObserver::RemovedPrimEntries removed_entries;

    std::vector<SdfPath> old_prune_prefixes;
    old_prune_prefixes.swap(m_prunned_prefixes);

    auto inp = _GetInputSceneIndex();

    // traverse subtree and remove from index paths that must be prunned,
    // update m_prunned_prefixes for those that mustn't be populated
    std::function<bool(const SdfPath &)> traverse_remove = [&traverse_remove, &removed_entries, inp, this](const SdfPath &path) {
        // check if current can be preserved, early return true
        if (!m_predicate || m_predicate(path))
        {
            return true;
        }

        bool should_preserve = false;
        std::vector<SdfPath> prune_candidates;
        for (const auto &child : inp->GetChildPrimPaths(path))
        {
            if (traverse_remove(child))
            {
                should_preserve = true;
            }
            else
            {
                prune_candidates.emplace_back(child);
            }
        }

        if (should_preserve)
        {
            removed_entries.insert(removed_entries.end(), prune_candidates.begin(), prune_candidates.end());
            m_prunned_prefixes.insert(m_prunned_prefixes.end(), prune_candidates.begin(), prune_candidates.end());
        }
        return should_preserve;
    };

    // traverse subtree and add to index paths that should be repopulated,
    // update m_prunned_prefixes for those that mustn't be populated
    std::function<bool(const SdfPath &)> traverse_add = [&traverse_add, &added_entries, inp, this](const SdfPath &path) {
        // check if current can be preserved, early return true
        if (!m_predicate || m_predicate(path))
        {
            auto view = PrimView(inp, path);
            auto skip_first = view.begin();
            for (auto it = ++skip_first; it != view.end(); ++it)
            {
                TfToken primType = inp->GetPrim(*it).primType;
                added_entries.emplace_back(*it, primType);
            }
            return true;
        }

        bool should_preserve = false;
        std::vector<SdfPath> prune_candidates;
        for (const auto &child : inp->GetChildPrimPaths(path))
        {
            if (traverse_add(child))
            {
                should_preserve = true;
                TfToken primType = inp->GetPrim(child).primType;
                added_entries.emplace_back(child, primType);
            }
            else
            {
                prune_candidates.emplace_back(child);
            }
        }

        if (should_preserve)
        {
            m_prunned_prefixes.insert(m_prunned_prefixes.end(), prune_candidates.begin(), prune_candidates.end());
        }
        return should_preserve;
    };

    // add whole subtree to index
    auto traverse_add_unconditionally = [inp, &added_entries](const SdfPath &path) {
        for (const auto &cur_path : PrimView(inp, path))
        {
            auto type = inp->GetPrim(cur_path).primType;
            added_entries.emplace_back(cur_path, type);
        }
    };

    if (predicate)
    {
        m_predicate = *predicate;
    }

    // handle isolate from changes
    if (isolate_from && m_isolate_from != *isolate_from)
    {
        auto old_isolate_from = m_isolate_from;
        m_isolate_from = *isolate_from;
        // isolate from changed, add all prims that were previously prunned
        if (!old_isolate_from.IsEmpty())
        {
            for (const auto &pref : old_prune_prefixes)
            {
                traverse_add_unconditionally(pref);
            }
        }

        if (!m_isolate_from.IsEmpty())
        {
            const auto view = PrimView(inp, m_isolate_from);
            auto skipped_root = view.begin();
            for (auto it = ++skipped_root; it != view.end(); ++it)
            {
                const auto &path = *it;
                if (!traverse_remove(path))
                {
                    m_prunned_prefixes.emplace_back(path);
                    removed_entries.emplace_back(path);
                }
                it.SkipDescendants();
            }
        }
    }

    // handle predicate changes
    // when we change predicate we must ensure that all constraints are satisfied
    if (predicate && !m_isolate_from.IsEmpty())
    {
        for (const auto &child : inp->GetChildPrimPaths(m_isolate_from))
        {
            const auto old_prunned = is_prunned_impl(old_prune_prefixes, child);
            const auto new_isolated = !m_predicate || m_predicate(child);

            if (old_prunned && !new_isolated)
            {
                // If path was previously prunned and it is not isolated,
                // we must traverse this subtree to find any subtrees that shouldn't be pruned with new predicate.
                // If there are nothing, then just update prunned prefixes.
                if (!traverse_add(child))
                {
                    m_prunned_prefixes.emplace_back(child);
                }
            }
            else if (old_prunned && new_isolated)
            {
                // If path was previously prunned and now it is isolated,
                // then all its children can't be prunned, so just add them to index
                traverse_add_unconditionally(child);
            }
            else if (!old_prunned && !new_isolated)
            {
                // If old path wasn't prunned and now it is not isolated, then
                // some of its subpath still could be isolated, we must traverse subtree and
                // find ones that should be prunned. If subtree doesn't contain any isolated paths,
                // then prune whole subtree
                if (!traverse_remove(child))
                {
                    m_prunned_prefixes.emplace_back(child);
                    removed_entries.emplace_back(child);
                }
            }
            else if (!old_prunned && new_isolated)
            {
                // If current path wasn't pruned before and it is isolated now,
                // then there are still might be some paths that were prunned before,
                // so we have to find them and add to index
                for (const auto child : inp->GetChildPrimPaths(child))
                {
                    const auto old_prunned = is_prunned_impl(old_prune_prefixes, child);
                    if (old_prunned)
                    {
                        traverse_add_unconditionally(child);
                    }
                }
            }
        }
    }

    SdfPath::RemoveDescendentPaths(&m_prunned_prefixes);

    if (!added_entries.empty())
    {
        _SendPrimsAdded(added_entries);
    }

    if (!removed_entries.empty())
    {
        _SendPrimsRemoved(removed_entries);
    }
}

PXR_NS::HdSceneIndexPrim IsolateSceneIndex::GetPrim(const PXR_NS::SdfPath &prim_path) const
{
    const auto prim = _GetInputSceneIndex()->GetPrim(prim_path);
    if (is_prunned(prim_path))
    {
        return { TfToken(), nullptr };
    }

    return prim;
}

PXR_NS::SdfPathVector IsolateSceneIndex::GetChildPrimPaths(const PXR_NS::SdfPath &prim_path) const
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

void IsolateSceneIndex::set_predicate(const IsolateSceneIndex::IsolatePredicate &predicate)
{
    update_prunned_prefixes({}, predicate);
}

void IsolateSceneIndex::set_isolate_from(const PXR_NS::SdfPath &isolate_from)
{
    update_prunned_prefixes(isolate_from, {});
}

void IsolateSceneIndex::set_args(const PXR_NS::SdfPath &isolate_from, const IsolatePredicate &predicate)
{
    update_prunned_prefixes(isolate_from, predicate);
}

void IsolateSceneIndex::_PrimsAdded(const PXR_NS::HdSceneIndexBase &sender, const PXR_NS::HdSceneIndexObserver::AddedPrimEntries &entries)
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

void IsolateSceneIndex::_PrimsRemoved(const PXR_NS::HdSceneIndexBase &sender, const PXR_NS::HdSceneIndexObserver::RemovedPrimEntries &entries)
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

void IsolateSceneIndex::_PrimsDirtied(const PXR_NS::HdSceneIndexBase &sender, const PXR_NS::HdSceneIndexObserver::DirtiedPrimEntries &entries)
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
