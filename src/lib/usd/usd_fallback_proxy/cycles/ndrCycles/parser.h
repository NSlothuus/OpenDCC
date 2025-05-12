/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "api.h"
#include <pxr/usd/ndr/parserPlugin.h>
#include "opendcc/opendcc.h"

OPENDCC_NAMESPACE_OPEN

class NdrCyclesParserPlugin : public PXR_NS::NdrParserPlugin
{
public:
    NdrCyclesParserPlugin();
    ~NdrCyclesParserPlugin() override;

    PXR_NS::NdrNodeUniquePtr Parse(const PXR_NS::NdrNodeDiscoveryResult& discoveryResult) override;

    const PXR_NS::NdrTokenVec& GetDiscoveryTypes() const override;
    const PXR_NS::TfToken& GetSourceType() const override;
};

OPENDCC_NAMESPACE_CLOSE
