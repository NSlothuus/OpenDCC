/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "pxr/imaging/hd/filteringSceneIndex.h"
#include "pxr/imaging/hdsi/api.h"
#include "pxr/imaging/hdsi/computeSceneIndexDiff.h"

#include "opendcc/opendcc.h"
#include "opendcc/hydra_op/translator/api.h"

OPENDCC_NAMESPACE_OPEN

class OPENDCC_HYDRA_OP_TRANSLATOR_API HydraOpTerminalSceneIndex : public PXR_NS::HdFilteringSceneIndexBase
{
public:
    using ComputeDiffFn = PXR_NS::HdsiComputeSceneIndexDiff;

    static PXR_NS::TfRefPtr<HydraOpTerminalSceneIndex> New(const PXR_NS::HdSceneIndexBaseRefPtr& index,
                                                           ComputeDiffFn computeDiffFn = PXR_NS::HdsiComputeSceneIndexDiffDelta);

protected:
    HydraOpTerminalSceneIndex(const PXR_NS::HdSceneIndexBaseRefPtr& index, ComputeDiffFn computeDiffFn);

public:
    void reset_index(PXR_NS::HdSceneIndexBaseRefPtr index);

public:
    std::vector<PXR_NS::HdSceneIndexBaseRefPtr> GetInputScenes() const override;

public:
    PXR_NS::HdSceneIndexPrim GetPrim(const PXR_NS::SdfPath& primPath) const override;

    PXR_NS::SdfPathVector GetChildPrimPaths(const PXR_NS::SdfPath& primPath) const override;

private:
    void update_scene_index(PXR_NS::HdSceneIndexBaseRefPtr index);

    void _PrimsAdded(const HdSceneIndexBase& sender, const PXR_NS::HdSceneIndexObserver::AddedPrimEntries& entries);

    void _PrimsRemoved(const HdSceneIndexBase& sender, const PXR_NS::HdSceneIndexObserver::RemovedPrimEntries& entries);

    void _PrimsDirtied(const HdSceneIndexBase& sender, const PXR_NS::HdSceneIndexObserver::DirtiedPrimEntries& entries);

    void _PrimsRenamed(const HdSceneIndexBase& sender, const PXR_NS::HdSceneIndexObserver::RenamedPrimEntries& entries);

    friend class Observer;
    class Observer final : public PXR_NS::HdSceneIndexObserver
    {
    public:
        Observer(HydraOpTerminalSceneIndex* owner)
            : m_owner(owner)
        {
        }

    public: // satisfying HdSceneIndexObserver
        void PrimsAdded(const HdSceneIndexBase& sender, const AddedPrimEntries& entries) override;
        void PrimsRemoved(const HdSceneIndexBase& sender, const RemovedPrimEntries& entries) override;
        void PrimsDirtied(const HdSceneIndexBase& sender, const DirtiedPrimEntries& entries) override;
        void PrimsRenamed(const HdSceneIndexBase& sender, const RenamedPrimEntries& entries) override;

    private:
        HydraOpTerminalSceneIndex* m_owner;
    };

    Observer m_observer;

    PXR_NS::HdSceneIndexBaseRefPtr m_current_scene_index;

    ComputeDiffFn m_compute_diff;
};

OPENDCC_NAMESPACE_CLOSE
