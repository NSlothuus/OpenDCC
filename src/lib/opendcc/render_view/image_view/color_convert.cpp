// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "color_convert.hpp"
#include <OpenEXR/OpenEXRConfig.h>
#ifdef ILMBASE_VERSION_MAJOR
#include <OpenEXR/half.h>
#elif defined(OPENEXR_VERSION_MAJOR)
#include <Imath/half.h>
#endif

OPENDCC_NAMESPACE_OPEN

template <class Rtype, class Atype>
static bool OCIO_apply_impl(OIIO::ImageBuf &R, const OIIO::ImageBuf &A, const ColorProcessor_OCIO *color_processor, OIIO::ROI roi, int nthreads)
{
    if (nthreads != 1 && roi.npixels() >= 1000)
    {
        // Possible multiple thread case -- recurse via parallel_image
        OIIO::ImageBufAlgo::parallel_image(
            OIIO::bind(OCIO_apply_impl<Rtype, Atype>, OIIO::ref(R), OIIO::cref(A), color_processor, std::placeholders::_1 /*roi*/, 1 /*nthreads*/),
            roi, nthreads);
        return true;
    }

    int width = roi.width();
    // Temporary space to hold one RGBA scanline
    std::vector<float> scanline(width * 4, 0.0f);

    // Only process up to, and including, the first 4 channels.  This
    // does let us process images with fewer than 4 channels, which is
    // the intent.
    // FIXME: Instead of loading the first 4 channels, obey
    //        Rspec.alpha_channel index (but first validate that the
    //        index is set properly for normal formats)

    int channelsToCopy = std::min(4, roi.nchannels());

    // Walk through all data in our buffer. (i.e., crop or overscan)
    // FIXME: What about the display window?  Should this actually promote
    // the datawindow to be union of data + display? This is useful if
    // the color of black moves.  (In which case non-zero sections should
    // now be promoted).  Consider the lin->log of a roto element, where
    // black now moves to non-black.

    float *dstPtr = NULL;
    const float fltmin = std::numeric_limits<float>::min();

    // If the processor has crosstalk, and we'll be using it, we should
    // reset the channels to 0 before loading each scanline.
    bool clearScanline = (channelsToCopy < 4 && (color_processor->hasChannelCrosstalk()));

    OIIO::ImageBuf::ConstIterator<Atype> a(A, roi);
    OIIO::ImageBuf::Iterator<Rtype> r(R, roi);
    for (int k = roi.zbegin; k < roi.zend; ++k)
    {
        for (int j = roi.ybegin; j < roi.yend; ++j)
        {
            // Clear the scanline
            if (clearScanline)
                memset(&scanline[0], 0, sizeof(float) * scanline.size());

            // Load the scanline
            dstPtr = &scanline[0];
            a.rerange(roi.xbegin, roi.xend, j, j + 1, k, k + 1);
            for (; !a.done(); ++a, dstPtr += 4)
                for (int c = 0; c < channelsToCopy; ++c)
                    dstPtr[c] = a[c];

            color_processor->apply(&scanline[0], width, 1, 4, sizeof(float), 4 * sizeof(float), width * 4 * sizeof(float));
            // Store the scanline
            dstPtr = &scanline[0];
            r.rerange(roi.xbegin, roi.xend, j, j + 1, k, k + 1);
            for (; !r.done(); ++r, dstPtr += 4)
                for (int c = 0; c < channelsToCopy; ++c)
                    r[c] = dstPtr[c];
        }
    }
    return true;
}

bool OCIO_apply(OIIO::ImageBuf &dst, const OIIO::ImageBuf &src, const ColorProcessor_OCIO *color_processor, OIIO::ROI roi, int nthreads)
{
    using namespace OIIO;
    bool ok = true;
    OIIO_DISPATCH_COMMON_TYPES2(ok, "OCIO_apply", OCIO_apply_impl, dst.spec().format, src.spec().format, dst, src, color_processor, roi, nthreads);
    return ok;
}

OPENDCC_NAMESPACE_CLOSE
