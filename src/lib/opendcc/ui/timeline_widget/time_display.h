/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/ui/timeline_widget/api.h"

#include <QMetaType>
#include <QString>

OPENDCC_NAMESPACE_OPEN

/**
 * @brief Enumeration for selecting the time display mode of a widget.
 *
 */
enum class TimeDisplay
{
    Frames, /**< Display time in frames */
    Timecode /**< Display time in timecode format */
};

/**
 * @brief Enumeration for selecting the current time indicator style of a widget.
 */
enum class CurrentTimeIndicator
{
    Default, /**< Default current time indicator */
    Arrow /**< Arrow-shaped current time indicator*/
};

OPENDCC_UI_TIMELINE_WIDGET_API QString to_timecode(double frame, double fps);

OPENDCC_NAMESPACE_CLOSE

Q_DECLARE_METATYPE(OpenDCC::TimeDisplay);
