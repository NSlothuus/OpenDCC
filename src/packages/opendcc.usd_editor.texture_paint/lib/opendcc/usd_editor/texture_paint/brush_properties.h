/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/ui/common_widgets/ramp.h"
#include <pxr/base/gf/vec3f.h>

OPENDCC_NAMESPACE_OPEN

class BrushProperties
{
public:
    BrushProperties();

    void set_radius(int radius);
    int get_radius() const;
    void set_color(const PXR_NS::GfVec3f& color);
    const PXR_NS::GfVec3f& get_color() const;
    std::shared_ptr<Ramp<float>> get_falloff_curve();
    void update_falloff_curve();

private:
    int m_radius;
    PXR_NS::GfVec3f m_color;
    std::shared_ptr<Ramp<float>> m_falloff_curve;
};

OPENDCC_NAMESPACE_CLOSE
