/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <QWidget>

class QTreeWidget;

OPENDCC_NAMESPACE_OPEN

class RenderViewMainWindow;

class RenderViewMetadataView : public QWidget
{
    Q_OBJECT
public:
    RenderViewMetadataView(RenderViewMainWindow* app);
    ~RenderViewMetadataView();

public Q_SLOTS:
    void update_metadata();

private:
    QTreeWidget* m_tree_widget = nullptr;
    RenderViewMainWindow* m_app = nullptr;
};

OPENDCC_NAMESPACE_CLOSE
