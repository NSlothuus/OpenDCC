// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include "opendcc/opendcc.h"
#include <GL/glew.h>

#include <OpenImageIO/imagebuf.h>

OPENDCC_NAMESPACE_OPEN

namespace gl_utils
{
    void typespec_to_opengl(const OIIO::ImageSpec &spec, int nchannels, GLenum &gltype, GLenum &glformat, GLenum &glinternalformat,
                            bool use_halffloat, bool use_srgb, bool use_float);

    void gl_rect_poly(float xmin, float ymin, float xmax, float ymax, float z = 0, float smin = 0, float tmin = 0, float smax = 1, float tmax = 1,
                      int rotate = 0);

    void gl_rect_lines(float xmin, float ymin, float xmax, float ymax, float z = 0, bool dotted = false);

    void gl_draw_line(float xmin, float ymin, float xmax, float ymax, float z = 0);

    void gl_shader_source(GLuint shader, const char *shader_path);
    std::string get_shader_source(const char *shader_path);
}

OPENDCC_NAMESPACE_CLOSE
