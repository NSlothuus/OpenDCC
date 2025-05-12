/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/usd_editor/usd_node_editor/api.h"
// workaround
#include "opendcc/base/qt_python.h"
#include <QListWidget>
#include <QWidget>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <pxr/usd/sdf/path.h>
#include <stack>

class QMenu;

OPENDCC_NAMESPACE_OPEN

class PathWidget;
class PathTokenWidget;
class PathTokenMenu;

class OPENDCC_USD_NODE_EDITOR_API NavigationBar : public QWidget
{
    Q_OBJECT
public:
    NavigationBar(QWidget* parent = nullptr);
    ~NavigationBar();

    void set_path(PXR_NS::SdfPath path);
    PXR_NS::SdfPath get_path() const;
Q_SIGNALS:
    void path_changed(PXR_NS::SdfPath path);

private:
    PathWidget* m_path_widget = nullptr;
    QMenu* m_recent_menu = nullptr;
    int32_t m_stack_pointer = -1;
    std::vector<QString> m_path_stack;
    PXR_NS::SdfPath m_path;
};

class PathWidget : public QLineEdit
{
    Q_OBJECT
public:
    PathWidget(QWidget* parent = nullptr);
    ~PathWidget() = default;
    void update_path(const QString& path);
Q_SIGNALS:
    void path_changed(const QString& path);

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private Q_SLOTS:
    void on_group_btn_clicked(QAbstractButton*);

private:
    void clear_address_token_widget();

    QHBoxLayout* m_layout = nullptr;
    QAbstractButton* m_last_check_btn = nullptr;
    QButtonGroup* m_address_group = nullptr;
    QString m_current_path = nullptr;
};

class PathTokenWidget : public QPushButton
{
    Q_OBJECT
public:
    PathTokenWidget(const QString& text, const QString& path, bool show_next = true, const QPixmap& icon = QPixmap(), QWidget* parent = nullptr);
    ~PathTokenWidget();

    void set_back_icon(bool flag);

Q_SIGNALS:
    void click_path(const QString& path);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QMenu* m_menu = nullptr;
    QFont m_font;
    QString m_path;
    QString m_text;
    QPixmap m_icon;

    QPixmap m_normal_icon;
    QPixmap m_checked_icon;
    int m_text_width = 0;
    bool m_show_next = true;
};

OPENDCC_NAMESPACE_CLOSE
