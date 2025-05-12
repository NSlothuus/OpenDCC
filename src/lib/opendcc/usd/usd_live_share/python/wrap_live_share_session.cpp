// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd/usd_live_share/live_share_session.h"
#include "opendcc/base/pybind_bridge/usd.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

PYBIND11_MODULE(MODULE_NAME, m)
{
    using namespace pybind11;

    class_<LiveShareSession>(m, "LiveShareSession")
        .def(init<UsdStageRefPtr>())
        .def("start_share", &LiveShareSession::start_share)
        .def("stop_share", &LiveShareSession::stop_share);
}
OPENDCC_NAMESPACE_CLOSE
