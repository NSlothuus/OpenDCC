/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include <string>

OPENDCC_NAMESPACE_OPEN

enum class Mode
{
    Sculpt = 0,
    Flatten = 1,
    Move = 2,
    Noise = 3,
    Smooth = 4,
    Relax = 5,
};

struct Properties
{
    static constexpr float default_strength = 0.2f;
    static constexpr float default_radius = 1.0f;
    static constexpr float default_falloff = 0.3f;

    float strength = default_strength;
    float radius = default_radius;
    float falloff = default_falloff;

    Mode mode = Mode::Sculpt;

    void read_from_settings(const std::string& prefix);
    void write_to_settings(const std::string& prefix) const;
};

OPENDCC_NAMESPACE_CLOSE
