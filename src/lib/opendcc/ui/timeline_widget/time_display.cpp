// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "time_display.h"

#include <cmath>

OPENDCC_NAMESPACE_OPEN

QString to_timecode(double frame, double fps)
{
    bool negative = frame < 0 ? true : false;
    unsigned long duration = (std::abs(frame) / fps) * 1000.0;

    long milliseconds = (duration % 1000) / 10; // rounded
    unsigned long seconds = (duration / 1000) % 60;
    unsigned long minutes = (duration / (1000 * 60)) % 60;
    unsigned long hours = (duration / (1000 * 60 * 60)) % 24;

    QString result;
    QString sign = negative ? "-" : "";
    QString milliseconds_str = QString("%1").arg(milliseconds, 2, 10, QChar('0'));
    QString seconds_str = QString("%1").arg(seconds, 2, 10, QChar('0'));
    QString minutes_str = QString("%1").arg(minutes, 2, 10, QChar('0'));
    QString hours_str = QString("%1").arg(hours, 2, 10, QChar('0'));
    result.reserve(sign.length() + milliseconds_str.length() + minutes_str.length() + hours_str.length() +
                   3); // should be fast according to the internet
    result.append(sign);
    result.append(hours_str);
    result.append(":");
    result.append(minutes_str);
    result.append(":");
    result.append(seconds_str);
    result.append(":");
    result.append(milliseconds_str);

    return result;
}

OPENDCC_NAMESPACE_CLOSE
