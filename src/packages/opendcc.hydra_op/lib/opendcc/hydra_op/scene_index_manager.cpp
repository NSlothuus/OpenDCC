// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/hydra_op/scene_index_manager.h"
#include "opendcc/hydra_op/session.h"
#include <pxr/usdImaging/usdImaging/selectionSceneIndex.h>
#include <opendcc/app/viewport/viewport_widget.h>

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

HydraOpSceneIndexManager::HydraOpSceneIndexManager()
{
    m_viewable_si = HydraOpSession::instance().get_view_scene_index();
    m_viewable_si->AddObserver(HdSceneIndexObserverPtr(&m_observer));
    m_selection_si = UsdImagingSelectionSceneIndex::New(m_viewable_si);
    set_selection(HydraOpSession::instance().get_selection());
    m_terminal_si = m_selection_si;
}

HydraOpSceneIndexManager::~HydraOpSceneIndexManager()
{
    if (!m_viewable_si.IsExpired())
    {
        m_viewable_si->RemoveObserver(HdSceneIndexObserverPtr(&m_observer));
    }
}

PXR_NS::HdSceneIndexBaseRefPtr HydraOpSceneIndexManager::get_terminal_index()
{
    return m_terminal_si;
}

void HydraOpSceneIndexManager::set_selection(const SelectionList& selection_list)
{
    m_selection_si->ClearSelection();
    for (const auto& entry : selection_list.get_fully_selected_paths())
    {
        m_selection_si->AddSelection(entry);
    }
    ViewportWidget::update_all_gl_widget();
}

void HydraOpSceneIndexManager::ViewportUpdateObserver::PrimsAdded(const HdSceneIndexBase& sender, const AddedPrimEntries& entries)
{
    update_viewport();
}

void HydraOpSceneIndexManager::ViewportUpdateObserver::PrimsRemoved(const HdSceneIndexBase& sender, const RemovedPrimEntries& entries)
{
    update_viewport();
}

void HydraOpSceneIndexManager::ViewportUpdateObserver::PrimsDirtied(const HdSceneIndexBase& sender, const DirtiedPrimEntries& entries)
{
    update_viewport();
}

void HydraOpSceneIndexManager::ViewportUpdateObserver::PrimsRenamed(const HdSceneIndexBase& sender, const RenamedPrimEntries& entries)
{
    update_viewport();
}

void HydraOpSceneIndexManager::ViewportUpdateObserver::update_viewport()
{
    ViewportWidget::update_all_gl_widget();
}

OPENDCC_NAMESPACE_CLOSE
