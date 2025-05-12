/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/base/packaging/package_entry_point.h"

OPENDCC_NAMESPACE_OPEN

class HydraOpSceneGraphEntryPoint : public PackageEntryPoint
{
public:
    HydraOpSceneGraphEntryPoint();
    virtual ~HydraOpSceneGraphEntryPoint();
    void initialize(const Package& package) override;
    void uninitialize(const Package& package) override;
};

OPENDCC_NAMESPACE_CLOSE
