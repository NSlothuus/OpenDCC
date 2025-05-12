//
// Copyright 2022 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modifications copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#ifndef PXR_IMAGING_HDUI_SCENE_INDEX_OBSERVER_LOGGING_WIDGET_H
#define PXR_IMAGING_HDUI_SCENE_INDEX_OBSERVER_LOGGING_WIDGET_H

#include "pxr/pxr.h"

#include <QPushButton>
#include <QLabel>

PXR_NAMESPACE_OPEN_SCOPE

class HduiSceneIndexObserverLoggingTreeView;

class HduiSceneIndexObserverLoggingWidget : public QWidget
{
    Q_OBJECT;

public:
    HduiSceneIndexObserverLoggingWidget(QWidget *parent = Q_NULLPTR);

    HduiSceneIndexObserverLoggingTreeView *GetTreeView();

    void SetLabel(const std::string &labelText);

private:
    QPushButton *_startStopButton;
    QPushButton *_clearButton;
    HduiSceneIndexObserverLoggingTreeView *_treeView;

    QLabel *_label;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_IMAGING_HDUI_SCENE_INDEX_OBSERVER_LOGGING_WIDGET_H
