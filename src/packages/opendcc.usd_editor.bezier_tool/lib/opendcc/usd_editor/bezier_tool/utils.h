/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/viewport/viewport_view.h"

#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/frustum.h>

#include <type_traits>
#include <limits>

OPENDCC_NAMESPACE_OPEN

constexpr const float epsilon = 10000 * std::numeric_limits<float>::epsilon();

PXR_NS::GfVec2f clip_from_screen(const PXR_NS::GfVec2f& screen, const int w, const int h);
PXR_NS::GfVec2f screen_from_clip(const PXR_NS::GfVec2f& clip, const int w, const int h);

PXR_NS::GfMatrix4d compute_view_projection(const ViewportViewPtr& viewport_view);

bool lie_on_one_line(const PXR_NS::GfVec3f& f, const PXR_NS::GfVec3f& s, const PXR_NS::GfVec3f& t);

OPENDCC_NAMESPACE_CLOSE
