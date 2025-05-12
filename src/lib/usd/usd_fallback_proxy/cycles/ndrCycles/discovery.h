/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include <pxr/usd/ndr/discoveryPlugin.h>

OPENDCC_NAMESPACE_OPEN

class NdrCyclesDiscoveryPlugin : public PXR_NS::NdrDiscoveryPlugin
{
public:
    using Context = PXR_NS::NdrDiscoveryPluginContext;

    NdrCyclesDiscoveryPlugin();
    ~NdrCyclesDiscoveryPlugin();

    PXR_NS::NdrNodeDiscoveryResultVec DiscoverNodes(const Context& context) override;

    const PXR_NS::NdrStringVec& GetSearchURIs() const override;
};

OPENDCC_NAMESPACE_CLOSE
