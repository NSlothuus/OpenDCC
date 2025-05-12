// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "terminal_scene_index.h"
#include <pxr/base/work/dispatcher.h>

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

using _RemovedPrimEntryQueue = tbb::concurrent_queue<HdSceneIndexObserver::RemovedPrimEntry>;
using _AddedPrimEntryQueue = tbb::concurrent_queue<HdSceneIndexObserver::AddedPrimEntry>;
using _DirtiedPrimEntryQueue = tbb::concurrent_queue<HdSceneIndexObserver::DirtiedPrimEntry>;

static void fill_added_child_entries_in_parallel(WorkDispatcher* dispatcher, const HdSceneIndexBaseRefPtr& sceneIndex, const SdfPath& path,
                                                 _AddedPrimEntryQueue* queue)
{
    queue->emplace(path, sceneIndex->GetPrim(path).primType);
    for (const SdfPath& childPath : sceneIndex->GetChildPrimPaths(path))
    {
        dispatcher->Run([=]() { fill_added_child_entries_in_parallel(dispatcher, sceneIndex, childPath, queue); });
    }
}

void HdsiComputeSceneIndexDiffRoot(const HdSceneIndexBaseRefPtr& siA, const HdSceneIndexBaseRefPtr& siB,
                                   HdSceneIndexObserver::RemovedPrimEntries* removedEntries, HdSceneIndexObserver::AddedPrimEntries* addedEntries,
                                   HdSceneIndexObserver::RenamedPrimEntries* renamedEntries, HdSceneIndexObserver::DirtiedPrimEntries* dirtiedEntries)
{
    if (siA)
    {
        removedEntries->emplace_back(SdfPath::AbsoluteRootPath());
    }

    if (siB)
    {
        WorkDispatcher dispatcher;
        _AddedPrimEntryQueue queue;
        fill_added_child_entries_in_parallel(&dispatcher, siB, SdfPath::AbsoluteRootPath(), &queue);
        dispatcher.Wait();
        addedEntries->insert(addedEntries->end(), queue.unsafe_begin(), queue.unsafe_end());
    }
}

// Given sorted ranges A [first1, last1) and B [first2, last2),
// this will write all elements in A^B into `d_firstBoth`, A-B into `d_first1`,
// and B-A into `d_first2`.
template <typename InputIt1, typename InputIt2, typename OutputIt>
void set_intersection_and_diff(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutputIt d_firstBoth, OutputIt d_first1,
                               OutputIt d_first2)
{
    while ((first1 != last1) && (first2 != last2))
    {
        if (*first1 < *first2)
        {
            // element is in A only
            *d_first1++ = *first1++;
        }
        else if (*first2 < *first1)
        {
            // element is in B only
            *d_first2++ = *first2++;
        }
        else
        {
            // element is in both
            *d_firstBoth++ = *first1++;
            first2++;
        }
    }

    // We've run out of elements in at least one of the input ranges.
    // Copy whatever may be left into the appropriate output.
    std::copy(first1, last1, d_first1);
    std::copy(first2, last2, d_first2);
}

static std::vector<SdfPath> get_sorted_child_paths(const HdSceneIndexBaseRefPtr& si, const SdfPath& path)
{
    std::vector<SdfPath> ret = si->GetChildPrimPaths(path);
    // XXX(edluong): could provide API to get these already sorted..
    std::sort(ret.begin(), ret.end());
    return ret;
}

bool deep_equals(const HdDataSourceBaseHandle& a, const HdDataSourceBaseHandle& b)
{
    std::function<bool(const HdDataSourceBaseHandle&, const HdDataSourceBaseHandle&)> traverse =
        [&traverse](const HdDataSourceBaseHandle& a_h, const HdDataSourceBaseHandle& b_h) -> bool {
        if (a_h == b_h)
        {
            return true;
        }

        auto a_cont = HdContainerDataSource::Cast(a_h);
        auto b_cont = HdContainerDataSource::Cast(b_h);
        if (a_cont && b_cont)
        {
            auto a_names = a_cont->GetNames();
            auto b_names = b_cont->GetNames();
            std::sort(a_names.begin(), a_names.end());
            std::sort(b_names.begin(), b_names.end());
            if (a_names != b_names)
            {
                return false;
            }

            for (const auto& n : a_names)
            {
                auto traverse_result = traverse(a_cont->Get(n), b_cont->Get(n));
                if (!traverse_result)
                {
                    return false;
                }
            }
            return true;
        }

        auto a_vec = HdVectorDataSource::Cast(a_h);
        auto b_vec = HdVectorDataSource::Cast(b_h);
        if (a_vec && b_vec)
        {
            if (a_vec->GetNumElements() != b_vec->GetNumElements())
            {
                for (int i = 0; i < a_vec->GetNumElements(); ++i)
                {
                    auto traverse_result = traverse(a_vec->GetElement(i), b_vec->GetElement(i));
                    if (!traverse_result)
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        auto a_samp = HdSampledDataSource::Cast(a_h);
        auto b_samp = HdSampledDataSource::Cast(b_h);
        if (a_samp && b_samp)
        {
            auto a_val = a_samp->GetValue(0.0f);
            auto b_val = b_samp->GetValue(0.0f);

            return a_val == b_val;
        }

        return false;
    };

    return traverse(a, b);
}

static void compute_delta_diff_helper(WorkDispatcher* dispatcher, const HdSceneIndexBaseRefPtr& siA, const HdSceneIndexBaseRefPtr& siB,
                                      const SdfPath& commonPath, _RemovedPrimEntryQueue* removedEntries, _AddedPrimEntryQueue* addedEntries,
                                      _DirtiedPrimEntryQueue* dirtiedEntries)
{
    const HdSceneIndexPrim primA = siA->GetPrim(commonPath);
    const HdSceneIndexPrim primB = siB->GetPrim(commonPath);

    if (primA.primType == primB.primType)
    {
        // if (!deep_equals(primA.dataSource, primB.dataSource))
        if (primA.dataSource != primB.dataSource)
        {
            // append to dirtiedEntries
            dirtiedEntries->emplace(commonPath, HdDataSourceLocatorSet::UniversalSet());
        }
    }
    else
    {
        // mark as added.  downstream clients should know to resync this.
        addedEntries->emplace(commonPath, primB.primType);
    }

    const std::vector<SdfPath> aPaths = get_sorted_child_paths(siA, commonPath);
    const std::vector<SdfPath> bPaths = get_sorted_child_paths(siB, commonPath);

    // For a common path, we are more likely to also have common children so
    // this is optimized for that.
    std::vector<SdfPath> sharedChildren;
    sharedChildren.reserve(std::min(aPaths.size(), bPaths.size()));
    std::vector<SdfPath> aOnlyPaths;
    std::vector<SdfPath> bOnlyPaths;
    set_intersection_and_diff(aPaths.begin(), aPaths.end(), bPaths.begin(), bPaths.end(), std::back_inserter(sharedChildren),
                              std::back_inserter(aOnlyPaths), std::back_inserter(bOnlyPaths));

    // XXX It might be nice to support renaming at this level.  If the prim
    // (path123, dataSource123) is removed, and (path456, dataSource123) is
    // added, we could express that as a rename(path123, path456).

    // For elements only in A, we remove.
    for (const SdfPath& aPath : aOnlyPaths)
    {
        removedEntries->emplace(aPath);
    }

    // For elements that are common, we recurse.
    for (const SdfPath& commonChildPath : sharedChildren)
    {
        dispatcher->Run([=]() { compute_delta_diff_helper(dispatcher, siA, siB, commonChildPath, removedEntries, addedEntries, dirtiedEntries); });
    }

    // For elements only in B, we recursively add.
    for (const SdfPath& bPath : bOnlyPaths)
    {
        fill_added_child_entries_in_parallel(dispatcher, siB, bPath, addedEntries);
    }
}

void HdsiComputeSceneIndexDiffDelta(const HdSceneIndexBaseRefPtr& siA, const HdSceneIndexBaseRefPtr& siB,
                                    HdSceneIndexObserver::RemovedPrimEntries* removedEntries, HdSceneIndexObserver::AddedPrimEntries* addedEntries,
                                    HdSceneIndexObserver::RenamedPrimEntries* renamedEntries,
                                    HdSceneIndexObserver::DirtiedPrimEntries* dirtiedEntries)
{
    if (!(siA && siB))
    {
        // If either is null, fall back to very coarse notifications.
        PXR_NS::HdsiComputeSceneIndexDiffRoot(siA, siB, removedEntries, addedEntries, renamedEntries, dirtiedEntries);
        return;
    }

    // We have both input scenes so we can do a diff.
    _RemovedPrimEntryQueue removedEntriesQueue;
    _AddedPrimEntryQueue addedEntriesQueue;
    _DirtiedPrimEntryQueue dirtiedEntriesQueue;
    {
        WorkDispatcher dispatcher;
        compute_delta_diff_helper(&dispatcher, siA, siB, SdfPath::AbsoluteRootPath(), &removedEntriesQueue, &addedEntriesQueue, &dirtiedEntriesQueue);
        dispatcher.Wait();
    }

    removedEntries->insert(removedEntries->end(), removedEntriesQueue.unsafe_begin(), removedEntriesQueue.unsafe_end());
    addedEntries->insert(addedEntries->end(), addedEntriesQueue.unsafe_begin(), addedEntriesQueue.unsafe_end());
    dirtiedEntries->insert(dirtiedEntries->end(), dirtiedEntriesQueue.unsafe_begin(), dirtiedEntriesQueue.unsafe_end());
}

PXR_NS::TfRefPtr<HydraOpTerminalSceneIndex> HydraOpTerminalSceneIndex::New(const PXR_NS::HdSceneIndexBaseRefPtr& index, ComputeDiffFn computeDiffFn)
{
    return PXR_NS::TfCreateRefPtr(new HydraOpTerminalSceneIndex(index, std::move(computeDiffFn)));
}

HydraOpTerminalSceneIndex::HydraOpTerminalSceneIndex(const HdSceneIndexBaseRefPtr& index, ComputeDiffFn computeDiffFn)
    : m_observer(this)
    , m_compute_diff(std::move(computeDiffFn))
{
    update_scene_index(index);
}

void HydraOpTerminalSceneIndex::reset_index(HdSceneIndexBaseRefPtr index)
{
    if (m_current_scene_index == index)
        return;

    update_scene_index(index);
}

std::vector<HdSceneIndexBaseRefPtr> HydraOpTerminalSceneIndex::GetInputScenes() const
{
    if (m_current_scene_index)
        return { m_current_scene_index };
    return {};
}

HdSceneIndexPrim HydraOpTerminalSceneIndex::GetPrim(const SdfPath& primPath) const
{
    if (m_current_scene_index)
    {
        return m_current_scene_index->GetPrim(primPath);
    }
    return {};
}

SdfPathVector HydraOpTerminalSceneIndex::GetChildPrimPaths(const SdfPath& primPath) const
{
    if (m_current_scene_index)
    {
        return m_current_scene_index->GetChildPrimPaths(primPath);
    }
    return {};
}

void HydraOpTerminalSceneIndex::update_scene_index(HdSceneIndexBaseRefPtr index)
{
    const HdSceneIndexBaseRefPtr prev_input_scene = m_current_scene_index;

    m_current_scene_index = index;

    if (prev_input_scene)
    {
        prev_input_scene->RemoveObserver(HdSceneIndexObserverPtr(&m_observer));
    }

    if (m_compute_diff && _IsObserved())
    {
        HdSceneIndexObserver::RemovedPrimEntries removedEntries;
        HdSceneIndexObserver::AddedPrimEntries addedEntries;
        HdSceneIndexObserver::RenamedPrimEntries renamedEntries;
        HdSceneIndexObserver::DirtiedPrimEntries dirtiedEntries;
        m_compute_diff(prev_input_scene, index, &removedEntries, &addedEntries, &renamedEntries, &dirtiedEntries);
        _SendPrimsRemoved(removedEntries);
        _SendPrimsAdded(addedEntries);
        _SendPrimsRenamed(renamedEntries);
        _SendPrimsDirtied(dirtiedEntries);
    }

    if (index)
    {
        index->AddObserver(HdSceneIndexObserverPtr(&m_observer));
    }
}

void HydraOpTerminalSceneIndex::_PrimsAdded(const HdSceneIndexBase& sender, const HdSceneIndexObserver::AddedPrimEntries& entries)
{
    if (!_IsObserved())
    {
        return;
    }
    _SendPrimsAdded(entries);
}

void HydraOpTerminalSceneIndex::_PrimsRemoved(const HdSceneIndexBase& sender, const HdSceneIndexObserver::RemovedPrimEntries& entries)
{
    if (!_IsObserved())
    {
        return;
    }
    _SendPrimsRemoved(entries);
}

void HydraOpTerminalSceneIndex::_PrimsDirtied(const HdSceneIndexBase& sender, const HdSceneIndexObserver::DirtiedPrimEntries& entries)
{
    if (!_IsObserved())
    {
        return;
    }
    _SendPrimsDirtied(entries);
}

void HydraOpTerminalSceneIndex::_PrimsRenamed(const HdSceneIndexBase& sender, const HdSceneIndexObserver::RenamedPrimEntries& entries)
{
    if (!_IsObserved())
    {
        return;
    }
    _SendPrimsRenamed(entries);
}

void HydraOpTerminalSceneIndex::Observer::PrimsAdded(const HdSceneIndexBase& sender, const AddedPrimEntries& entries)
{
    m_owner->_PrimsAdded(sender, entries);
}

void HydraOpTerminalSceneIndex::Observer::PrimsRemoved(const HdSceneIndexBase& sender, const RemovedPrimEntries& entries)
{
    m_owner->_PrimsRemoved(sender, entries);
}

void HydraOpTerminalSceneIndex::Observer::PrimsDirtied(const HdSceneIndexBase& sender, const DirtiedPrimEntries& entries)
{
    m_owner->_PrimsDirtied(sender, entries);
}

void HydraOpTerminalSceneIndex::Observer::PrimsRenamed(const HdSceneIndexBase& sender, const RenamedPrimEntries& entries)
{
    m_owner->_PrimsRenamed(sender, entries);
}

OPENDCC_NAMESPACE_CLOSE
