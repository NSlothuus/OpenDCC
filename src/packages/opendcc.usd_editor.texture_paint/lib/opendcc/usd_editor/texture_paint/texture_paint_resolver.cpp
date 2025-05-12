// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "texture_paint_resolver.h"
#include <pxr/usd/ar/defineResolver.h>
#include <pxr/base/tf/stringUtils.h>
#include "opendcc/app/viewport/texture_plugin.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

AR_DEFINE_RESOLVER(TextureResolver, ArDefaultResolver);

ArResolvedPath TextureResolver::_Resolve(const std::string& assetPath) const
{
    if (TfStringStartsWith(assetPath, "texblock://"))
    {
        if (InMemoryTextureRegistry::instance().get_texture(assetPath))
            return ArResolvedPath(assetPath);
    }

    return ArDefaultResolver::_Resolve(assetPath);
}

OPENDCC_NAMESPACE_CLOSE
