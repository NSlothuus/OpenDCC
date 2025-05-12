/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/ui/logger_panel/logger_widget.h"
#include "opendcc/ui/common_widgets/search_widget.h"
#include "opendcc/ui/logger_panel/message_table_proxy.h"

#include <QHash>
#include <QLineEdit>
#include <QTableView>
#include <QActionGroup>
#include <QStyledItemDelegate>

OPENDCC_NAMESPACE_OPEN

class TableItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT;

public:
    TableItemDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

class TableView : public QTableView
{
    Q_OBJECT;

public:
    TableView(QWidget* parent = nullptr);

    void mousePressEvent(QMouseEvent* event) override;
};

class LoggerMessageListWidget : public LoggerWidget
{
    Q_OBJECT;

public:
    LoggerMessageListWidget(MessageModel* model, QWidget* parent = 0);

    void resizeEvent(QResizeEvent* event) override;
public Q_SLOTS:
    void on_selected_channels_cleared() override;

    void on_selected_channels_changed(const QSet<QString>& selected_channels) override;
    void on_wrap_mode_changed(bool is_wrap) override;

    void on_model_updated(const QVector<Message>& messages) override;
    void on_message_added(const Message& message) override;
private Q_SLOTS:
    void set_search_query(QString query);

private:
    void update_log_level();
    void resize_row(int row);

    MessageTableProxy* m_messages_table_proxy = nullptr;
    uint32_t m_log_level_mask = LogLevelFlags::None;

    SearchWidget* m_search_line = nullptr;
    TableView* m_messages_table = nullptr;
};

OPENDCC_NAMESPACE_CLOSE
