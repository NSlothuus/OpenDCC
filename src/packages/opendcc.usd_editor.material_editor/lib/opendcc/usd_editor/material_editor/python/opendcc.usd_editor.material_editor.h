/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#define NOMINMAX
#include <opendcc/base/qt_python.h>

// to bypass tbb check
#define _MT 1

// to bypass boost check
#define _DLL 1
// Make "signals:", "slots:" visible as access specifiers
#define QT_ANNOTATE_ACCESS_SPECIFIER(a) __attribute__((annotate(#a)))

#include <QSize>
#include <QPoint>
#include <QRect>
#include <QColor>
#include <QLayout>
#include <QPainterPath>
#include <QPixmap>
#include <QGraphicsSceneMouseEvent>

#include "opendcc/usd_editor/material_editor/utils.h"
#include "opendcc/usd_editor/material_editor/material_item_registry.h"
#include "opendcc/usd_editor/material_editor/shader_node.h"
#include "opendcc/usd_editor/material_editor/material_output_item.h"
#include "opendcc/usd_editor/material_editor/model.h"
#include "opendcc/usd_editor/material_editor/nodegraph_item.h"

using namespace OPENDCC_NAMESPACE;
