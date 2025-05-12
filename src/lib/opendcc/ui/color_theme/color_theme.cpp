// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "color_theme.h"

OPENDCC_NAMESPACE_OPEN

static ColorTheme g_color_theme = ColorTheme::DARK;

ColorTheme get_color_theme()
{
    return g_color_theme;
}

void set_color_theme(ColorTheme color_theme)
{
    g_color_theme = color_theme;
}

OPENDCC_NAMESPACE_CLOSE
