// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/core/wrap_usd_clipboard.h"
#include "opendcc/base/pybind_bridge/usd.h"
#include "opendcc/app/core/usd_clipboard.h"

OPENDCC_NAMESPACE_OPEN
using namespace pybind11;
PXR_NAMESPACE_USING_DIRECTIVE
void py_interp::bind::wrap_usd_clipboard(pybind11::module& m)
{
    class_<UsdClipboard>(m, "UsdClipboard")
        .def("get_clipboard", &UsdClipboard::get_clipboard)
        .def("clear_clipboard", &UsdClipboard::clear_clipboard)
        .def("set_clipboard", &UsdClipboard::set_clipboard)
        .def("set_clipboard_path", &UsdClipboard::set_clipboard_path)
        .def("set_clipboard_file_format", &UsdClipboard::set_clipboard_file_format)
        .def("save_clipboard_data", &UsdClipboard::save_clipboard_data)
        .def("set_clipboard_attribute", &UsdClipboard::set_clipboard_attribute)
        .def("set_clipboard_stage", &UsdClipboard::set_clipboard_stage)
        .def("get_clipboard_attribute", &UsdClipboard::get_clipboard_attribute)
        .def("get_clipboard_stage", &UsdClipboard::get_clipboard_stage)
        .def("get_new_clipboard_stage", &UsdClipboard::get_new_clipboard_stage)
        .def("get_new_clipboard_attribute", &UsdClipboard::get_new_clipboard_attribute);
}
OPENDCC_NAMESPACE_CLOSE
