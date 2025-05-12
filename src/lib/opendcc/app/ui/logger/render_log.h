/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/base/qt_python.h"
#include "opendcc/app/ui/logger/render_catalog.h"
#include "opendcc/ui/common_widgets/group_widget.h"

#include <QComboBox>
#include <QDockWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QPlainTextEdit>

#include <memory>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API RenderLog : public QWidget
{
    Q_OBJECT;

public:
    RenderLog(QWidget* parent = 0);
    ~RenderLog();

Q_SIGNALS:
    void clear_log();
    void add_msg(QString msg);
    void new_catalog(QString msg);
    void set_log(QString log);

private:
    void setup_new_catalog();
    void setup_active_catalog();
    void setup_add_msg();
    void add_catalog(QString catalog_name);

    QVBoxLayout* m_data_lay = nullptr;
    QWidget* m_main_widget = nullptr;
    QPlainTextEdit* m_output = nullptr;
    QListWidget* m_catalog_list = nullptr;

    std::string m_current_catalog;
    RenderCatalog::NewCatalogHandle m_new_catalog_handle;
    RenderCatalog::ActivateHandle m_active_catalog_handle;
    RenderCatalog::AddMsgHandle m_add_msg_handle;
    void setup_catalog_changed();
};

OPENDCC_NAMESPACE_CLOSE
