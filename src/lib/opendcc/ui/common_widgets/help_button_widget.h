/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"
#include <functional>

#include <QWidget>
#include <QFrame>
#include <QPushButton>

OPENDCC_NAMESPACE_OPEN
/**
 * @brief The HelpDialog class is a custom dialog that displays help information.
 */
class OPENDCC_UI_COMMON_WIDGETS_API HelpDialog : public QFrame
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a HelpDialog object with the specified title, help text,
     * markdown flag, parent widget, and window flags.
     *
     * @param title The title of the help dialog.
     * @param help_text The help text to display.
     * @param markdown Flag indicating whether the help text is in markdown format.
     * @param parent The parent widget.
     */
    HelpDialog(QString title, QString help_text, bool markdown, QWidget* parent = nullptr);
};

/**
 * @brief The HelpButtonWidget class is a custom widget that provides a help
 * button functionality.
 */
class OPENDCC_UI_COMMON_WIDGETS_API HelpButtonWidget : public QPushButton
{
    Q_OBJECT;

protected:
    virtual void mouseReleaseEvent(QMouseEvent* e) override;

public:
    /**
     * @brief Constructs a HelpButtonWidget object with the specified parent widget.
     *
     * @param parent The parent widget.
     */
    HelpButtonWidget(QWidget* parent = nullptr);
    /**
     * @brief Sets the title and documentation for the help button.
     *
     * @param title The title of the help button.
     * @param docs The documentation to display.
     */
    void set_docs(QString title, QString docs);
    /**
     * @brief Enables or disables markdown formatting for the documentation.
     *
     * @param markdown Flag indicating whether markdown formatting should be enabled.
     */
    void enable_markdown(bool markdown) { m_markdown = markdown; };

private:
    bool m_markdown = false;
    QString m_title = "";
    QString m_docs = "";
};

OPENDCC_NAMESPACE_CLOSE
