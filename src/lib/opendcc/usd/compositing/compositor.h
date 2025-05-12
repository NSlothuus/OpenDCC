/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/usd/compositing/api.h"
#include "opendcc/usd/compositing/layer.h"

#include <pxr/imaging/hgi/hgi.h>
#include <pxr/imaging/hgiInterop/hgiInterop.h>

#include <vector>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_COMPOSITING_API Compositor
{
public:
    void add_layer(const std::shared_ptr<Layer> layer);

    void composite(const int in, PXR_NS::Hgi* hgi);

private:
    std::vector<LayerPtr> m_layers;

    PXR_NS::HgiInterop m_interop;
};
using CompositorPtr = std::shared_ptr<Compositor>;

OPENDCC_NAMESPACE_CLOSE
