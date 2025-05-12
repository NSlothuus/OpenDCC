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

#include "opendcc/hydra_op/ui/node_editor/hydra_op_utils.h"
#include "opendcc/hydra_op/ui/node_editor/hydra_op_node_item.h"
#include "opendcc/hydra_op/ui/node_editor/hydra_op_item_registry.h"
#include "opendcc/hydra_op/ui/node_editor/hydra_op_graph_model.h"

using namespace OPENDCC_NAMESPACE;
