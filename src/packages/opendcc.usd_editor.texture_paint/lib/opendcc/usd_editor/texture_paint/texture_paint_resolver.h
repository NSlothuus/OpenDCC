/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include <pxr/usd/ar/defaultResolver.h>

OPENDCC_NAMESPACE_OPEN

class TextureResolver : public PXR_NS::ArDefaultResolver
{
public:
protected:
    PXR_NS::ArResolvedPath _Resolve(const std::string& assetPath) const override;
};

OPENDCC_NAMESPACE_CLOSE
