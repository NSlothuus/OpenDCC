// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/base/packaging/package_entry_point.h"
#include "opendcc/base/pybind_bridge/usd.h"
#include "opendcc/base/packaging/package_registry.h"
#include "pxr/base/tf/pyResultConversions.h"
#include "opendcc/base/packaging/filesystem_package_provider.h"
#include "opendcc/base/packaging/toml_parser.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

using namespace pybind11;

namespace
{
    struct PackageEntryPointWrap : PackageEntryPoint
    {
        using PackageEntryPoint::PackageEntryPoint;

        virtual void initialize(const Package& package) override
        {
            OPENDCC_PYBIND11_OVERRIDE_EXCEPTION_SAFE(void, PackageEntryPoint, initialize, package);
        }
        virtual void uninitialize(const Package& package) override
        {
            OPENDCC_PYBIND11_OVERRIDE_EXCEPTION_SAFE(void, PackageEntryPoint, uninitialize, package);
        }
    };
};

PYBIND11_MODULE(PYMODULE_NAME, m)
{
    class_<Package>(m, "Package")
        .def("get_name", &Package::get_name)
        .def("get_all_attributes", &Package::get_all_attributes)
        .def("get_attribute", &Package::get_attribute)
        .def("is_loaded", &Package::is_loaded)
        .def("get_direct_dependencies", &Package::get_direct_dependencies)
        .def("get_root_dir", &Package::get_root_dir);

    class_<PackageEntryPoint, PackageEntryPointWrap>(m, "PackageEntryPoint")
        .def(init<>())
        .def("initialize", &PackageEntryPoint::initialize)
        .def("uninitialize", &PackageEntryPoint::uninitialize);

    class_<PackageRegistry, std::unique_ptr<PackageRegistry, nodelete>>(m, "PackageRegistry")
        .def("fetch_packages", &PackageRegistry::fetch_packages, "load_fetched"_a = false)
        .def("add_package_provider", &PackageRegistry::add_package_provider, "package_resolver"_a)
        .def("remove_package_provider", &PackageRegistry::remove_package_provider, "package_resolver"_a)
        .def("get_all_packages", &PackageRegistry::get_all_packages)
        .def("get_package", &PackageRegistry::get_package, "name"_a)
        .def("define_token", &PackageRegistry::define_token, "token_name"_a, "token_value"_a)
        .def("undefine_token", &PackageRegistry::undefine_token, "token_name"_a)
        .def("load", overload_cast<const std::string&>(&PackageRegistry::load), "pkg_name"_a)
        .def("load", overload_cast<const Package&>(&PackageRegistry::load), "pkg"_a);

    class_<PackageProvider, std::shared_ptr<PackageProvider>>(m, "PackageProvider")
        .def("fetch", &PackageProvider::fetch)
        .def("get_cached_packages", &PackageProvider::get_cached_packages);

    class_<FileSystemPackageProvider, PackageProvider, std::shared_ptr<FileSystemPackageProvider>>(m, "FileSystemPackageProvider")
        .def(init<>())
        .def("fetch", &FileSystemPackageProvider::fetch)
        .def("get_cached_packages", &FileSystemPackageProvider::get_cached_packages)
        .def("register_package_parser", &FileSystemPackageProvider::register_package_parser)
        .def("add_path", &FileSystemPackageProvider::add_path)
        .def("remove_path", &FileSystemPackageProvider::remove_path);

    class_<PackageAttribute>(m, "PackageAttribute").def_readwrite("name", &PackageAttribute::name).def_readwrite("value", &PackageAttribute::value);

    class_<PackageData>(m, "PackageData")
        .def_readwrite("name", &PackageData::name)
        .def_readwrite("path", &PackageData::path)
        .def_readwrite("raw_attributes", &PackageData::raw_attributes);

    class_<PackageParser, std::shared_ptr<PackageParser>>(m, "PackageParser").def("parse", &PackageParser::parse);

    class_<TOMLParser, PackageParser, std::shared_ptr<TOMLParser>>(m, "TOMLParser").def(init<>()).def("parse", &TOMLParser::parse);
}

OPENDCC_NAMESPACE_CLOSE
