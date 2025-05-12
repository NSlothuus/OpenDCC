/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/usd/compositing/api.h"
#include "opendcc/usd/compositing/frame_info.h"

#include <memory>

OPENDCC_NAMESPACE_OPEN

class ViewportWidget;

class OPENDCC_COMPOSITING_API Layer
{
public:
    Layer() = default;
    virtual ~Layer() = default;

    virtual bool begin_render() = 0;
    virtual bool end_render() = 0;

    virtual bool begin_frame() = 0;
    virtual bool render_frame() = 0;
    virtual bool end_frame() = 0;

    virtual const FrameInfo& get_frame_info() const = 0;

    virtual bool is_finished() const = 0;
};
using LayerPtr = std::shared_ptr<Layer>;

OPENDCC_NAMESPACE_CLOSE
