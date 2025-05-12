/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/ui/node_editor/api.h"
#include <QVector>

OPENDCC_NAMESPACE_OPEN
class NodeItem;

OPENDCC_UI_NODE_EDITOR_API bool layout_items(const QVector<NodeItem*>& items, bool vertical = false);

OPENDCC_NAMESPACE_CLOSE
