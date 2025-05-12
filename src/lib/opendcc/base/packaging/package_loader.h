/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/packaging/api.h"
#include "opendcc/base/packaging/package.h"
#include "opendcc/base/packaging/package_resolver.h"

OPENDCC_NAMESPACE_OPEN

class PackageLoader
{
public:
    PackageLoader(std::shared_ptr<PackageResolver> pkg_resolver,
                  std::unordered_map<std::string, std::shared_ptr<PackageSharedData>>& pkg_shared_data);
    ~PackageLoader();

    bool load(const std::string& pkg_name);
    bool unload(const std::string& pkg_name);

private:
    bool load_cpp_libs(const std::shared_ptr<PackageSharedData>& pkg_data);
    bool load_python_modules(const std::shared_ptr<PackageSharedData>& pkg_data);

    void unload_cpp_libs(const std::shared_ptr<PackageSharedData>& pkg_data);

    bool extend_python_path(const std::string& path, const std::string& pkg_name, const std::string& dependent_package) const;

    std::shared_ptr<PackageResolver> m_pkg_resolver;
    std::unordered_map<std::string, std::shared_ptr<PackageSharedData>>& m_pkg_shared_data;
};

OPENDCC_NAMESPACE_CLOSE
