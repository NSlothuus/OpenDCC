// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/wrap_def_cam_settings.h"
#include "opendcc/app/viewport/def_cam_settings.h"
#include <pybind11/pybind11.h>

OPENDCC_NAMESPACE_OPEN
using namespace pybind11;
void py_interp::bind::wrap_def_cam_settings(pybind11::module& m)
{
    class_<DefCamSettings>(m, "DefCamSettings")
        .def_static("instance", &DefCamSettings::instance, return_value_policy::reference)
        .def_property("fov", &DefCamSettings::get_fov, &DefCamSettings::set_fov)
        .def_property("near_clip_plane", &DefCamSettings::get_near_clip_plane, &DefCamSettings::set_near_clip_plane)
        .def_property("far_clip_plane", &DefCamSettings::get_far_clip_plane, &DefCamSettings::set_far_clip_plane)
        .def_property("focal_length", &DefCamSettings::get_focal_length, &DefCamSettings::set_focal_length)
        .def_property("horizontal_aperture", &DefCamSettings::get_horizontal_aperture, &DefCamSettings::set_horizontal_aperture)
        .def_property("vertical_aperture", &DefCamSettings::get_vertical_aperture, &DefCamSettings::set_vertical_aperture)
        .def_property("aspect_ratio", &DefCamSettings::get_aspect_ratio, &DefCamSettings::set_aspect_ratio)
        .def_property("is_perspective", &DefCamSettings::is_perspective, &DefCamSettings::set_perspective)
        .def_property("orthographic_size", &DefCamSettings::get_orthographic_size, &DefCamSettings::set_orthographic_size)
        .def("save_settings", &DefCamSettings::save_settings);
}

OPENDCC_NAMESPACE_CLOSE
