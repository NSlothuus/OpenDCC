/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>

#include "opendcc/usd_editor/sculpt_tool/sculpt_strategies.h"
#include "opendcc/usd_editor/sculpt_tool/sculpt_properties.h"
#include "opendcc/usd_editor/sculpt_tool/sculpt_tool_context.h"

OPENDCC_NAMESPACE_OPEN

struct MeshManipulationData;

struct SculptIn
{
    std::shared_ptr<MeshManipulationData> mesh_data;
    Properties properties;
    PXR_NS::GfVec3f hit_normal;
    PXR_NS::GfVec3f hit_point;
    PXR_NS::GfVec3f direction;
    bool inverts = false;
};

PXR_NS::VtVec3fArray sculpt(const SculptIn& in, const PXR_NS::VtVec3fArray& prev_points, const std::vector<int>& indices);

OPENDCC_NAMESPACE_CLOSE
