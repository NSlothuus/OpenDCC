/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/usd/usdUtils/timeCodeRange.h>
#include <pxr/usd/usd/stage.h>
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN
class ViewportCameraMapper;
class ViewportGLWidget;

class OPENDCC_API IStageResolver
{
public:
    struct StageEntry
    {
        PXR_NS::SdfPath prefix;
        PXR_NS::UsdStageRefPtr stage;
        PXR_NS::UsdUtilsTimeCodeRange time_range;
    };

    struct StageResolve
    {
        PXR_NS::SdfPath prefix;
        PXR_NS::UsdStageRefPtr stage;
        PXR_NS::UsdTimeCode local_time;
    };

    virtual ~IStageResolver() = default;

    // Pure virtual functions to be implemented by derived classes
    virtual PXR_NS::UsdStageRefPtr get_stage(PXR_NS::SdfPath prefix) const = 0;
    virtual std::vector<StageResolve> get_stages(PXR_NS::UsdTimeCode time) const = 0;
    virtual std::vector<StageEntry> get_stages() const = 0;
    virtual PXR_NS::SdfPathVector get_stage_roots(PXR_NS::UsdTimeCode time) const = 0;
    virtual PXR_NS::SdfPathVector get_stage_roots() const = 0;
    virtual PXR_NS::UsdTimeCode resolve_time(PXR_NS::SdfPath stage, PXR_NS::UsdTimeCode global_time) const = 0;
    virtual std::shared_ptr<ViewportCameraMapper> create_camera_mapper(const PXR_NS::SdfPath& path, ViewportGLWidget* gl_widget) = 0;

    // Optional default implementations
    virtual void mark_clean() { /* default no-op */ }
    virtual bool is_dirty() const { return false; /* default no-op */ }

protected:
    // Default constructor
    IStageResolver() = default;
};

OPENDCC_NAMESPACE_CLOSE
