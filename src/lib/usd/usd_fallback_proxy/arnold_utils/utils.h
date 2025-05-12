/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include <pxr/usd/ndr/property.h>
#include <pxr/usd/sdf/layer.h>

#include <ai.h>

#include "usd_fallback_proxy/arnold_utils/api.h"

OPENDCC_NAMESPACE_OPEN

unsigned ARNOLD_UTILS_API get_arnold_node_type(const PXR_NS::TfToken& prim_type);
PXR_NS::VtTokenArray ARNOLD_UTILS_API get_nodes_by_type(unsigned node_type);
PXR_NS::VtTokenArray ARNOLD_UTILS_API get_allowed_tokens(const AtParamEntry* param);
PXR_NS::VtValue ARNOLD_UTILS_API get_value_for_arnold_param(const AtParamEntry* param);

/**
 * @brief Returns PXR_NS::SdfLayerRefPtr with the Prim '/temp_prim' containing all attributes of the Arnold node. If such an Arnold node does not
 * exist, it returns nullptr.
 *
 * @param node_type The type of Arnold node. Valid values are stored in ai_node_entry.h and start with AI_NODE_.
 * @param node_entry_type The internal name of an Arnold node can be obtained using kick, which is supplied with Arnold. (kick -nodes, kick -info
 * <node_entry_type>)
 * @param attr_namespace The namespace for attributes of an Arnold node Prim in PXR_NS::SdfLayerRefPtr.
 */
PXR_NS::SdfLayerRefPtr ARNOLD_UTILS_API get_arnold_entry_map(unsigned node_type, const std::string& node_entry_type,
                                                             const std::string& attr_namespace = std::string());

OPENDCC_NAMESPACE_CLOSE
