/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <pxr/imaging/hdx/shadowMatrixComputation.h>

OPENDCC_NAMESPACE_OPEN

class ShadowMatrix : public PXR_NS::HdxShadowMatrixComputation
{
public:
    ShadowMatrix(const PXR_NS::GfMatrix4d& shadow_matrix)
        : m_shadow_matrix(shadow_matrix)
    {
    }

#if PXR_VERSION >= 2005
    virtual std::vector<PXR_NS::GfMatrix4d> Compute(const PXR_NS::GfVec4f& viewport, PXR_NS::CameraUtilConformWindowPolicy policy) override
    {
        return std::vector<PXR_NS::GfMatrix4d>(1, m_shadow_matrix);
    }
#if PXR_VERSION >= 2108
    virtual std::vector<PXR_NS::GfMatrix4d> Compute(const PXR_NS::CameraUtilFraming& framing, PXR_NS::CameraUtilConformWindowPolicy policy) override
    {
        return { m_shadow_matrix };
    }
#endif
#else
    virtual PXR_NS::GfMatrix4d Compute(const PXR_NS::GfVec4f& viewport, PXR_NS::CameraUtilConformWindowPolicy policy) override
    {
        return m_shadow_matrix;
    }
#endif
private:
    PXR_NS::GfMatrix4d m_shadow_matrix;
};

OPENDCC_NAMESPACE_CLOSE
