// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "gl_utils.hpp"
#include <QtCore/QResource>
#include <algorithm>

OPENDCC_NAMESPACE_OPEN

namespace gl_utils
{
    void typespec_to_opengl(const OIIO::ImageSpec &spec, int nchannels, GLenum &gltype, GLenum &glformat, GLenum &glinternalformat,
                            bool use_halffloat, bool use_srgb, bool use_float)
    {
        switch (spec.format.basetype)
        {
        case OIIO::TypeDesc::FLOAT:
            gltype = GL_FLOAT;
            break;
        case OIIO::TypeDesc::HALF:
            if (use_halffloat)
            {
                gltype = GL_HALF_FLOAT_ARB;
            }
            else
            {
                // If we reach here then something really wrong
                // happened: When half-float is not supported,
                // the image should be loaded as UINT8 (no GLSL
                // support) or FLOAT (GLSL support).
                // See IvImage::loadCurrentImage()
                std::cerr << "Tried to load an unsupported half-float image.\n";
                gltype = GL_INVALID_ENUM;
            }
            break;
        case OIIO::TypeDesc::INT:
            gltype = GL_INT;
            break;
        case OIIO::TypeDesc::UINT:
            gltype = GL_UNSIGNED_INT;
            break;
        case OIIO::TypeDesc::INT16:
            gltype = GL_SHORT;
            break;
        case OIIO::TypeDesc::UINT16:
            gltype = GL_UNSIGNED_SHORT;
            break;
        case OIIO::TypeDesc::INT8:
            gltype = GL_BYTE;
            break;
        case OIIO::TypeDesc::UINT8:
            gltype = GL_UNSIGNED_BYTE;
            break;
        default:
            gltype = GL_UNSIGNED_BYTE; // punt
            break;
        }

        bool issrgb = strcmp(spec.get_string_attribute("oiio:ColorSpace").c_str(), "sRGB") == 0;

        glinternalformat = nchannels;
        if (nchannels == 1)
        {
            glformat = GL_LUMINANCE;
            if (use_srgb && issrgb)
            {
                if (spec.format.basetype == OIIO::TypeDesc::UINT8)
                {
                    glinternalformat = GL_SLUMINANCE8;
                }
                else
                {
                    glinternalformat = GL_SLUMINANCE;
                }
            }
            else if (spec.format.basetype == OIIO::TypeDesc::UINT8)
            {
                glinternalformat = GL_LUMINANCE8;
            }
            else if (spec.format.basetype == OIIO::TypeDesc::UINT16)
            {
                glinternalformat = GL_LUMINANCE16;
            }
            else if (use_float && spec.format.basetype == OIIO::TypeDesc::FLOAT)
            {
                glinternalformat = GL_LUMINANCE32F_ARB;
            }
            else if (use_float && spec.format.basetype == OIIO::TypeDesc::HALF)
            {
                glinternalformat = GL_LUMINANCE16F_ARB;
            }
        }
        else if (nchannels == 2)
        {
            glformat = GL_LUMINANCE_ALPHA;
            if (use_srgb && issrgb)
            {
                if (spec.format.basetype == OIIO::TypeDesc::UINT8)
                {
                    glinternalformat = GL_SLUMINANCE8_ALPHA8;
                }
                else
                {
                    glinternalformat = GL_SLUMINANCE_ALPHA;
                }
            }
            else if (spec.format.basetype == OIIO::TypeDesc::UINT8)
            {
                glinternalformat = GL_LUMINANCE8_ALPHA8;
            }
            else if (spec.format.basetype == OIIO::TypeDesc::UINT16)
            {
                glinternalformat = GL_LUMINANCE16_ALPHA16;
            }
            else if (use_float && spec.format.basetype == OIIO::TypeDesc::FLOAT)
            {
                glinternalformat = GL_LUMINANCE_ALPHA32F_ARB;
            }
            else if (use_float && spec.format.basetype == OIIO::TypeDesc::HALF)
            {
                glinternalformat = GL_LUMINANCE_ALPHA16F_ARB;
            }
        }
        else if (nchannels == 3)
        {
            glformat = GL_RGB;
            if (use_srgb && issrgb)
            {
                if (spec.format.basetype == OIIO::TypeDesc::UINT8)
                {
                    glinternalformat = GL_SRGB8;
                }
                else
                {
                    glinternalformat = GL_SRGB;
                }
            }
            else if (spec.format.basetype == OIIO::TypeDesc::UINT8)
            {
                glinternalformat = GL_RGB8;
            }
            else if (spec.format.basetype == OIIO::TypeDesc::UINT16)
            {
                glinternalformat = GL_RGB16;
            }
            else if (use_float && spec.format.basetype == OIIO::TypeDesc::FLOAT)
            {
                glinternalformat = GL_RGB32F_ARB;
            }
            else if (use_float && spec.format.basetype == OIIO::TypeDesc::HALF)
            {
                glinternalformat = GL_RGB16F_ARB;
            }
        }
        else if (nchannels == 4)
        {
            glformat = GL_RGBA;
            if (use_srgb && issrgb)
            {
                if (spec.format.basetype == OIIO::TypeDesc::UINT8)
                {
                    glinternalformat = GL_SRGB8_ALPHA8;
                }
                else
                {
                    glinternalformat = GL_SRGB_ALPHA;
                }
            }
            else if (spec.format.basetype == OIIO::TypeDesc::UINT8)
            {
                glinternalformat = GL_RGBA8;
            }
            else if (spec.format.basetype == OIIO::TypeDesc::UINT16)
            {
                glinternalformat = GL_RGBA16;
            }
            else if (use_float && spec.format.basetype == OIIO::TypeDesc::FLOAT)
            {
                glinternalformat = GL_RGBA32F_ARB;
            }
            else if (use_float && spec.format.basetype == OIIO::TypeDesc::HALF)
            {
                glinternalformat = GL_RGBA16F_ARB;
            }
        }
        else
        {
            glformat = GL_INVALID_ENUM;
            glinternalformat = GL_INVALID_ENUM;
        }
    }

    void gl_rect_poly(float xmin, float ymin, float xmax, float ymax, float z /*= 0*/, float smin /*= 0*/, float tmin /*= 0*/, float smax /*= 1*/,
                      float tmax /*= 1*/, int rotate /*= 0*/)
    {
        glBegin(GL_POLYGON);
        float tex[] = { smin, tmin, smax, tmin, smax, tmax, smin, tmax };
        glTexCoord2f(tex[(0 + 2 * rotate) & 7], tex[(1 + 2 * rotate) & 7]);
        glVertex3f(xmin, ymin, z);
        glTexCoord2f(tex[(2 + 2 * rotate) & 7], tex[(3 + 2 * rotate) & 7]);
        glVertex3f(xmax, ymin, z);
        glTexCoord2f(tex[(4 + 2 * rotate) & 7], tex[(5 + 2 * rotate) & 7]);
        glVertex3f(xmax, ymax, z);
        glTexCoord2f(tex[(6 + 2 * rotate) & 7], tex[(7 + 2 * rotate) & 7]);
        glVertex3f(xmin, ymax, z);
        glEnd();
    }

    void gl_rect_lines(float xmin, float ymin, float xmax, float ymax, float z /*= 0*/, bool dotted /* = false*/)
    {
        if (dotted)
        {
            glPushAttrib(GL_ENABLE_BIT);
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x00FF);
        }
        glBegin(GL_LINES);
        glVertex3f(xmin, ymin, z);
        glVertex3f(xmax, ymin, z);

        glVertex3f(xmax, ymin, z);
        glVertex3f(xmax, ymax, z);

        glVertex3f(xmax, ymax, z);
        glVertex3f(xmin, ymax, z);

        glVertex3f(xmin, ymax, z);
        glVertex3f(xmin, ymin, z);
        glEnd();

        if (dotted)
        {
            glPopAttrib();
        }
    }

    void gl_draw_line(float xmin, float ymin, float xmax, float ymax, float z /*= 0*/)
    {
        glBegin(GL_LINES);
        glVertex3f(xmin, ymin, z);
        glVertex3f(xmax, ymax, z);
        glEnd();
    }

    std::string get_shader_source(const char *shader_path)
    {
        QResource r(shader_path);

        std::string result;
        result.resize(r.size() + 1);
        std::copy(r.data(), r.data() + r.size(), result.begin());
        result[r.size()] = '\0';
        return result;
    }

    void gl_shader_source(GLuint shader, const char *shader_path)
    {
        const auto data = get_shader_source(shader_path);
        const GLchar *data_ptr[] = { data.c_str() };
        glShaderSource(shader, 1, data_ptr, nullptr);
    }
}

OPENDCC_NAMESPACE_CLOSE
