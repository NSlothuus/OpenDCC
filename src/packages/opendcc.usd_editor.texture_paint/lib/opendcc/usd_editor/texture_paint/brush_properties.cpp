// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/texture_paint/brush_properties.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/settings.h"
#include "opendcc/usd_editor/texture_paint/texture_paint_tool_context.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

BrushProperties::BrushProperties()
{
    auto settings = Application::instance().get_settings();
    m_radius = settings->get(TexturePaintToolContext::settings_prefix() + ".radius", 20);
    m_color = settings->get(TexturePaintToolContext::settings_prefix() + ".color", GfVec3f(1, 1, 1));
    auto falloff_cvs = settings->get(TexturePaintToolContext::settings_prefix() + ".falloff_curve", std::vector<double>());
    using CurveRamp = Ramp<float>;
    m_falloff_curve = std::make_shared<CurveRamp>();
    for (int i = 0; i < falloff_cvs.size(); i += 3)
    {
        int interp_type = falloff_cvs[i + 2];
        m_falloff_curve->add_point(falloff_cvs[i], static_cast<float>(falloff_cvs[i + 1]), static_cast<CurveRamp::InterpType>(interp_type));
    }
    if (m_falloff_curve->cv().size() == 2)
    {
        m_falloff_curve->add_point(0, 1, CurveRamp::InterpType::kSmooth);
        m_falloff_curve->add_point(1, 0, CurveRamp::InterpType::kSmooth);
    }
    m_falloff_curve->prepare_points();
}

void BrushProperties::update_falloff_curve()
{
    std::vector<double> CV_data;
    CV_data.reserve((m_falloff_curve->cv().size() - 2) * 3);
    for (int i = 1; i < m_falloff_curve->cv().size() - 1; i++)
    {
        const auto& cv = m_falloff_curve->cv()[i];
        CV_data.push_back(cv.position);
        CV_data.push_back(cv.value);
        CV_data.push_back(cv.interp_type);
    }
    Application::instance().get_settings()->set(TexturePaintToolContext::settings_prefix() + "falloff_curve", CV_data);
}

std::shared_ptr<Ramp<float>> BrushProperties::get_falloff_curve()
{
    return m_falloff_curve;
}

const GfVec3f& BrushProperties::get_color() const
{
    return m_color;
}

void BrushProperties::set_color(const GfVec3f& color)
{
    if (m_color == color)
        return;
    m_color[0] = GfClamp(color[0], 0, 1);
    m_color[1] = GfClamp(color[1], 0, 1);
    m_color[2] = GfClamp(color[2], 0, 1);

    Application::instance().get_settings()->set(TexturePaintToolContext::settings_prefix() + ".color", m_color);
}

int BrushProperties::get_radius() const
{
    return m_radius;
}

void BrushProperties::set_radius(int radius)
{
    if (m_radius == radius)
        return;
    m_radius = std::min(std::max(radius, 1), 500);
    Application::instance().get_settings()->set(TexturePaintToolContext::settings_prefix() + ".radius", m_radius);
}

OPENDCC_NAMESPACE_CLOSE
