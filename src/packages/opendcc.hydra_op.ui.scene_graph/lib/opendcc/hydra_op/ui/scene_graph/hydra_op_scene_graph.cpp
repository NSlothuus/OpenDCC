//
// Copyright 2022 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modifications copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "hydra_op_scene_graph.h"
#include "hydra_op_scene_graph_tree_widget.h"
#include "opendcc/hydra_op/translator/terminal_scene_index.h"

#include "pxr/imaging/hd/filteringSceneIndex.h"
#if PXR_VERSION >= 2408
#include "pxr/imaging/hd/utils.h"
#endif
#include "pxr/base/arch/fileSystem.h"
#include "pxr/base/tf/stringUtils.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <fstream>
#include <sstream>
#include <typeinfo>

PXR_NAMESPACE_OPEN_SCOPE

#if PXR_VERSION < 2408
// XXX stevel: low-tech temporary symbol name demangling until we manage these
// via a formal plug-in/type registry
static std::string Hdui_StripNumericPrefix(const std::string &s)
{
    return TfStringTrimLeft(s, "0123456789");
}
#endif

HydraOpSceneGraph::HydraOpSceneGraph(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    _siTreeWidget = new HydraOpTree;
    mainLayout->addWidget(_siTreeWidget);

    auto sceneIndex = OpenDCC::HydraOpSession::instance().get_view_scene_index();
    if (sceneIndex)
    {
        this->SetRegisteredSceneIndex(sceneIndex);
    }

    std::function<void()> view_node_changed = [this] {
        auto sceneIndex = OpenDCC::HydraOpSession::instance().get_view_scene_index();
        if (sceneIndex)
        {
            this->SetRegisteredSceneIndex(sceneIndex);
        }
    };

    m_view_node_changed_cid =
        OpenDCC::HydraOpSession::instance().register_event_handler(OpenDCC::HydraOpSession::EventType::ViewNodeChanged, view_node_changed);
    connect(_siTreeWidget, &HydraOpTree::PrimSelected, this, &HydraOpSceneGraph::selection_changed);
}

void HydraOpSceneGraph::SetRegisteredSceneIndex(const HdSceneIndexBaseRefPtr &sceneIndex)
{
    SetSceneIndex(std::move(sceneIndex), true);
}

void HydraOpSceneGraph::SetSceneIndex(const HdSceneIndexBaseRefPtr &sceneIndex, bool pullRoot)
{
    _currentSceneIndex = sceneIndex;

    bool inputsPresent = false;
    if (HdFilteringSceneIndexBaseRefPtr filteringSi = TfDynamic_cast<HdFilteringSceneIndexBaseRefPtr>(sceneIndex))
    {
        if (!filteringSi->GetInputScenes().empty())
        {
            inputsPresent = true;
        }
    }

    _siTreeWidget->SetSceneIndex(sceneIndex);

    if (pullRoot)
    {
        _siTreeWidget->Requery();
    }
}

namespace
{
    class _InputSelectionItem : public QTreeWidgetItem
    {
    public:
        _InputSelectionItem(QTreeWidgetItem *parent)
            : QTreeWidgetItem(parent)
        {
        }

        HdSceneIndexBasePtr sceneIndex;
    };
}

void HydraOpSceneGraph::selection_changed(const SdfPath &path)
{
    OpenDCC::HydraOpSession::instance().set_selection(OpenDCC::SelectionList(SdfPathVector { path }));
}

HydraOpSceneGraph::~HydraOpSceneGraph()
{
    OpenDCC::HydraOpSession::instance().unregister_event_handler(OpenDCC::HydraOpSession::EventType::ViewNodeChanged, m_view_node_changed_cid);
}

PXR_NAMESPACE_CLOSE_SCOPE
