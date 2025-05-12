/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/packaging/api.h"
#include <unordered_map>
#include <pxr/base/vt/value.h>

OPENDCC_NAMESPACE_OPEN

OPENDCC_PACKAGING_API extern std::unordered_map<std::string, PXR_NS::VtValue> s_core_property_defaults;

OPENDCC_NAMESPACE_CLOSE
