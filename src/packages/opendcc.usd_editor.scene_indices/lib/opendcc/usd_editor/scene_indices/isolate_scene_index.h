/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include "opendcc/usd_editor/scene_indices/api.h"
#include <pxr/imaging/hd/filteringSceneIndex.h>
#include <optional>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_USD_EDITOR_SCENE_INDICES_API IsolateSceneIndex : public PXR_NS::HdSingleInputFilteringSceneIndexBase
{
public:
    using IsolatePredicate = std::function<bool(const PXR_NS::SdfPath &)>;

    IsolateSceneIndex(const PXR_NS::HdSceneIndexBaseRefPtr &input_scene_index);
    IsolateSceneIndex(const IsolateSceneIndex &) = delete;
    IsolateSceneIndex(IsolateSceneIndex &&) = delete;

    IsolateSceneIndex &operator=(const IsolateSceneIndex &) = delete;
    IsolateSceneIndex &operator=(IsolateSceneIndex &&) = delete;

    PXR_NS::HdSceneIndexPrim GetPrim(const PXR_NS::SdfPath &prim_path) const override;
    PXR_NS::SdfPathVector GetChildPrimPaths(const PXR_NS::SdfPath &prim_path) const override;

    void set_predicate(const IsolatePredicate &predicate);
    void set_isolate_from(const PXR_NS::SdfPath &isolate_from);
    void set_args(const PXR_NS::SdfPath &isolate_from, const IsolatePredicate &predicate);

protected:
    void _PrimsAdded(const PXR_NS::HdSceneIndexBase &sender, const PXR_NS::HdSceneIndexObserver::AddedPrimEntries &entries) override;
    void _PrimsRemoved(const PXR_NS::HdSceneIndexBase &sender, const PXR_NS::HdSceneIndexObserver::RemovedPrimEntries &entries) override;
    void _PrimsDirtied(const PXR_NS::HdSceneIndexBase &sender, const PXR_NS::HdSceneIndexObserver::DirtiedPrimEntries &entries) override;

private:
    bool is_prunned(const PXR_NS::SdfPath &prim_path) const;
    template <class T>
    T prune_entries(const T &source_entries) const;

    void update_prunned_prefixes(const std::optional<PXR_NS::SdfPath> &isolate_from, const std::optional<IsolatePredicate> &predicate);

    IsolatePredicate m_predicate;
    std::vector<PXR_NS::SdfPath> m_prunned_prefixes;
    PXR_NS::SdfPath m_isolate_from;
};

OPENDCC_NAMESPACE_CLOSE
