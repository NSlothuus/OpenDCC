//
// Copyright 2022 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modifications copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#ifndef PXR_IMAGING_HDUI_SCENE_INDEX_DEBUGGING_WIDGET_H
#define PXR_IMAGING_HDUI_SCENE_INDEX_DEBUGGING_WIDGET_H

#include "pxr/pxr.h"

#include "pxr/imaging/hd/sceneIndex.h"
#include "opendcc/hydra_op/session.h"

#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QTreeWidgetItem>
#include <QSplitter>

PXR_NAMESPACE_OPEN_SCOPE

class HydraOpTree;

class HydraOpSceneGraph
    : public QWidget
    , public TfWeakBase
{
    Q_OBJECT;

public:
    // customSceneIndexGraphWidget: clients can pass their own custom
    // widget. It will be added as first column in the debugger and is
    // in charge of selecting the scene index to be inspected.
    // Thus, we suppress the "Inputs" button to select a scene index if
    // such a custom widget is given.
    //
    HydraOpSceneGraph(QWidget *parent = Q_NULLPTR);
    ~HydraOpSceneGraph();

    // Called when we select a registered (terminal) scene index.
    virtual void SetRegisteredSceneIndex(const HdSceneIndexBaseRefPtr &sceneIndex);

    // Sets which scene index we are inspecting.
    void SetSceneIndex(const HdSceneIndexBaseRefPtr &sceneIndex, bool pullRoot);

public Q_SLOTS:
    void selection_changed(const SdfPath &path);

private:
    HydraOpTree *_siTreeWidget;
    HdSceneIndexBasePtr _currentSceneIndex;
    OpenDCC::HydraOpSession::Handle m_view_node_changed_cid;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
