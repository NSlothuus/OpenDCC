// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "../color_theme.h"
#include <pybind11/pybind11.h>

OPENDCC_NAMESPACE_OPEN
using namespace pybind11;

PYBIND11_MODULE(color_theme, m)
{
    enum_<ColorTheme>(m, "ColorTheme").value("DARK", ColorTheme::DARK).value("LIGHT", ColorTheme::LIGHT);

    m.def("get_color_theme", &get_color_theme);
    m.def("set_color_theme", &set_color_theme);
}

OPENDCC_NAMESPACE_CLOSE
