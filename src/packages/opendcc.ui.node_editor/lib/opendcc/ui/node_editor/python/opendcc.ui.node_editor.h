/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <opendcc/base/qt_python.h>

// Make "signals:", "slots:" visible as access specifiers
#define QT_ANNOTATE_ACCESS_SPECIFIER(a) __attribute__((annotate(#a)))

#include <QByteArray>
#include <QObject>
#include <QSize>
#include <QPointF>
#include <QRectF>
#include <QObject>
#include <QPainterPath>

#include "opendcc/ui/node_editor/graph_model.h"
#include "opendcc/ui/node_editor/scene.h"
#include "opendcc/ui/node_editor/connection.h"
#include "opendcc/ui/node_editor/node.h"
#include "opendcc/ui/node_editor/item_registry.h"
#include "opendcc/ui/node_editor/view.h"
#include "opendcc/ui/node_editor/thumbnail_cache.h"
#include "opendcc/ui/node_editor/text_item.h"
#include "opendcc/ui/node_editor/tab_search.h"

using namespace OPENDCC_NAMESPACE;
