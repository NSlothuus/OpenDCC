/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"
#include <functional>

#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

OPENDCC_NAMESPACE_OPEN

/**
 * @brief The GroupWidget class is a custom widget for displaying a group with a name,
 * description, and icon.
 */
class OPENDCC_UI_COMMON_WIDGETS_API GroupWidget : public QWidget
{
    Q_OBJECT;
    void setup(const QString& name, QWidget* desc, QWidget* icon);
    void switch_group();

public:
    /**
     * @brief Constructs a GroupWidget object with the specified name, description,
     * parent widget, and window flags.
     *
     * @param name The name of the group.
     * @param desc The description of the group.
     * @param parent The parent widget.
     * @param flags The window flags.
     */
    GroupWidget(const QString& name, const QString& desc, QWidget* parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    /**
     * @brief Constructs a GroupWidget object with the specified icon, name, description,
     * parent widget, and window flags.
     *
     * @param icon The icon widget of the group.
     * @param name The name of the group.
     * @param desc The description of the group.
     * @param parent The parent widget.
     * @param flags The window flags.
     */
    GroupWidget(QWidget* icon, const QString& name, const QString& desc, QWidget* parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    /**
     * @brief Constructs a GroupWidget object with the specified name, description,
     * icon, parent widget, and window flags.
     *
     * @param name The name of the group.
     * @param desc The description of the group.
     * @param icon The icon widget of the group.
     * @param parent The parent widget.
     * @param flags The window flags.
     */
    GroupWidget(const QString& name, QWidget* desc, QWidget* icon = 0, QWidget* parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    ~GroupWidget() {}

    /**
     * @brief Sets the close state of the group.
     *
     * @param is_closed The close state to set.
     */
    void set_close(bool is_closed);
    /**
     * @brief Sets the name of the group.
     *
     * @param val The name to set.
     */
    void name(const QString& val);
    /**
     * @brief Sets the description of the group.
     *
     * @param val The description to set.
     */
    void desc(const QString& val);
    /**
     * @brief Clears the group widget.
     */
    void clear();
    /**
     * @brief Adds a widget to the group.
     *
     * @param widget The widget to add.
     */
    void add_widget(QWidget* widget);

    /**
     * @brief Event filter function for the group widget.
     *
     * @param watched The watched object.
     * @param event The event.
     * @return Whether the event was handled.
     */
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

protected:
    virtual void paintEvent(QPaintEvent* event) override;

Q_SIGNALS:
    /**
     * @brief Signal emitted when the open state of the group changes.
     *
     * @param new_state The new open state of the group.
     */
    void open_state_changed(bool new_state);

private:
    bool m_content_visible = true;
    QWidget* m_data_widget = nullptr;
    QVBoxLayout* m_main_layout = 0;
    QVBoxLayout* m_content_layout = 0;
    QVBoxLayout* m_data_layout = 0;
    QWidget* m_content_widget = 0;
    QLabel* m_name = 0;
    QLabel* m_desc = 0;
    std::vector<QWidget*> m_widgets;
    QPushButton* m_open_btn;
};

OPENDCC_NAMESPACE_CLOSE
