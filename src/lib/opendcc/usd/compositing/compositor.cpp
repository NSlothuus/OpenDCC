// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd/compositing/compositor.h"
#include "opendcc/usd/compositing/layer.h"

#include <pxr/imaging/garch/glApi.h>
#include <pxr/imaging/hgi/tokens.h>
#include <pxr/base/vt/value.h>

OPENDCC_NAMESPACE_OPEN

void Compositor::add_layer(const std::shared_ptr<Layer> layer)
{
    m_layers.push_back(layer);
}

void Compositor::composite(const int in, PXR_NS::Hgi* hgi)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_THIRD_PARTY, 0, -1, "Compositor");

    const auto in_value = PXR_NS::VtValue(static_cast<uint32_t>(in));
    for (const auto layer : m_layers)
    {
        if (!layer->begin_render())
        {
            layer->end_render();
            continue;
        }

        while (!layer->is_finished())
        {
            if (!layer->begin_frame())
            {
                layer->end_frame();
                continue;
            }
            if (!layer->render_frame())
            {
                layer->end_frame();
                continue;
            }
            if (!layer->end_frame())
            {
                continue;
            }

            const auto& info = layer->get_frame_info();
            if (!info.valid())
            {
                continue;
            }
            m_interop.TransferToApp(hgi, info.color, info.depth, PXR_NS::HgiTokens->OpenGL, in_value, info.region);
        }

        layer->end_render();
    }

    glPopDebugGroup();
}

OPENDCC_NAMESPACE_CLOSE
