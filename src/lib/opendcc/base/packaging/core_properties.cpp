// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/base/packaging/core_properties.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/vt/dictionary.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

std::unordered_map<std::string, PXR_NS::VtValue> s_core_property_defaults = {
    { "base.name", VtValue("") },
    { "base.first_entry_point", VtValue("cpp") },
    { "base.root", VtValue(".") },
    { "base.unloadable", VtValue(false) },
    { "dependencies", VtValue(VtDictionary()) },
    { "native.entry_point", VtValue(VtArray<VtDictionary>()) },
    { "native.load", VtValue(VtArray<VtDictionary>()) },
    { "python.entry_point", VtValue(VtArray<VtDictionary>()) },
    { "python.import", VtValue(VtArray<VtDictionary>()) },
};

OPENDCC_NAMESPACE_CLOSE
