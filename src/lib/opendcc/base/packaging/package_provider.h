/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/packaging/api.h"
#include "opendcc/base/packaging/package_parser.h"

OPENDCC_NAMESPACE_OPEN

class PackageProvider
{
public:
    virtual ~PackageProvider() = default;
    virtual void fetch() = 0;
    virtual const std::vector<PackageData>& get_cached_packages() const = 0;
};

OPENDCC_NAMESPACE_CLOSE
