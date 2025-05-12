/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include <pxr/base/gf/vec4i.h>
#include <pxr/imaging/hgi/texture.h>

OPENDCC_NAMESPACE_OPEN

struct FrameInfo
{
    PXR_NS::HgiTextureHandle color;
    PXR_NS::HgiTextureHandle depth;
    PXR_NS::GfVec4i region;

    bool valid() const;
};

OPENDCC_NAMESPACE_CLOSE
