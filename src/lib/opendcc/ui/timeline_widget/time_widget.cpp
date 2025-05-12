// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "time_widget.h"

#include <QRegularExpression>

OPENDCC_NAMESPACE_OPEN

TimeWidget::TimeWidget(TimeDisplay mode, QWidget* parent)
    : QDoubleSpinBox(parent)
{
    m_time_display = mode;
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    setFixedWidth(70);
    setMaximum(1e10);
    setMinimum(-1e10);
    setLocale(QLocale(QLocale::Hawaiian, QLocale::UnitedStates));
}

TimeWidget::~TimeWidget() {}

void TimeWidget::set_time_display(TimeDisplay mode)
{
    m_time_display = mode;

    // force update
    blockSignals(true);
    setValue(value());
    blockSignals(false);
}

void TimeWidget::set_fps(double fps)
{
    m_fps = fps;

    if (m_time_display == TimeDisplay::Timecode)
    {
        // force update
        blockSignals(true);
        setValue(value());
        blockSignals(false);
    }
}

QString TimeWidget::textFromValue(double value) const
{
    switch (m_time_display)
    {
    case OPENDCC_NAMESPACE::TimeDisplay::Frames:
        return QDoubleSpinBox::textFromValue(value);
        break;
    case OPENDCC_NAMESPACE::TimeDisplay::Timecode:
        return to_timecode(value, m_fps);
        break;
    default:
        return QDoubleSpinBox::textFromValue(value);
    }
}

QValidator::State TimeWidget::validate(QString& text, int& pos) const
{
    switch (m_time_display)
    {
    case OPENDCC_NAMESPACE::TimeDisplay::Frames:
        return QDoubleSpinBox::validate(text, pos);
        break;
    case OPENDCC_NAMESPACE::TimeDisplay::Timecode:
    {
        auto match = m_timecode_exp.match(text);
        if (match.hasMatch())
        {
            QString minutes_str = match.captured(3);
            QString seconds_str = match.captured(4);

            unsigned long minutes = minutes_str.toULong();
            unsigned long seconds = seconds_str.toULong();

            if (minutes > 59 || seconds > 59)
            {
                return QValidator::State::Intermediate;
            }

            return QValidator::State::Acceptable;
        }
        return QValidator::State::Intermediate;
    }
    break;
    default:
        return QDoubleSpinBox::validate(text, pos);
        break;
    }
}

double TimeWidget::valueFromText(const QString& text) const
{
    switch (m_time_display)
    {
    case OPENDCC_NAMESPACE::TimeDisplay::Frames:
        return QDoubleSpinBox::valueFromText(text);
        break;
    case OPENDCC_NAMESPACE::TimeDisplay::Timecode:
    {
        auto match = m_timecode_exp.match(text);
        if (match.hasMatch())
        {
            QString sign_str = match.captured(1);
            QString hours_str = match.captured(2);
            QString minutes_str = match.captured(3);
            QString seconds_str = match.captured(4);
            QString milliseconds_str = match.captured(5);

            bool sign = sign_str.length() ? false : true;
            unsigned long hours = hours_str.toULong();
            unsigned long minutes = minutes_str.toULong();
            unsigned long seconds = seconds_str.toULong();
            unsigned long milliseconds = milliseconds_str.toULong();

            unsigned long hours_to_mili = hours * 60 * 60 * 1000;
            unsigned long minutes_to_mili = minutes * 60 * 1000;
            unsigned long seconds_to_mili = seconds * 1000;

            unsigned long total = hours_to_mili + minutes_to_mili + seconds_to_mili + milliseconds;

            double result = double(total) / 1000.0 * m_fps;

            return sign ? result : -result;
        }
        return 0.0;
    }
    break;
    default:
        return QDoubleSpinBox::valueFromText(text);
        break;
    }
}

OPENDCC_NAMESPACE_CLOSE
