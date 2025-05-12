/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <pybind11/pybind11.h>

OPENDCC_NAMESPACE_OPEN

namespace py_interp
{
    namespace bind
    {
        void wrap_selection_list(pybind11::module& m);
    }
} // namespace py_interp
OPENDCC_NAMESPACE_CLOSE
