/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/node_editor/api.h"
#include <QMenu>
#include <QWidgetAction>
#include <QLabel>

OPENDCC_NAMESPACE_OPEN

class CompleterWidget;

class OPENDCC_UI_NODE_EDITOR_API TabSearchMenu : public QMenu
{
    Q_OBJECT
public:
    TabSearchMenu(QWidget* parent);
    virtual ~TabSearchMenu() {};

    void clear_actions();
    void update_completer_text();

protected:
    bool event(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    CompleterWidget* m_completer_widget;
    QString m_current_text;

    void update_completer();
    void collect_actions(QList<QAction*>& all_actions, QMenu* menu);
};

class CompletingItemsMenu : public QMenu
{
    Q_OBJECT

public:
    CompletingItemsMenu(TabSearchMenu* parent_menu);
    virtual ~CompletingItemsMenu() {};

private Q_SLOTS:
    void on_triggered(QAction* action);
    void on_hovered(QAction* action);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    TabSearchMenu* m_parent_menu;
};

class CompleterWidget : public QWidgetAction
{
    Q_OBJECT
public:
    CompleterWidget(TabSearchMenu* parent);
    virtual ~CompleterWidget() {};

    void set_input_text(const QString& value);
    void build_menu(const QString& value, const QList<QAction*>& actions, QMenu* parent_menu);

    CompletingItemsMenu* get_menu() { return m_completing_menu; }

private:
    const QString m_default_text = "(Type to search)";

    CompletingItemsMenu* m_completing_menu;

    QLabel* m_search_icon;
    QLabel* m_search_text;
    QWidget* m_search_input;
};
OPENDCC_NAMESPACE_CLOSE
