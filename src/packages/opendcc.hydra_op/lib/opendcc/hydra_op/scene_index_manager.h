/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/app/viewport/viewport_scene_context.h"
#include "opendcc/hydra_op/translator/terminal_scene_index.h"
#include <pxr/usdImaging/usdImaging/selectionSceneIndex.h>

OPENDCC_NAMESPACE_OPEN

class HydraOpSceneIndexManager final : public SceneIndexManager
{
public:
    HydraOpSceneIndexManager();
    ~HydraOpSceneIndexManager();

    PXR_NS::HdSceneIndexBaseRefPtr get_terminal_index() override;

    void set_selection(const SelectionList& selection_list) override;

private:
    class ViewportUpdateObserver final : public PXR_NS::HdSceneIndexObserver
    {
    public: // satisfying HdSceneIndexObserver
        void PrimsAdded(const PXR_NS::HdSceneIndexBase& sender, const AddedPrimEntries& entries) override;
        void PrimsRemoved(const PXR_NS::HdSceneIndexBase& sender, const RemovedPrimEntries& entries) override;
        void PrimsDirtied(const PXR_NS::HdSceneIndexBase& sender, const DirtiedPrimEntries& entries) override;
        void PrimsRenamed(const PXR_NS::HdSceneIndexBase& sender, const RenamedPrimEntries& entries) override;

    private:
        void update_viewport();
    };

    ViewportUpdateObserver m_observer;
    PXR_NS::TfWeakPtr<HydraOpTerminalSceneIndex> m_viewable_si;
    PXR_NS::UsdImagingSelectionSceneIndexRefPtr m_selection_si;
    PXR_NS::HdSceneIndexBaseRefPtr m_terminal_si;
};

OPENDCC_NAMESPACE_CLOSE
