// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include "opendcc/opendcc.h"
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/imagebufalgo_util.h>
#include <OpenColorIO/OpenColorIO.h>

OPENDCC_NAMESPACE_OPEN

namespace OCIO = OCIO_NAMESPACE;

//
// OIIO color convert allow using OCIO processor but only from disk (as far I can found this)
// so we strip out some of internal implementation and make it work properly from in memory color config
//
class ColorProcessor_OCIO
{
public:
    ColorProcessor_OCIO(OCIO::ConstProcessorRcPtr p)
        : m_p(p) {};

    virtual ~ColorProcessor_OCIO(void) {};

    bool isNoOp() const { return m_p->isNoOp(); }
    bool hasChannelCrosstalk() const { return m_p->hasChannelCrosstalk(); }
    void apply(float *data, int width, int height, int channels, OIIO::stride_t chanstride, OIIO::stride_t xstride, OIIO::stride_t ystride) const
    {
#if OCIO_VERSION_HEX < 0x02000000
        OCIO::PackedImageDesc pid(data, width, height, channels, chanstride, xstride, ystride);
        m_p->apply(pid);
#else
        OCIO::PackedImageDesc pid(data, width, height, channels, OCIO::BitDepth::BIT_DEPTH_F32, chanstride, xstride, ystride);
        m_p->getDefaultCPUProcessor()->apply(pid);
#endif
    }

private:
    OCIO::ConstProcessorRcPtr m_p;
};

bool OCIO_apply(OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, const ColorProcessor_OCIO *color_processor, OIIO::ROI roi, int nthreads);

OPENDCC_NAMESPACE_CLOSE
