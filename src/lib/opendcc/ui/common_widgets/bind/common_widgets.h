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
#include <QLayout>

#include "opendcc/ui/common_widgets/color_widget.h"
#include "opendcc/ui/common_widgets/marking_menu.h"
#include "opendcc/ui/common_widgets/round_marking_menu.h"
#include "opendcc/ui/common_widgets/ramp_widget.h"
#include "opendcc/ui/common_widgets/gradient_widget.h"
#include "opendcc/ui/common_widgets/float_slider.h"
#include "opendcc/ui/common_widgets/number_value_widget.h"
#include "opendcc/ui/common_widgets/group_widget.h"
#include "opendcc/ui/common_widgets/rollout_widget.h"
#include "opendcc/ui/common_widgets/popup_window.h"
#include "opendcc/ui/common_widgets/double_widget.h"
#include "opendcc/ui/common_widgets/help_button_widget.h"
#include "opendcc/ui/common_widgets/search_widget.h"
