// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/base/packaging/package.h"
#include "opendcc/base/packaging/core_properties.h"

PXR_NAMESPACE_USING_DIRECTIVE
OPENDCC_NAMESPACE_OPEN

const std::string& Package::get_name() const
{
    return m_data->name;
}

const VtDictionary& Package::get_all_attributes() const
{
    return m_data->resolved_attributes;
}

Package::Package(const std::shared_ptr<PackageSharedData>& data)
    : m_data(data)
{
}

const VtValue& Package::get_attribute(const std::string& name) const
{
    auto it = m_data->resolved_attributes.GetValueAtPath(name, ".");
    if (it == nullptr)
    {
        static const VtValue empty;
        return empty;
    }
    return *it;
}

bool Package::is_loaded() const
{
    return m_data->loaded;
}

const VtDictionary& Package::get_direct_dependencies() const
{
    return m_data->direct_dependencies;
}

const std::string& Package::get_root_dir() const
{
    return m_data->root_dir;
}

bool OPENDCC_NAMESPACE::Package::is_valid() const
{
    return m_data != nullptr;
}

OPENDCC_NAMESPACE_CLOSE
