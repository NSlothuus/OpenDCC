// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <GL/glew.h>

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QMenu>
#include <QtCore/QTimer>
#include <QtCore/QResource>

#include <OpenColorIO/OpenColorIO.h>

#include "gl_utils.hpp"
#include "app.h"

#include "gl_widget.h"
#include "gl_widget_tools.hpp"

OPENDCC_NAMESPACE_OPEN

namespace OCIO = OCIO_NAMESPACE;

const int LUT3D_EDGE_SIZE = 32;

const QVector<QString> RenderViewGLWidget::background_mode_names = { i18n("render_view.gl_widget.background_mode", "Checker"),
                                                                     i18n("render_view.gl_widget.background_mode", "Black"),
                                                                     i18n("render_view.gl_widget.background_mode", "Gray"),
                                                                     i18n("render_view.gl_widget.background_mode", "White") };

void print_link_status(GLuint shader_program)
{
    GLint status;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
    if (!status)
    {
        int log_len = 0;

        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &log_len);
        char logbuf[2048];
        glGetProgramInfoLog(shader_program, 2048 * sizeof(char), NULL, logbuf);
        printf("link error:%s\n", logbuf);
    }
}

RenderViewGLWidget::RenderViewGLWidget(QWidget* parent, RenderViewMainWindow* app)
    : QOpenGLWidget(parent)
{
    m_app = app;

    m_centerx = 0;
    m_centery = 0;
    m_zoom = 1.0f;

    m_current_tool = nullptr;

    m_mouse_image_x = 0;
    m_mouse_image_y = 0;

    m_mouse_image_color[0] = 0.0f;
    m_mouse_image_color[1] = 0.0f;
    m_mouse_image_color[2] = 0.0f;
    m_mouse_image_color[3] = 0.0f;

    m_texture_format = GL_BYTE;
    m_texture_nchannels = 3;
    m_texture_data_stride = sizeof(unsigned char);
    m_spec.width = 1;
    m_spec.height = 1;
    m_spec.full_width = 1;
    m_spec.full_height = 1;

    setMouseTracking(true);
    setAutoFillBackground(false);
    m_popup_menu = nullptr;

    m_use_shaders = false;
    m_use_float = false;
    m_use_halffloat = false;
    m_use_srgb = false;

    m_timer = new QTimer();
    m_timer->setInterval(1);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

void RenderViewGLWidget::initialize_glew()
{
    GLenum glew_error = glewInit();
    if (glew_error != GLEW_OK)
    {
        fprintf(stderr, "GLEW Init Error %d\n", glew_error);
    }

    m_use_shaders = glewIsSupported("GL_VERSION_2_0");

    if (!m_use_shaders && glewIsSupported("GL_ARB_shader_objects "
                                          "GL_ARB_vertex_shader "
                                          "GL_ARB_fragment_shader"))
    {
        m_use_shaders = true;
        m_shaders_using_extensions = true;
    }

    m_use_srgb = glewIsSupported("GL_VERSION_2_1") || glewIsSupported("GL_EXT_texture_sRGB");

    m_use_halffloat = glewIsSupported("GL_VERSION_3_0") || glewIsSupported("GL_ARB_half_float_pixel") || glewIsSupported("GL_NV_half_float_pixel");

    m_use_float = glewIsSupported("GL_VERSION_3_0") || glewIsSupported("GL_ARB_texture_float") || glewIsSupported("GL_ATI_texture_float");
}

void RenderViewGLWidget::initializeGL()
{
    initialize_glew();

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_3D);

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    // Make sure initial matrix is identity (returning to this stack level loads
    // back this matrix).
    glLoadIdentity();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDisable(GL_DEPTH_TEST);

    glGenTextures(1, &m_image_texture);
    glGenTextures(1, &m_background_texture.texture);
    // Allocate LUT3D

    glGenTextures(1, &m_lut_texture);

    int num3Dentries = 3 * LUT3D_EDGE_SIZE * LUT3D_EDGE_SIZE * LUT3D_EDGE_SIZE;
    float* lut_data = new float[num3Dentries];
    memset(lut_data, 0, sizeof(float) * num3Dentries);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, m_lut_texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F_ARB, LUT3D_EDGE_SIZE, LUT3D_EDGE_SIZE, LUT3D_EDGE_SIZE, 0, GL_RGB, GL_FLOAT, lut_data);

    delete[] lut_data;
    GLint status;
    m_texture_shader_program = 0;
    m_texture_vertex_shader = 0;
    m_texture_fragment_shader = 0;

    m_texture_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    gl_utils::gl_shader_source(m_texture_vertex_shader, ":/shaders/image.vert");
    glCompileShader(m_texture_vertex_shader);

    update_lut();

    GLint background_vertex_shader = 0;
    GLint background_fragment_shader = 0;

    m_background_shader_program = 0;
    m_background_shader_program = glCreateProgram();
    background_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    gl_utils::gl_shader_source(background_vertex_shader, ":/shaders/background.vert");
    glCompileShader(background_vertex_shader);
    glGetShaderiv(background_vertex_shader, GL_COMPILE_STATUS, &status);
    glAttachShader(m_background_shader_program, background_vertex_shader);

    background_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_utils::gl_shader_source(background_fragment_shader, ":/shaders/background.frag");
    glCompileShader(background_fragment_shader);
    glAttachShader(m_background_shader_program, background_fragment_shader);

    glLinkProgram(m_background_shader_program);
    print_link_status(m_background_shader_program);

    init_lines_shader();
    RenderViewHUD::init_gl();

    m_timer->start();
}

void RenderViewGLWidget::init_lines_shader()
{
    GLint status;
    m_lines_shader_program = 0;
    m_lines_shader_program = glCreateProgram();
    m_lines_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    gl_utils::gl_shader_source(m_lines_vertex_shader, ":/shaders/lines.vert");
    glCompileShader(m_lines_vertex_shader);

    glGetShaderiv(m_lines_vertex_shader, GL_COMPILE_STATUS, &status);
    glAttachShader(m_lines_shader_program, m_lines_vertex_shader);

    m_lines_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    gl_utils::gl_shader_source(m_lines_fragment_shader, ":/shaders/lines.frag");
    glCompileShader(m_lines_fragment_shader);
    glGetShaderiv(m_lines_fragment_shader, GL_COMPILE_STATUS, &status);
    glAttachShader(m_lines_shader_program, m_lines_fragment_shader);

    glLinkProgram(m_lines_shader_program);

    print_link_status(m_lines_shader_program);
}

void RenderViewGLWidget::update_lut()
{
    OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();
    std::string s;
#if OCIO_VERSION_HEX < 0x02000000
    auto transform = m_app->get_color_transform();

    OCIO::ConstProcessorRcPtr processor;
    processor = config->getProcessor(transform);

    OCIO::GpuShaderDesc shaderDesc;
    shaderDesc.setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_0);
    shaderDesc.setFunctionName("OCIODisplay");
    shaderDesc.setLut3DEdgeLen(LUT3D_EDGE_SIZE);

    std::string lut_cache_id = processor->getGpuLut3DCacheID(shaderDesc);
    if (m_lut_cache_id != lut_cache_id)
    {
        int num3Dentries = 3 * LUT3D_EDGE_SIZE * LUT3D_EDGE_SIZE * LUT3D_EDGE_SIZE;
        float* lut_data = new float[num3Dentries];
        memset(lut_data, 0, sizeof(float) * num3Dentries);

        processor->getGpuLut3D(lut_data, shaderDesc);

        // glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, m_lut_texture);
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, LUT3D_EDGE_SIZE, LUT3D_EDGE_SIZE, LUT3D_EDGE_SIZE, GL_RGB, GL_FLOAT, lut_data);
        delete lut_data;

        m_lut_cache_id = lut_cache_id;
    }

    s += processor->getGpuShaderText(shaderDesc);
#else
    auto vpt = m_app->get_viewing_pipeline();
    OCIO::ConstProcessorRcPtr processor;
    try
    {
        processor = vpt->getProcessor(config);
    }
    catch (OCIO::Exception& e)
    {
        printf("%s", e.what());
        return;
    }
#if OCIO_VERSION_MINOR >= 1
    auto desc = OCIO::GpuShaderDesc::CreateShaderDesc();
    auto gpu = processor->getOptimizedLegacyGPUProcessor(OCIO::OPTIMIZATION_DEFAULT, LUT3D_EDGE_SIZE);
#else
    auto desc = OCIO::GpuShaderDesc::CreateLegacyShaderDesc(LUT3D_EDGE_SIZE);
    auto gpu = processor->getDefaultGPUProcessor();
#endif
    desc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_2);
    desc->setFunctionName("OCIODisplay");
    gpu->extractGpuShaderInfo(desc);
    if (desc->getCacheID() == m_lut_cache_id)
        return;
    else
        m_lut_cache_id = desc->getCacheID();
    if (desc->getNum3DTextures() == 1)
    {
        const char* texture_name = nullptr;
        const char* sampler_name = nullptr;
        unsigned edgelen = 0;
        OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
        desc->get3DTexture(0, texture_name, sampler_name, edgelen, interpolation);
        if (!texture_name || !*texture_name || !sampler_name || !*sampler_name || edgelen == 0)
            return;

        const float* values = nullptr;
        desc->get3DTextureValues(0, values);
        if (!values)
            return;
        glBindTexture(GL_TEXTURE_3D, m_lut_texture);
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, LUT3D_EDGE_SIZE, LUT3D_EDGE_SIZE, LUT3D_EDGE_SIZE, GL_RGB, GL_FLOAT, values);
    }
    s += desc->getShaderText();
#endif
    s += "\n";
    s += "#define OCIO_VERSION_MAJOR " + std::to_string(OCIO_VERSION_HEX >> 24) + '\n';
    s += gl_utils::get_shader_source(":/shaders/image.frag");

    if (m_texture_fragment_shader)
        glDeleteShader(m_texture_fragment_shader);
    if (m_texture_shader_program)
        glDeleteProgram(m_texture_shader_program);

    m_texture_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* frag_src = s.c_str();
    glShaderSource(m_texture_fragment_shader, 1, (const GLchar**)&frag_src, NULL);
    glCompileShader(m_texture_fragment_shader);

    m_texture_shader_program = glCreateProgram();
    glAttachShader(m_texture_shader_program, m_texture_vertex_shader);
    glAttachShader(m_texture_shader_program, m_texture_fragment_shader);
    glLinkProgram(m_texture_shader_program);
    print_link_status(m_texture_shader_program);
}

void RenderViewGLWidget::update_image_region(const int image_id, const ROI& region, std::shared_ptr<const std::vector<char>> bucket_data)
{
    // assume image already updated just refresh
    if (image_id != m_app->get_current_image_id())
        return;

    auto image = m_app->get_current_image();

    if (!image || !isValid())
        return;

    Bucket bucket;
    bucket.image_id = image_id;
    bucket.region = region;
    bucket.data = bucket_data;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_input_buckets.push(bucket);
    }

    update();
}

void RenderViewGLWidget::update_image()
{
    auto image = m_app->get_current_image();

    if (!image)
        return;

    auto upload_image = [&](OIIO::ImageBuf* image, bool background_texture) {
        const OIIO::ImageSpec& spec(image->spec());
        int nchannels = image->nchannels();

        GLenum gltype = GL_UNSIGNED_BYTE;
        GLenum glformat = GL_RGB;
        GLenum glinternalformat = GL_RGB;
        gl_utils::typespec_to_opengl(spec, nchannels, gltype, glformat, glinternalformat, m_use_halffloat, m_use_srgb, m_use_float);

        if (!background_texture)
        {
            m_spec = spec;
            glBindTexture(GL_TEXTURE_2D, m_image_texture);
            m_texture_format = gltype;
            m_texture_data_stride = spec.channel_bytes();
            m_texture_nchannels = nchannels;
        }
        else
        {
            m_background_texture.spec = spec;
            glBindTexture(GL_TEXTURE_2D, m_background_texture.texture);
            m_background_texture.texture_format = gltype;
            m_background_texture.texture_data_stride = spec.channel_bytes();
            m_background_texture.texture_nchannels = nchannels;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        const size_t image_size = spec.width * spec.height * nchannels * spec.channel_bytes();

        std::vector<uint8_t> texture_data(image_size);
        image->get_pixels(image->roi(), spec.format, texture_data.data());
        glTexImage2D(GL_TEXTURE_2D, 0, glinternalformat, spec.width, spec.height, 0, glformat, gltype, texture_data.data());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    };

    upload_image(image, false);

    if (m_app->is_toggle_background_mode())
    {
        auto back_image = m_app->get_background_image();

        if (back_image)
        {
            upload_image(back_image, true);
        }
    }

    m_hud.clear();
    m_hud.add_text(m_spec.full_width + 4, m_spec.full_height,
                   "(" + std::to_string(m_spec.full_width) + "x" + std::to_string(m_spec.full_height) + ")");
    if (image->spec().roi() != m_spec.roi_full())
    {

        m_hud.add_text(m_spec.x + m_spec.width, m_spec.y,
                       std::to_string(m_spec.x + m_spec.width + 1) + ", " + std::to_string(m_spec.full_height - m_spec.y + 1));
        m_hud.add_text(m_spec.x, m_spec.y + m_spec.height,
                       std::to_string(m_spec.x - 1) + ", " + std::to_string(m_spec.full_height - m_spec.y - m_spec.height - 1));
    }
}

void RenderViewGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, w, h, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
}

void RenderViewGLWidget::widget_to_image_pos(float widget_x, float widget_y, float& image_x, float& image_y)
{
    image_x = (widget_x - width() * 0.5) / m_zoom + width() * 0.5 + m_centerx;
    image_y = (widget_y - height() * 0.5) / m_zoom + height() * 0.5 + m_centery;
}

void RenderViewGLWidget::image_to_widget_pos(float image_x, float image_y, float& widget_x, float& widget_y)
{
    widget_x = image_x * m_zoom + width() * 0.5 * (1 - m_zoom) - m_centerx * m_zoom;
    widget_y = image_y * m_zoom + height() * 0.5 * (1 - m_zoom) - m_centery * m_zoom;
}

void RenderViewGLWidget::get_focus_image_pixel_color(const int x, const int y, float& r, float& g, float& b, float& a)
{
    auto img = m_app->get_current_image();
    if (x > 0 && y > 0 && x <= m_spec.full_width && y <= m_spec.full_height && img)
    {

        float* channel_data[] = { &r, &g, &b, &a };
        float pixel[4];
        img->getpixel(x, y, 0, &pixel[0], 4);

        // int it_channels = std::min(3, m_texture_nchannels);
        for (int i = 0; i < 4; i++)
        {
            *(channel_data[i]) = pixel[i];
        }
    }
    else
    {
        r = 0.0f;
        g = 0.0f;
        b = 0.0f;
        a = 0.0f;
    }
}

void RenderViewGLWidget::wheelEvent(QWheelEvent* event)
{

    m_current_tool->wheel_event(event);
    m_app->update_titlebar();
}

void RenderViewGLWidget::mousePressEvent(QMouseEvent* event)
{
    m_current_tool->mouse_press(event);
}

void RenderViewGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    m_current_tool->mouse_release(event);
}

void RenderViewGLWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_popup_menu)
    {
        m_popup_menu = new QMenu;
        m_app->create_menus(m_popup_menu);
    }
    m_popup_menu->exec(event->globalPos());
}

void RenderViewGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    m_current_tool->mouse_move(event);

    if (!is_lock_pixel_readout)
    {
        float image_x, image_y;
        widget_to_image_pos(m_mousex, m_mousey, image_x, image_y);
        m_mouse_image_x = (int)(image_x);
        m_mouse_image_y = (int)(image_y);
    }
    update_pixel_info();
}

void RenderViewGLWidget::update_pixel_info()
{
    float r, g, b, a;
    get_focus_image_pixel_color(m_mouse_image_x, m_mouse_image_y, r, g, b, a);
    m_mouse_image_color[0] = r;
    m_mouse_image_color[1] = g;
    m_mouse_image_color[2] = b;
    m_mouse_image_color[3] = a;

    m_app->update_pixel_info();
}

void RenderViewGLWidget::lock_pixel_readout()
{
    is_lock_pixel_readout = !is_lock_pixel_readout;
    float image_x, image_y;
    widget_to_image_pos(m_mousex, m_mousey, image_x, image_y);
    m_mouse_image_x = (int)(image_x);
    m_mouse_image_y = (int)(image_y);
}

void RenderViewGLWidget::load_input_buckets()
{
    Bucket bucket;
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_input_buckets.empty())
                break;

            bucket = m_input_buckets.front();
            m_input_buckets.pop();
        }

        if (bucket.image_id == m_app->get_current_image_id())
        {
            auto image = m_app->get_current_image();

            if (image)
            {
                const OIIO::ImageSpec& spec(image->spec());

                int nchannels = image->nchannels();

                GLenum gltype = GL_UNSIGNED_BYTE;
                GLenum glformat = GL_RGB;
                GLenum glinternalformat = GL_RGB;

                gl_utils::typespec_to_opengl(spec, nchannels, gltype, glformat, glinternalformat, m_use_halffloat, m_use_srgb, m_use_float);

                glBindTexture(GL_TEXTURE_2D, m_image_texture);

                glTexSubImage2D(GL_TEXTURE_2D, 0, bucket.region.xstart, bucket.region.ystart, bucket.region.xend - bucket.region.xstart,
                                bucket.region.yend - bucket.region.ystart, glformat, gltype, bucket.data->data());
            }
        }
    }
}

void RenderViewGLWidget::paintGL()
{
    load_input_buckets();

    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    {
        glUseProgram(m_background_shader_program);
        GLint loc;
        loc = glGetUniformLocation(m_background_shader_program, "width");
        glUniform1i(loc, width());
        loc = glGetUniformLocation(m_background_shader_program, "height");
        glUniform1i(loc, height());
        loc = glGetUniformLocation(m_background_shader_program, "mode");
        glUniform1i(loc, background_mode_idx);

        gl_utils::gl_rect_poly(-1, -1, 1, 1);
    }
    glPopMatrix();
    glPushMatrix();

    const bool background_mode = m_app->is_toggle_background_mode() && m_app->get_current_image_id() != m_app->get_background_image_id();

    if (background_mode)
    {
        glUseProgram(m_texture_shader_program);

        GLint loc;
        loc = glGetUniformLocation(m_texture_shader_program, "startchannel");
        glUniform1i(loc, m_app->current_channel());

        loc = glGetUniformLocation(m_texture_shader_program, "imgtex");
        glUniform1i(loc, 2);
#if OCIO_VERSION_HEX < 0x02000000
        loc = glGetUniformLocation(m_texture_shader_program, "lut3d");
#else
        loc = glGetUniformLocation(m_texture_shader_program, "ocio_lut3d_0Sampler");
#endif
        glUniform1i(loc, 1);

        loc = glGetUniformLocation(m_texture_shader_program, "colormode");
        glUniform1i(loc, static_cast<int>(m_app->current_color_mode()));

        loc = glGetUniformLocation(m_texture_shader_program, "imgchannels");
        glUniform1i(loc, m_background_texture.texture_nchannels);

        loc = glGetUniformLocation(m_texture_shader_program, "transparent");
        glUniform1i(loc, 0);

        float x, y, x2, y2;
        image_to_widget_pos(m_background_texture.spec.x, m_background_texture.spec.y, x, y);
        image_to_widget_pos(m_background_texture.spec.x + m_background_texture.spec.width,
                            m_background_texture.spec.y + m_background_texture.spec.height, x2, y2);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_background_texture.texture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, m_lut_texture);
        gl_utils::gl_rect_poly(x, y, x2, y2);
    }

    if (m_app->get_current_image())
    {
        glUseProgram(m_texture_shader_program);

        GLint loc;
        loc = glGetUniformLocation(m_texture_shader_program, "startchannel");
        glUniform1i(loc, m_app->current_channel());

        loc = glGetUniformLocation(m_texture_shader_program, "imgtex");
        glUniform1i(loc, 0);
#if OCIO_VERSION_HEX < 0x02000000
        loc = glGetUniformLocation(m_texture_shader_program, "lut3d");
#else
        loc = glGetUniformLocation(m_texture_shader_program, "ocio_lut3d_0Sampler");
#endif
        glUniform1i(loc, 1);

        loc = glGetUniformLocation(m_texture_shader_program, "colormode");
        glUniform1i(loc, static_cast<int>(m_app->current_color_mode()));

        // we want transparent surface only in toggle background mode
        loc = glGetUniformLocation(m_texture_shader_program, "transparent");
        glUniform1i(loc, background_mode ? 1 : 0);

        loc = glGetUniformLocation(m_texture_shader_program, "imgchannels");
        glUniform1i(loc, m_texture_nchannels);

        float x, y, x2, y2;
        image_to_widget_pos(m_spec.x, m_spec.y, x, y);
        image_to_widget_pos(m_spec.x + m_spec.width, m_spec.y + m_spec.height, x2, y2);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_image_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, m_lut_texture);
        gl_utils::gl_rect_poly(x, y, x2, y2);

        glUseProgram(m_lines_shader_program);
        loc = glGetUniformLocation(m_lines_shader_program, "color");
        glUniform3f(loc, 1, 0, 0);
        if (is_lock_pixel_readout)
        {
            float pixel_x, pixel_y;
            image_to_widget_pos(m_mouse_image_x + 0.5, m_mouse_image_y + 0.5, pixel_x, pixel_y);

            gl_utils::gl_draw_line(pixel_x, 0, pixel_x, height());
            gl_utils::gl_draw_line(0, pixel_y + 0.5, width(), pixel_y);
        }

        if (m_display_crop)
        {
            float xstart, ystart, xend, yend;

            if (m_crop_region.xstart != m_crop_region.xend && m_crop_region.ystart != m_crop_region.yend)
            {
                image_to_widget_pos(m_crop_region.xstart, m_crop_region.ystart, xstart, ystart);
                image_to_widget_pos(m_crop_region.xend + 1, m_crop_region.yend + 1, xend, yend);

                gl_utils::gl_rect_lines(xstart, ystart, xend, yend);
            }
        }

        if (show_resolution_guides)
        {
            loc = glGetUniformLocation(m_lines_shader_program, "color");
            glUniform3f(loc, 0.7, 0.7, 0.7);

            float xstart, ystart, xend, yend;
            if (m_spec.roi() != m_spec.roi_full())
            {
                image_to_widget_pos(m_spec.x, m_spec.y, xstart, ystart);
                image_to_widget_pos(m_spec.x + m_spec.width, m_spec.y + m_spec.height, xend, yend);
                gl_utils::gl_rect_lines(xstart, ystart, xend, yend, 0, true);
            }

            float xstart_full, ystart_full, xend_full, yend_full;
            image_to_widget_pos(0, 0, xstart_full, ystart_full);
            image_to_widget_pos(m_spec.full_width, m_spec.full_height, xend_full, yend_full);

            gl_utils::gl_rect_lines(xstart_full, ystart_full, xend_full, yend_full);

            m_hud.draw([this](float image_x, float image_y, float& widget_x, float& widget_y) {
                image_to_widget_pos(image_x, image_y, widget_x, widget_y);
            });
        }
    }

    auto gl_error = glGetError();
    if (gl_error != GL_NO_ERROR)
        printf("error in openGL: %d", gl_error);
    glPopMatrix();
}

void RenderViewGLWidget::set_crop_region(const ROI& region)
{
    m_crop_region = region;
}

void RenderViewGLWidget::set_crop_display(const bool show)
{
    m_display_crop = show;
}

void RenderViewGLWidget::update_mouse_pos(QMouseEvent* event)
{
    QPoint point = event->pos();
    m_mousex = point.x();
    m_mousey = point.y();
}

void RenderViewGLWidget::set_current_tool(RenderViewGLWidgetTool* tool)
{
    m_current_tool = tool;
}

RenderViewGLWidget::ROI RenderViewGLWidget::get_crop_region()
{
    return m_crop_region;
}

OPENDCC_NAMESPACE_CLOSE
