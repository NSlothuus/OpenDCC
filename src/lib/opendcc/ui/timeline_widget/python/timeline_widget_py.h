/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <opendcc/base/qt_python.h>

// Make "signals:", "slots:" visible as access specifiers
#ifndef QT_NOT_GEN
#define QT_ANNOTATE_ACCESS_SPECIFIER(a) __attribute__((annotate(#a)))
#endif

#include <QSize>
#include <QPoint>
#include <QRect>
#include <QColor>
// #include <boost/python.hpp>
#include <QLayout>

#include "opendcc/ui/timeline_widget/time_display.h"
#include "opendcc/ui/timeline_widget/timebar_widget.h"
#include "opendcc/ui/timeline_widget/timeline_widget.h"

using namespace OPENDCC_NAMESPACE;
