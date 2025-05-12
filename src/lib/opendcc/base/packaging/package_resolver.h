/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/packaging/api.h"
#include "opendcc/base/packaging/package.h"
#include "opendcc/base/vendor/nonstd/optional.hpp"

OPENDCC_NAMESPACE_OPEN

class PackageResolver
{
public:
    void set_packages(const std::unordered_map<std::string, std::shared_ptr<PackageSharedData>>& pkg_data);
    std::vector<std::string> get_dependencies(const std::string& pkg_name) const;
    std::vector<std::string> get_dependees(const std::string& package_name) const;

private:
    std::vector<std::string> traverse(const std::string& pkg_name,
                                      const std::function<const PXR_NS::VtStringArray&(const std::string&)>& get_deps_fn) const;

    // Package to outcoming deps
    struct PackageRelations
    {
        PXR_NS::VtStringArray dependencies;
        PXR_NS::VtStringArray dependees;
        bool circular = false;
    };
    std::unordered_map<std::string, PackageRelations> m_package_relations;
};

OPENDCC_NAMESPACE_CLOSE
