/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <pxr/base/gf/vec2i.h>
#include "opendcc/app/viewport/viewport_view.h"

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API ViewportColorCorrection
{
public:
    enum ColorCorrectionMode
    {
        DISABLED,
        SRGB,
        OCIO
    };

    ViewportColorCorrection(ColorCorrectionMode mode, const std::string& ocio_view, const std::string& input_color_space, float gamma,
                            float exposure);
    ~ViewportColorCorrection();
    void apply(ViewportViewPtr viewport_view);
    void apply(int width, int height);
    void set_gamma(float gamma);
    void set_exposure(float exposure);
    void set_mode(ColorCorrectionMode mode);
    ColorCorrectionMode get_mode() const;
    void set_ocio_view(const std::string& view);
    void set_color_space(const std::string& color_space);

private:
    void initialize(ViewportViewPtr viewport_view);
    void initialize(int width, int height);
    void init_vertex_buffer();
    void init_shader();
    std::string get_ocio_shader_text();
    void init_framebuffer(ViewportViewPtr viewport_view);
    void init_framebuffer(int width, int height);
    void blit_intermediate();

    void apply_correction();

    bool verify_shader_compilation(GLuint shader_id) const;
    bool verify_shader_program_link(GLuint shader_id) const;

    ColorCorrectionMode m_mode;
    std::string m_view;
    std::string m_input_color_space;
    float m_gamma = 1.0f;
    float m_exposure = 0.0f;
    int m_lut3d_size = 65;
    PXR_NS::GfVec2i m_framebuffer_size;

    GLuint m_vao = 0;
    GLuint m_vertex_buffer = 0;
    GLuint m_shader_program = 0;
    GLuint m_texture_3d_lut = 0;
    GLuint m_intermediate_texture = 0;
    GLuint m_intermediate_framebuffer = 0;
    GLint m_color_in_loc = -1;
    GLint m_lut3d_in_loc = -1;
};

OPENDCC_NAMESPACE_CLOSE
