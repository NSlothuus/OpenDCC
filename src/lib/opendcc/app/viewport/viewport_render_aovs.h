/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/app/viewport/viewport_render_frame_processor.h"
#include <pxr/imaging/hgi/texture.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/usd/timeCode.h>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API ViewportRenderAOVs : public ViewportRenderFrameProcessor
{
public:
    struct AOV
    {
        std::string name;
        PXR_NS::HgiTextureDesc desc;
        std::vector<uint8_t> data;
    };

    void post_frame(const ViewportHydraEngineParams& params, std::shared_ptr<ViewportEngineProxy> engine) override;

    const std::vector<AOV>& get_aovs() const;
    void flip(bool flip);
    bool is_flipped() const;

private:
    std::vector<uint8_t> read_texture(const PXR_NS::HgiTextureHandle& handle) const;

    std::vector<AOV> m_aovs;
    bool m_flip = false;
};

OPENDCC_NAMESPACE_CLOSE
