/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/packaging/api.h"
#include <pxr/base/vt/value.h>

#include <unordered_map>
#include "pxr/base/vt/dictionary.h"
#include "pxr/base/vt/array.h"

OPENDCC_NAMESPACE_OPEN

class Package;
class PackageEntryPoint;

struct PackageAttribute
{
    std::string name;
    PXR_NS::VtValue value;

    bool is_valid() const { return !name.empty(); }
    operator bool() const { return is_valid(); }
};

struct PackageSharedData
{
    PXR_NS::VtDictionary resolved_attributes;
    PXR_NS::VtDictionary direct_dependencies;
    std::string root_dir;
    std::string name;
    bool loaded = false;

    template <class T>
    const T& get_resolved(const std::string& path, const T& default_ = T()) const
    {
        const auto res = resolved_attributes.GetValueAtPath(path, ".");
        return res ? res->Get<T>() : default_;
    }
    std::string get_resolved(const std::string& path, const char* default_ = "") const { return get_resolved<std::string>(path, default_); }

    struct LoadedEntity
    {
        virtual bool close() = 0;
        virtual void uninitialize(const Package& package) = 0;
    };
    std::vector<std::unique_ptr<LoadedEntity>> loaded_entities;
};

class OPENDCC_PACKAGING_API Package
{
public:
    Package() = default;
    Package(const std::shared_ptr<PackageSharedData>& data);
    const std::string& get_name() const;

    const PXR_NS::VtDictionary& get_all_attributes() const;
    const PXR_NS::VtValue& get_attribute(const std::string& name) const;
    bool is_loaded() const;
    const PXR_NS::VtDictionary& get_direct_dependencies() const;
    const std::string& get_root_dir() const;

    bool is_valid() const;
    operator bool() const { return is_valid(); }

private:
    std::shared_ptr<PackageSharedData> m_data;
};

OPENDCC_NAMESPACE_CLOSE
