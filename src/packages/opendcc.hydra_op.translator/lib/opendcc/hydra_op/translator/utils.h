/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "schema/tokens.h"

OPENDCC_NAMESPACE_OPEN

inline PXR_NS::TfToken get_default_output_name()
{
    return PXR_NS::UsdHydraOpTokens->outputsOut;
}

inline PXR_NS::TfTokenVector get_default_inputs()
{
    return { PXR_NS::UsdHydraOpTokens->inputsIn };
}

inline PXR_NS::TfToken get_group_output_name()
{
    return get_default_output_name();
}

OPENDCC_NAMESPACE_CLOSE
