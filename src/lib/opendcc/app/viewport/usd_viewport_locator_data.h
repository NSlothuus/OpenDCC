/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/viewport/viewport_locator_data.h"
#include "opendcc/app/viewport/viewport_usd_locator.h"

OPENDCC_NAMESPACE_OPEN

class ViewportLocatorDelegate;

class UsdViewportLocatorData
{
public:
    UsdViewportLocatorData(ViewportLocatorDelegate* scene_delegate);
    ~UsdViewportLocatorData() = default;

    bool is_locator(const PXR_NS::SdfPath& path) const;
    bool insert_locator(const PXR_NS::SdfPath& path, const PXR_NS::UsdTimeCode& time = PXR_NS::UsdTimeCode::Default());
    void remove_locator(const PXR_NS::SdfPath& path);
    void mark_locator_dirty(const PXR_NS::SdfPath& path, PXR_NS::HdDirtyBits bits);
    void update(const PXR_NS::SdfPath& path, const PXR_NS::UsdTimeCode& time = PXR_NS::UsdTimeCode::Default());

    void initialize_locators(const PXR_NS::UsdTimeCode& time = PXR_NS::UsdTimeCode::Default());
    bool contains_light(const PXR_NS::SdfPath& path) const;
    bool contains_texture(const PXR_NS::SdfPath& path) const;

private:
    ViewportLocatorDelegate* m_delegate = nullptr;
};

OPENDCC_NAMESPACE_CLOSE
