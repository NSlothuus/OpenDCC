/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QKeySequence>
#include <QHBoxLayout>

class KeySequenceEdit : public QWidget
{
    Q_OBJECT

public:
    KeySequenceEdit(QWidget *parent = nullptr, int sequence_length = 0);

    QKeySequence key_sequence() const;
    bool eventFilter(QObject *object, QEvent *event) override;

public Q_SLOTS:
    void set_key_sequence(const QKeySequence &sequence);
    void set_sequence_length(int length);
    void set_is_error(bool is_error);

Q_SIGNALS:
    void key_sequence_changed(const QKeySequence &sequence);

protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

private Q_SLOTS:
    void clear_shortcut();

private:
    void handle_key_event(QKeyEvent *event);
    int translate_modifiers(Qt::KeyboardModifiers modifiers, const QString &text) const;

    int m_current_index = 0;
    int m_sequence_length = 0;
    QKeySequence m_current_sequence;
    QLineEdit *m_line_edit = nullptr;
};