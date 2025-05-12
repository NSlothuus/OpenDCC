/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "api.h"

OPENDCC_NAMESPACE_OPEN

enum class ColorTheme
{
    DARK,
    LIGHT
};

OPENDCC_COLOR_THEME_API ColorTheme get_color_theme();
OPENDCC_COLOR_THEME_API void set_color_theme(ColorTheme color_theme);

OPENDCC_NAMESPACE_CLOSE
