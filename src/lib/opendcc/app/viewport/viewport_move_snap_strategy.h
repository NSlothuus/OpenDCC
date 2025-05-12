/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/base/gf/vec3d.h>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API ViewportSnapStrategy
{
public:
    virtual PXR_NS::GfVec3d get_snap_point(const PXR_NS::GfVec3d& start_pos, const PXR_NS::GfVec3d& start_drag, const PXR_NS::GfVec3d& cur_drag) = 0;
};

class OPENDCC_API ViewportRelativeSnapStrategy : public ViewportSnapStrategy
{
public:
    ViewportRelativeSnapStrategy(double step);
    PXR_NS::GfVec3d get_snap_point(const PXR_NS::GfVec3d& start_pos, const PXR_NS::GfVec3d& start_drag, const PXR_NS::GfVec3d& cur_drag) override;

private:
    double m_step;
};

class OPENDCC_API ViewportAbsoluteSnapStrategy : public ViewportSnapStrategy
{
public:
    ViewportAbsoluteSnapStrategy(double step);
    PXR_NS::GfVec3d get_snap_point(const PXR_NS::GfVec3d& start_pos, const PXR_NS::GfVec3d& start_drag, const PXR_NS::GfVec3d& cur_drag) override;

private:
    double m_step;
};
OPENDCC_NAMESPACE_CLOSE