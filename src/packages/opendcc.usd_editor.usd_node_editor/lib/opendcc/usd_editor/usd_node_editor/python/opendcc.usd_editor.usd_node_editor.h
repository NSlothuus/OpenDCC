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

#include <QByteArray>
#include <QObject>
#include <QSize>
#include <QPoint>
#include <QRect>
#include <QColor>
#include <QLayout>
#include <QGraphicsLayoutItem>
#include <QPainterPath>
#include <QPixmap>
#include <QBrush>
#include <QPen>
#include <QWidget>

#include "opendcc/usd_editor/usd_node_editor/graph_model.h"
#include "opendcc/usd_editor/usd_node_editor/item_registry.h"
#include "opendcc/usd_editor/usd_node_editor/navigation_bar.h"
#include "opendcc/usd_editor/usd_node_editor/node.h"
#include "opendcc/usd_editor/usd_node_editor/oiio_thumbnail_cache.h"
#include "opendcc/usd_editor/usd_node_editor/backdrop_node.h"

using namespace OPENDCC_NAMESPACE;
