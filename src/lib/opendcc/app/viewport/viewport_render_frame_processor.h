/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <opendcc/opendcc.h>
#include "opendcc/app/viewport/viewport_engine_proxy.h"
#include <memory>

OPENDCC_NAMESPACE_OPEN

class ViewportHydraEngineParams;
class ViewportEngineProxy;

class ViewportRenderFrameProcessor
{
public:
    virtual void pre_frame(const ViewportHydraEngineParams& params, std::shared_ptr<ViewportEngineProxy> engine) {};
    virtual void post_frame(const ViewportHydraEngineParams& params, std::shared_ptr<ViewportEngineProxy> engine) {};
};

OPENDCC_NAMESPACE_CLOSE
