// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd/compositing/frame_info.h"

OPENDCC_NAMESPACE_OPEN

bool FrameInfo::valid() const
{
    return color && depth && region.GetLengthSq() != 0.0;
}

OPENDCC_NAMESPACE_CLOSE
