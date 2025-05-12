/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>

class QDoubleValidator;

OPENDCC_NAMESPACE_OPEN

/**
 * @brief This class represents a line edit widget for entering frames per second (FPS) values.
 */
class FramesPerSecondLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a FramesPerSecondLineEdit.
     *
     * @param parent The parent QWidget.
     */
    FramesPerSecondLineEdit(QWidget* parent = nullptr);
    ~FramesPerSecondLineEdit();

protected:
    virtual void paintEvent(QPaintEvent* event) override;

private Q_SLOTS:
    void handle_textChanged(const QString& text);

private:
    QPixmap m_warning { ":/icons/warning" };
};

/**
 * @brief This class represents a widget for selecting frames per second (FPS) values.
 *        It derives from QComboBox for the selection functionality.
 */
class FramesPerSecondWidget : public QComboBox
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a FramesPerSecondWidget.
     *
     * @param parent The parent QWidget.
     */
    FramesPerSecondWidget(QWidget* parent = nullptr);
    ~FramesPerSecondWidget();

    /**
     * @brief Sets the value of the FramesPerSecondWidget.
     *
     * @param value The frames per second value to set.
     */
    void setValue(double value);
    /**
     * @brief Retrieves the current value of the FramesPerSecondWidget.
     *
     * @return The current frames per second value.
     */
    double getValue() const;

Q_SIGNALS:
    /**
     * @brief Signal emitted when the value of the FramesPerSecondWidget changes.
     *
     * @param value The updated frames per second value.
     */
    void valueChanged(double);

private Q_SLOTS:
    void handle_editingFinished();

private:
    QDoubleValidator* m_validator;
    FramesPerSecondLineEdit* m_line_edit;
    double m_value = 24.0;
};

OPENDCC_NAMESPACE_CLOSE
