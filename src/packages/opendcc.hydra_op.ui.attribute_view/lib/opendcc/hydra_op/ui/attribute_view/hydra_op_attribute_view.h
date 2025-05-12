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

class HduiDataSourceTreeWidget;
class HduiDataSourceValueTreeView;

class HduiSceneIndexDebuggerWidget
    : public QWidget
    , public TfWeakBase
{
    Q_OBJECT;

public:
    struct Options
    {
        Options() {};
        bool showInputsButton = true;
    };

    // customSceneIndexGraphWidget: clients can pass their own custom
    // widget. It will be added as first column in the debugger and is
    // in charge of selecting the scene index to be inspected.
    // Thus, we suppress the "Inputs" button to select a scene index if
    // such a custom widget is given.
    //
    HduiSceneIndexDebuggerWidget(QWidget *parent = Q_NULLPTR, const Options &options = Options());
    ~HduiSceneIndexDebuggerWidget();

protected:
    QSplitter *_splitter;

private Q_SLOTS:
    void _AddSceneIndexToTreeMenu(QTreeWidgetItem *parentItem, HdSceneIndexBaseRefPtr sceneIndex, bool includeSelf);

private:
    HduiDataSourceTreeWidget *_dsTreeWidget;
    HduiDataSourceValueTreeView *_valueTreeView;

    OpenDCC::HydraOpSession::Handle m_selection_event_hndl;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
