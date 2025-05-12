/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "api.h"
#include "opendcc/opendcc.h"
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/property.h>

OPENDCC_NAMESPACE_OPEN

bool OPENDCC_USD_NODE_EDITOR_API remove_connection(PXR_NS::UsdProperty& prop, const PXR_NS::SdfPath& connection);
bool OPENDCC_USD_NODE_EDITOR_API remove_connections(PXR_NS::UsdProperty& prop, const PXR_NS::SdfPathVector& connections);

OPENDCC_NAMESPACE_CLOSE