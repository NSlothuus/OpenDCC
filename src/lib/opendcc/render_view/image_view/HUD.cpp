// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <QImage>
#include <QPainter>
#include <GL/glew.h>
#include "HUD.hpp"

#include "gl_utils.hpp"

OPENDCC_NAMESPACE_OPEN

GLint RenderViewHUD::m_simple_textue_shader_program;

void RenderViewHUD::init_gl()
{
    GLint m_simple_texture_fragment_shader;
    GLint m_simple_texture_vertex_shader;
    GLint status;
    m_simple_textue_shader_program = 0;
    m_simple_textue_shader_program = glCreateProgram();
    m_simple_texture_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    gl_utils::gl_shader_source(m_simple_texture_vertex_shader, ":/shaders/simple_texture.vert");
    glCompileShader(m_simple_texture_vertex_shader);

    glGetShaderiv(m_simple_texture_vertex_shader, GL_COMPILE_STATUS, &status);
    glAttachShader(m_simple_textue_shader_program, m_simple_texture_vertex_shader);

    m_simple_texture_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_utils::gl_shader_source(m_simple_texture_fragment_shader, ":/shaders/simple_texture.frag");
    glCompileShader(m_simple_texture_fragment_shader);
    glGetShaderiv(m_simple_texture_fragment_shader, GL_COMPILE_STATUS, &status);
    glAttachShader(m_simple_textue_shader_program, m_simple_texture_fragment_shader);

    glLinkProgram(m_simple_textue_shader_program);

    auto gl_error = glGetError();
    if (gl_error != GL_NO_ERROR)
        printf("error in openGL: %d", gl_error);
}

void RenderViewHUD::add_text(int x, int y, const std::string& text)
{
    m_groups.resize(m_groups.size() + 1);
    Group& group = m_groups.back();
    group.x = x;
    group.y = y;

    glGenTextures(1, &group.texture);
    int nchannels = 4;
    group.image = QImage(text.size() * 10, 20, QImage::Format_RGBA8888);
    group.image.fill(QColor(0, 0, 0, 0));
    QPainter p(&group.image);
    p.setPen(Qt::gray);
    p.drawText(0, 12, text.c_str());
    p.end();

    GLenum gltype = GL_UNSIGNED_BYTE;
    GLenum glformat = GL_RGBA;
    GLenum glinternalformat = GL_RGBA;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, group.texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, glinternalformat, group.image.width(), group.image.height(), 0, glformat, gltype, group.image.bits());

    auto gl_error = glGetError();
    if (gl_error != GL_NO_ERROR)
        printf("error in openGL: %d", gl_error);
}

void RenderViewHUD::draw(std::function<void(float widget_x, float widget_y, float& image_x, float& image_y)> image_to_widget_pos)
{
    for (Group& group : m_groups)
    {
        glUseProgram(m_simple_textue_shader_program);

        GLint loc = glGetUniformLocation(m_simple_textue_shader_program, "imgtex");
        glUniform1i(loc, 2);
        float x, y;
        image_to_widget_pos(group.x, group.y, x, y);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, group.texture);
        gl_utils::gl_rect_poly(x + 3, y - 15, x + group.image.width() + 3, y + +group.image.height() - 15);
    }
    auto gl_error = glGetError();
    if (gl_error != GL_NO_ERROR)
        printf("error in openGL: %d", gl_error);
}

RenderViewHUD::Group::~Group()
{
    glDeleteTextures(1, &texture);
    auto gl_error = glGetError();
    if (gl_error != GL_NO_ERROR)
        printf("error in openGL: %d", gl_error);
}

OPENDCC_NAMESPACE_CLOSE
