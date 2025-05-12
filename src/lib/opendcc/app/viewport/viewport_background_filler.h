/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/app/core/settings.h"
#include "opendcc/opendcc.h"
#include <array>

OPENDCC_NAMESPACE_OPEN

class ViewportGLWidget;

class ViewportBackgroundFiller
{
public:
    ViewportBackgroundFiller(ViewportGLWidget* widget)
        : m_gl_widget(widget)
    {
    }
    virtual void draw() = 0;
    virtual ~ViewportBackgroundFiller() = default;

protected:
    ViewportGLWidget* m_gl_widget = nullptr;
};

class SolidBackgroundFiller : public ViewportBackgroundFiller
{
public:
    SolidBackgroundFiller(ViewportGLWidget* widget);

    ~SolidBackgroundFiller();

    virtual void draw() override;
    void update(const PXR_NS::GfVec3f& color);

private:
    PXR_NS::GfVec3f m_color;
    Settings::SettingChangedHandle m_cid;
    static const std::string s_background_color_key;
};

class GradientBackgroundFiller : public ViewportBackgroundFiller
{
public:
    GradientBackgroundFiller(ViewportGLWidget* widget);
    ~GradientBackgroundFiller();

    virtual void draw() override;

private:
    std::array<PXR_NS::GfVec3f, 2> m_gradient_colors;
    Settings::SettingChangedHandle m_gradient_top_cid;
    Settings::SettingChangedHandle m_gradient_bottom_cid;
    static const std::string s_gradient_top_key;
    static const std::string s_gradient_bottom_key;

    uint32_t m_shader_id = -1;
    uint32_t m_vao = -1;
    uint32_t m_vbo = -1;
    int32_t m_gradient_colors_location = -1;
};

OPENDCC_NAMESPACE_CLOSE
