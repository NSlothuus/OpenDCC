// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/sculpt_tool/sculpt_properties.h"

#include "opendcc/app/core/application.h"

OPENDCC_NAMESPACE_OPEN
constexpr float Properties::default_strength;
constexpr float Properties::default_radius;
constexpr float Properties::default_falloff;

void Properties::read_from_settings(const std::string& prefix)
{
    auto settings = Application::instance().get_settings();

    mode = static_cast<Mode>(settings->get(prefix + ".last_mode", 0));

    const auto mode_postfix = "." + std::to_string(static_cast<int>(mode));

    radius = settings->get(prefix + ".radius" + mode_postfix, default_radius);
    strength = settings->get(prefix + ".strength" + mode_postfix, default_strength);
    falloff = settings->get(prefix + ".falloff" + mode_postfix, default_falloff);
}

void Properties::write_to_settings(const std::string& prefix) const
{
    auto settings = Application::instance().get_settings();

    settings->set(prefix + ".last_mode", static_cast<int>(mode));

    const auto mode_postfix = '.' + std::to_string(static_cast<int>(mode));

    settings->set(prefix + ".radius" + mode_postfix, radius);
    settings->set(prefix + ".strength" + mode_postfix, strength);
    settings->set(prefix + ".falloff" + mode_postfix, falloff);
}

OPENDCC_NAMESPACE_CLOSE
