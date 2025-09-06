/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <queue>
#include <QtOpenGL/QOpenGLWidget>
#include <OpenImageIO/imagebuf.h>
#include "HUD.hpp"

class QMenu;
class QTimer;

OPENDCC_NAMESPACE_OPEN

class RenderViewMainWindow;
class RenderViewGLWidgetTool;

class RenderViewGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    RenderViewGLWidget(QWidget* parent, RenderViewMainWindow* app);
    ~RenderViewGLWidget() {};

    void initialize_glew();
    void initializeGL() override;

    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

    struct ROI
    {
        int xstart;
        int ystart;
        int xend;
        int yend;
    };

    // mouse pixel info position and color, it can be locked using pixel readout
    int m_mouse_image_x, m_mouse_image_y;
    float m_mouse_image_color[4];

    // viewport position and mouse tracking, should be modified by tools
    int m_mousex, m_mousey;
    float m_zoom;
    float m_centerx;
    float m_centery;

    void update_image();
    void update_lut();
    void update_image_region(const int image_id, const ROI& region, std::shared_ptr<const std::vector<char>> bucket_data);

    void set_crop_region(const ROI& region);
    void set_crop_display(const bool show);
    void update_mouse_pos(QMouseEvent* event);
    void widget_to_image_pos(float widget_x, float widget_y, float& image_x, float& image_y);
    void set_current_tool(RenderViewGLWidgetTool* tool);

    ROI get_crop_region();

    inline bool is_crop_displayed() { return m_display_crop; }

    const static QVector<QString> background_mode_names;
    int background_mode_idx = 0;
    bool show_resolution_guides = false;

public Q_SLOTS:
    void update_pixel_info();
    void lock_pixel_readout();

private:
    void init_lines_shader();
    void get_focus_image_pixel_color(const int x, const int y, float& r, float& g, float& b, float& a);
    void load_input_buckets();

    void image_to_widget_pos(float image_x, float image_y, float& widget_x, float& widget_y);

    struct GLTexture
    {
        GLuint texture;
        GLenum texture_format;
        size_t texture_data_stride;
        unsigned int texture_nchannels;
        OIIO::ImageSpec spec;
    };

    struct Bucket
    {
        int image_id;
        RenderViewGLWidget::ROI region;
        std::shared_ptr<const std::vector<char>> data;
    };
    std::mutex m_mutex;
    RenderViewHUD m_hud;
    std::queue<Bucket> m_input_buckets;
    GLuint m_image_texture;
    GLuint m_lut_texture;
    GLenum m_texture_format;
    GLTexture m_background_texture;
    GLint m_texture_fragment_shader;
    GLint m_texture_vertex_shader;

    GLint m_lines_vertex_shader;
    GLint m_lines_fragment_shader;

    size_t m_texture_data_stride;
    int m_texture_nchannels;
    OIIO::ImageSpec m_spec;
    GLuint m_texture_shader_program;
    GLuint m_background_shader_program;

    GLuint m_lines_shader_program;

    bool m_use_shaders, m_use_srgb, m_use_float, m_use_halffloat, m_shaders_using_extensions;

    bool is_lock_pixel_readout = false;
    QTimer* m_timer;
    std::string m_lut_cache_id;
    QMenu* m_popup_menu;
    RenderViewMainWindow* m_app;

    bool m_display_crop;
    ROI m_crop_region = ROI { 0, 0, 0, 0 };

    RenderViewGLWidgetTool* m_current_tool;
};

OPENDCC_NAMESPACE_CLOSE
