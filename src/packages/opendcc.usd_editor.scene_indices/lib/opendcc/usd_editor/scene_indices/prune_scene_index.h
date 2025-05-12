/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include "opendcc/usd_editor/scene_indices/api.h"
#include <pxr/imaging/hd/filteringSceneIndex.h>

OPENDCC_NAMESPACE_OPEN
// Inspired by HdsiPrimTypeAndPathPruningSceneIndex
// https://github.com/PixarAnimationStudios/OpenUSD/blob/release/pxr/imaging/hdsi/primTypeAndPathPruningSceneIndex.h
class OPENDCC_USD_EDITOR_SCENE_INDICES_API PruneSceneIndex : public PXR_NS::HdSingleInputFilteringSceneIndexBase
{
public:
    using PrunePredicate = std::function<bool(const PXR_NS::SdfPath &)>;

    PruneSceneIndex(const PXR_NS::HdSceneIndexBaseRefPtr &input_scene_index);
    PruneSceneIndex(const PruneSceneIndex &) = delete;
    PruneSceneIndex(PruneSceneIndex &&) = delete;

    PruneSceneIndex &operator=(const PruneSceneIndex &) = delete;
    PruneSceneIndex &operator=(PruneSceneIndex &&) = delete;

    PXR_NS::HdSceneIndexPrim GetPrim(const PXR_NS::SdfPath &prim_path) const override;
    PXR_NS::SdfPathVector GetChildPrimPaths(const PXR_NS::SdfPath &prim_path) const override;

    void set_predicate(const PrunePredicate &predicate);

protected:
    void _PrimsAdded(const PXR_NS::HdSceneIndexBase &sender, const PXR_NS::HdSceneIndexObserver::AddedPrimEntries &entries) override;
    void _PrimsRemoved(const PXR_NS::HdSceneIndexBase &sender, const PXR_NS::HdSceneIndexObserver::RemovedPrimEntries &entries) override;
    void _PrimsDirtied(const PXR_NS::HdSceneIndexBase &sender, const PXR_NS::HdSceneIndexObserver::DirtiedPrimEntries &entries) override;

private:
    bool is_prunned(const PXR_NS::SdfPath &prim_path) const;
    template <class T>
    T prune_entries(const T &source_entries) const;

    PrunePredicate m_predicate;
    std::vector<PXR_NS::SdfPath> m_prunned_prefixes;
};

OPENDCC_NAMESPACE_CLOSE
