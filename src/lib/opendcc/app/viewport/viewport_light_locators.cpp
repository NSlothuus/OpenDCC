// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_light_locators.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

void RectLightLocatorRenderData::update(const std::unordered_map<std::string, VtValue>& data)
{
    auto w_iter = data.find("width");
    auto h_iter = data.find("height");
    if (w_iter != data.end() && h_iter != data.end())
    {
        w = w_iter->second.Get<float>() / 2.f;
        h = h_iter->second.Get<float>() / 2.f;
    }
    update_points();
}

const PXR_NS::VtArray<int>& RectLightLocatorRenderData::vertex_per_curve() const
{
    static VtArray<int> vcount { 2, 2, 2, 2, 2, 2, 2 };
    return vcount;
}

const PXR_NS::VtArray<int>& RectLightLocatorRenderData::vertex_indexes() const
{
    static VtArray<int> indexes { 0, 1, 0, 2, 2, 3, 1, 3, 4, 5, 1, 2, 0, 3 };
    return indexes;
}

const PXR_NS::VtVec3fArray& RectLightLocatorRenderData::vertex_positions() const
{
    return m_points;
}

RectLightLocatorRenderData::RectLightLocatorRenderData()
{
    update_points();
}

void RectLightLocatorRenderData::update_points()
{
    m_points = VtVec3fArray { { -w, +h, +0 }, { +w, +h, +0 }, { -w, -h, +0 }, { +w, -h, +0 }, { +0, +0, +0 }, { +0, +0, -std::min(w, h) } };
    m_bbox = { { -w / 2.f, -h / 2.f, -2 }, { w / 2.f, h / 2.f, 2 } };
}

const PXR_NS::GfRange3d& RectLightLocatorRenderData::bbox() const
{
    return m_bbox;
}

const PXR_NS::TfToken& RectLightLocatorRenderData::topology() const
{
    return HdTokens->segmented;
}

DomeLightLocatorRenderData::DomeLightLocatorRenderData() {}

void DomeLightLocatorRenderData::update(const std::unordered_map<std::string, VtValue>& data)
{
    auto texture_path = data.find("texture_path");
    m_is_double_sided = texture_path != data.end() && !texture_path->second.Get<std::string>().empty();
}

const PXR_NS::VtArray<int>& DomeLightLocatorRenderData::vertex_per_curve() const
{
    static VtArray<int> vpc = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                                4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                                4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };
    return vpc;
}

const PXR_NS::VtArray<int>& DomeLightLocatorRenderData::vertex_indexes() const
{
    static VtArray<int> indexes = {
        0,  1,  11, 10, 1,  2,  12, 11, 2,  3,  13, 12, 3,  4,  14, 13, 4,  5,  15, 14, 5,  6,  16, 15, 6,  7,  17, 16, 7,  8,  18, 17, 8,  9,  19,
        18, 9,  0,  10, 19, 10, 11, 21, 20, 11, 12, 22, 21, 12, 13, 23, 22, 13, 14, 24, 23, 14, 15, 25, 24, 15, 16, 26, 25, 16, 17, 27, 26, 17, 18,
        28, 27, 18, 19, 29, 28, 19, 10, 20, 29, 20, 21, 31, 30, 21, 22, 32, 31, 22, 23, 33, 32, 23, 24, 34, 33, 24, 25, 35, 34, 25, 26, 36, 35, 26,
        27, 37, 36, 27, 28, 38, 37, 28, 29, 39, 38, 29, 20, 30, 39, 30, 31, 41, 40, 31, 32, 42, 41, 32, 33, 43, 42, 33, 34, 44, 43, 34, 35, 45, 44,
        35, 36, 46, 45, 36, 37, 47, 46, 37, 38, 48, 47, 38, 39, 49, 48, 39, 30, 40, 49, 40, 41, 51, 50, 41, 42, 52, 51, 42, 43, 53, 52, 43, 44, 54,
        53, 44, 45, 55, 54, 45, 46, 56, 55, 46, 47, 57, 56, 47, 48, 58, 57, 48, 49, 59, 58, 49, 40, 50, 59, 50, 51, 61, 60, 51, 52, 62, 61, 52, 53,
        63, 62, 53, 54, 64, 63, 54, 55, 65, 64, 55, 56, 66, 65, 56, 57, 67, 66, 57, 58, 68, 67, 58, 59, 69, 68, 59, 50, 60, 69, 60, 61, 71, 70, 61,
        62, 72, 71, 62, 63, 73, 72, 63, 64, 74, 73, 64, 65, 75, 74, 65, 66, 76, 75, 66, 67, 77, 76, 67, 68, 78, 77, 68, 69, 79, 78, 69, 60, 70, 79,
        70, 71, 81, 80, 71, 72, 82, 81, 72, 73, 83, 82, 73, 74, 84, 83, 74, 75, 85, 84, 75, 76, 86, 85, 76, 77, 87, 86, 77, 78, 88, 87, 78, 79, 89,
        88, 79, 70, 80, 89, 1,  0,  90, 2,  1,  90, 3,  2,  90, 4,  3,  90, 5,  4,  90, 6,  5,  90, 7,  6,  90, 8,  7,  90, 9,  8,  90, 0,  9,  90,
        80, 81, 91, 81, 82, 91, 82, 83, 91, 83, 84, 91, 84, 85, 91, 85, 86, 91, 86, 87, 91, 87, 88, 91, 88, 89, 91, 89, 80, 91
    };
    return indexes;
}

const PXR_NS::VtVec3fArray& DomeLightLocatorRenderData::vertex_positions() const
{
    static const float scale = 1000.f;
    static GfVec3f points[] = { scale * GfVec3f(0.25000003, -0.95105654, -0.1816357),
                                scale * GfVec3f(0.09549149, -0.95105654, -0.2938927),
                                scale * GfVec3f(-0.09549155, -0.95105654, -0.29389265),
                                scale * GfVec3f(-0.25000006, -0.95105654, -0.18163563),
                                scale * GfVec3f(-0.30901703, -0.95105654, 1.8418849e-8),
                                scale * GfVec3f(-0.25000003, -0.95105654, 0.18163568),
                                scale * GfVec3f(-0.0954915, -0.95105654, 0.29389265),
                                scale * GfVec3f(0.09549151, -0.95105654, 0.29389265),
                                scale * GfVec3f(0.25, -0.95105654, 0.18163563),
                                scale * GfVec3f(0.309017, -0.95105654, 0),
                                scale * GfVec3f(0.4755283, -0.809017, -0.3454916),
                                scale * GfVec3f(0.1816356, -0.809017, -0.5590171),
                                scale * GfVec3f(-0.18163572, -0.809017, -0.55901706),
                                scale * GfVec3f(-0.47552836, -0.809017, -0.3454915),
                                scale * GfVec3f(-0.5877853, -0.809017, 3.503473e-8),
                                scale * GfVec3f(-0.4755283, -0.809017, 0.34549156),
                                scale * GfVec3f(-0.18163562, -0.809017, 0.55901706),
                                scale * GfVec3f(0.18163565, -0.809017, 0.559017),
                                scale * GfVec3f(0.47552827, -0.809017, 0.3454915),
                                scale * GfVec3f(0.58778524, -0.809017, 0),
                                scale * GfVec3f(0.65450853, -0.58778524, -0.4755284),
                                scale * GfVec3f(0.24999996, -0.58778524, -0.76942104),
                                scale * GfVec3f(-0.25000012, -0.58778524, -0.769421),
                                scale * GfVec3f(-0.65450865, -0.58778524, -0.47552827),
                                scale * GfVec3f(-0.8090171, -0.58778524, 4.822117e-8),
                                scale * GfVec3f(-0.65450853, -0.58778524, 0.47552836),
                                scale * GfVec3f(-0.24999999, -0.58778524, 0.769421),
                                scale * GfVec3f(0.25000003, -0.58778524, 0.7694209),
                                scale * GfVec3f(0.65450853, -0.58778524, 0.47552827),
                                scale * GfVec3f(0.809017, -0.58778524, 0),
                                scale * GfVec3f(0.769421, -0.30901697, -0.5590172),
                                scale * GfVec3f(0.2938926, -0.30901697, -0.9045087),
                                scale * GfVec3f(-0.29389277, -0.30901697, -0.9045086),
                                scale * GfVec3f(-0.7694211, -0.30901697, -0.559017),
                                scale * GfVec3f(-0.95105666, -0.30901697, 5.6687387e-8),
                                scale * GfVec3f(-0.769421, -0.30901697, 0.5590171),
                                scale * GfVec3f(-0.29389262, -0.30901697, 0.9045086),
                                scale * GfVec3f(0.29389268, -0.30901697, 0.90450853),
                                scale * GfVec3f(0.7694209, -0.30901697, 0.559017),
                                scale * GfVec3f(0.95105654, -0.30901697, 0),
                                scale * GfVec3f(0.80901706, 0, -0.5877854),
                                scale * GfVec3f(0.30901694, 0, -0.9510567),
                                scale * GfVec3f(-0.30901715, 0, -0.9510566),
                                scale * GfVec3f(-0.8090172, 0, -0.58778524),
                                scale * GfVec3f(-1.0000001, 0, 5.9604645e-8),
                                scale * GfVec3f(-0.80901706, 0, 0.58778536),
                                scale * GfVec3f(-0.30901697, 0, 0.9510566),
                                scale * GfVec3f(0.30901703, 0, 0.95105654),
                                scale * GfVec3f(0.809017, 0, 0.58778524),
                                scale * GfVec3f(1, 0, 0),
                                scale * GfVec3f(0.769421, 0.30901697, -0.5590172),
                                scale * GfVec3f(0.2938926, 0.30901697, -0.9045087),
                                scale * GfVec3f(-0.29389277, 0.30901697, -0.9045086),
                                scale * GfVec3f(-0.7694211, 0.30901697, -0.559017),
                                scale * GfVec3f(-0.95105666, 0.30901697, 5.6687387e-8),
                                scale * GfVec3f(-0.769421, 0.30901697, 0.5590171),
                                scale * GfVec3f(-0.29389262, 0.30901697, 0.9045086),
                                scale * GfVec3f(0.29389268, 0.30901697, 0.90450853),
                                scale * GfVec3f(0.7694209, 0.30901697, 0.559017),
                                scale * GfVec3f(0.95105654, 0.30901697, 0),
                                scale * GfVec3f(0.65450853, 0.58778524, -0.4755284),
                                scale * GfVec3f(0.24999996, 0.58778524, -0.76942104),
                                scale * GfVec3f(-0.25000012, 0.58778524, -0.769421),
                                scale * GfVec3f(-0.65450865, 0.58778524, -0.47552827),
                                scale * GfVec3f(-0.8090171, 0.58778524, 4.822117e-8),
                                scale * GfVec3f(-0.65450853, 0.58778524, 0.47552836),
                                scale * GfVec3f(-0.24999999, 0.58778524, 0.769421),
                                scale * GfVec3f(0.25000003, 0.58778524, 0.7694209),
                                scale * GfVec3f(0.65450853, 0.58778524, 0.47552827),
                                scale * GfVec3f(0.809017, 0.58778524, 0),
                                scale * GfVec3f(0.4755283, 0.809017, -0.3454916),
                                scale * GfVec3f(0.1816356, 0.809017, -0.5590171),
                                scale * GfVec3f(-0.18163572, 0.809017, -0.55901706),
                                scale * GfVec3f(-0.47552836, 0.809017, -0.3454915),
                                scale * GfVec3f(-0.5877853, 0.809017, 3.503473e-8),
                                scale * GfVec3f(-0.4755283, 0.809017, 0.34549156),
                                scale * GfVec3f(-0.18163562, 0.809017, 0.55901706),
                                scale * GfVec3f(0.18163565, 0.809017, 0.559017),
                                scale * GfVec3f(0.47552827, 0.809017, 0.3454915),
                                scale * GfVec3f(0.58778524, 0.809017, 0),
                                scale * GfVec3f(0.25000003, 0.95105654, -0.1816357),
                                scale * GfVec3f(0.09549149, 0.95105654, -0.2938927),
                                scale * GfVec3f(-0.09549155, 0.95105654, -0.29389265),
                                scale * GfVec3f(-0.25000006, 0.95105654, -0.18163563),
                                scale * GfVec3f(-0.30901703, 0.95105654, 1.8418849e-8),
                                scale * GfVec3f(-0.25000003, 0.95105654, 0.18163568),
                                scale * GfVec3f(-0.0954915, 0.95105654, 0.29389265),
                                scale * GfVec3f(0.09549151, 0.95105654, 0.29389265),
                                scale * GfVec3f(0.25, 0.95105654, 0.18163563),
                                scale * GfVec3f(0.309017, 0.95105654, 0),
                                scale * GfVec3f(0, -1, 0),
                                scale * GfVec3f(0, 1, 0) };

    static size_t num_points = sizeof(points) / sizeof(points[0]);
    static VtArray<GfVec3f> output;
    if (output.empty())
    {
        output.resize(num_points);
        std::copy(points, points + num_points, output.begin());
    }
    return output;
}

const bool DomeLightLocatorRenderData::as_mesh() const
{
    return true;
}

const PXR_NS::GfRange3d& DomeLightLocatorRenderData::bbox() const
{
    static const GfRange3d bbox = { { -1000, -1000, -1000 }, { 1000, 1000, 1000 } };
    return bbox;
}

const bool DomeLightLocatorRenderData::is_double_sided() const
{
    return m_is_double_sided;
}

DirectLightLocatorData::DirectLightLocatorData() {}

void DirectLightLocatorData::update(const std::unordered_map<std::string, VtValue>& data) {}

const PXR_NS::VtArray<int>& DirectLightLocatorData::vertex_per_curve() const
{
    static VtArray<int> vcount { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
    return vcount;
}

const PXR_NS::VtArray<int>& DirectLightLocatorData::vertex_indexes() const
{
    static VtArray<int> indexes {
        0,      1,      1,      2,      1,      3,      1,      4,      1,      5,      0 + 6,  1 + 6,  1 + 6,  2 + 6,
        1 + 6,  3 + 6,  1 + 6,  4 + 6,  1 + 6,  5 + 6,  0 + 12, 1 + 12, 1 + 12, 2 + 12, 1 + 12, 3 + 12, 1 + 12, 4 + 12,
        1 + 12, 5 + 12, 0 + 18, 1 + 18, 1 + 18, 2 + 18, 1 + 18, 3 + 18, 1 + 18, 4 + 18, 1 + 18, 5 + 18,
    };
    return indexes;
}

const PXR_NS::VtVec3fArray& DirectLightLocatorData::vertex_positions() const
{
    static std::mutex sync;
    std::lock_guard<std::mutex> lock(sync);
    static VtVec3fArray points;

    if (points.empty())
    {
        auto m_points = VtVec3fArray {
            { +0, 0, -0 }, { +0, 0, -1 }, { +0, 0.25, -0.75 }, { +0, -0.25, -0.75 }, { +0.25, 0, -0.75 }, { -0.25, 0, -0.75 },
        };

        for (auto point : m_points)
            points.push_back(point + GfVec3f { 0, 0.3f, 0.5 });
        for (auto point : m_points)
            points.push_back(point + GfVec3f { 0, -0.3f, 0.5 });
        for (auto point : m_points)
            points.push_back(point + GfVec3f { 0.3f, 0, 0.5 });
        for (auto point : m_points)
            points.push_back(point + GfVec3f { -0.3f, 0, 0.5 });
    }
    return points;
}

const PXR_NS::GfRange3d& DirectLightLocatorData::bbox() const
{
    static const GfRange3d bbox = { { -2, -2, -2 }, { 2, 2, 2 } };
    return bbox;
}

const PXR_NS::TfToken& DirectLightLocatorData::topology() const
{
    return HdTokens->segmented;
}

SphereLightLocatorRenderData::SphereLightLocatorRenderData()
{
    update_points();
}

void SphereLightLocatorRenderData::update(const std::unordered_map<std::string, VtValue>& data)
{
    auto radius_iter = data.find("radius");
    if (radius_iter != data.end())
    {
        m_radius = radius_iter->second.Get<float>();
    }

    update_points();
}

const PXR_NS::VtArray<int>& SphereLightLocatorRenderData::vertex_per_curve() const
{
    static VtArray<int> vpc = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                                4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                                4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };
    return vpc;
}

const PXR_NS::VtArray<int>& SphereLightLocatorRenderData::vertex_indexes() const
{
    static VtArray<int> indexes = {
        0,  1,  11, 10, 1,  2,  12, 11, 2,  3,  13, 12, 3,  4,  14, 13, 4,  5,  15, 14, 5,  6,  16, 15, 6,  7,  17, 16, 7,  8,  18, 17, 8,  9,  19,
        18, 9,  0,  10, 19, 10, 11, 21, 20, 11, 12, 22, 21, 12, 13, 23, 22, 13, 14, 24, 23, 14, 15, 25, 24, 15, 16, 26, 25, 16, 17, 27, 26, 17, 18,
        28, 27, 18, 19, 29, 28, 19, 10, 20, 29, 20, 21, 31, 30, 21, 22, 32, 31, 22, 23, 33, 32, 23, 24, 34, 33, 24, 25, 35, 34, 25, 26, 36, 35, 26,
        27, 37, 36, 27, 28, 38, 37, 28, 29, 39, 38, 29, 20, 30, 39, 30, 31, 41, 40, 31, 32, 42, 41, 32, 33, 43, 42, 33, 34, 44, 43, 34, 35, 45, 44,
        35, 36, 46, 45, 36, 37, 47, 46, 37, 38, 48, 47, 38, 39, 49, 48, 39, 30, 40, 49, 40, 41, 51, 50, 41, 42, 52, 51, 42, 43, 53, 52, 43, 44, 54,
        53, 44, 45, 55, 54, 45, 46, 56, 55, 46, 47, 57, 56, 47, 48, 58, 57, 48, 49, 59, 58, 49, 40, 50, 59, 50, 51, 61, 60, 51, 52, 62, 61, 52, 53,
        63, 62, 53, 54, 64, 63, 54, 55, 65, 64, 55, 56, 66, 65, 56, 57, 67, 66, 57, 58, 68, 67, 58, 59, 69, 68, 59, 50, 60, 69, 60, 61, 71, 70, 61,
        62, 72, 71, 62, 63, 73, 72, 63, 64, 74, 73, 64, 65, 75, 74, 65, 66, 76, 75, 66, 67, 77, 76, 67, 68, 78, 77, 68, 69, 79, 78, 69, 60, 70, 79,
        70, 71, 81, 80, 71, 72, 82, 81, 72, 73, 83, 82, 73, 74, 84, 83, 74, 75, 85, 84, 75, 76, 86, 85, 76, 77, 87, 86, 77, 78, 88, 87, 78, 79, 89,
        88, 79, 70, 80, 89, 1,  0,  90, 2,  1,  90, 3,  2,  90, 4,  3,  90, 5,  4,  90, 6,  5,  90, 7,  6,  90, 8,  7,  90, 9,  8,  90, 0,  9,  90,
        80, 81, 91, 81, 82, 91, 82, 83, 91, 83, 84, 91, 84, 85, 91, 85, 86, 91, 86, 87, 91, 87, 88, 91, 88, 89, 91, 89, 80, 91
    };
    return indexes;
}

const PXR_NS::VtVec3fArray& SphereLightLocatorRenderData::vertex_positions() const
{
    return m_points;
}

const bool SphereLightLocatorRenderData::as_mesh() const
{
    return true;
}

void SphereLightLocatorRenderData::update_points()
{
    m_points = VtVec3fArray { m_radius * GfVec3f(0.25000003, -0.95105654, -0.1816357),
                              m_radius * GfVec3f(0.09549149, -0.95105654, -0.2938927),
                              m_radius * GfVec3f(-0.09549155, -0.95105654, -0.29389265),
                              m_radius * GfVec3f(-0.25000006, -0.95105654, -0.18163563),
                              m_radius * GfVec3f(-0.30901703, -0.95105654, 1.8418849e-8),
                              m_radius * GfVec3f(-0.25000003, -0.95105654, 0.18163568),
                              m_radius * GfVec3f(-0.0954915, -0.95105654, 0.29389265),
                              m_radius * GfVec3f(0.09549151, -0.95105654, 0.29389265),
                              m_radius * GfVec3f(0.25, -0.95105654, 0.18163563),
                              m_radius * GfVec3f(0.309017, -0.95105654, 0),
                              m_radius * GfVec3f(0.4755283, -0.809017, -0.3454916),
                              m_radius * GfVec3f(0.1816356, -0.809017, -0.5590171),
                              m_radius * GfVec3f(-0.18163572, -0.809017, -0.55901706),
                              m_radius * GfVec3f(-0.47552836, -0.809017, -0.3454915),
                              m_radius * GfVec3f(-0.5877853, -0.809017, 3.503473e-8),
                              m_radius * GfVec3f(-0.4755283, -0.809017, 0.34549156),
                              m_radius * GfVec3f(-0.18163562, -0.809017, 0.55901706),
                              m_radius * GfVec3f(0.18163565, -0.809017, 0.559017),
                              m_radius * GfVec3f(0.47552827, -0.809017, 0.3454915),
                              m_radius * GfVec3f(0.58778524, -0.809017, 0),
                              m_radius * GfVec3f(0.65450853, -0.58778524, -0.4755284),
                              m_radius * GfVec3f(0.24999996, -0.58778524, -0.76942104),
                              m_radius * GfVec3f(-0.25000012, -0.58778524, -0.769421),
                              m_radius * GfVec3f(-0.65450865, -0.58778524, -0.47552827),
                              m_radius * GfVec3f(-0.8090171, -0.58778524, 4.822117e-8),
                              m_radius * GfVec3f(-0.65450853, -0.58778524, 0.47552836),
                              m_radius * GfVec3f(-0.24999999, -0.58778524, 0.769421),
                              m_radius * GfVec3f(0.25000003, -0.58778524, 0.7694209),
                              m_radius * GfVec3f(0.65450853, -0.58778524, 0.47552827),
                              m_radius * GfVec3f(0.809017, -0.58778524, 0),
                              m_radius * GfVec3f(0.769421, -0.30901697, -0.5590172),
                              m_radius * GfVec3f(0.2938926, -0.30901697, -0.9045087),
                              m_radius * GfVec3f(-0.29389277, -0.30901697, -0.9045086),
                              m_radius * GfVec3f(-0.7694211, -0.30901697, -0.559017),
                              m_radius * GfVec3f(-0.95105666, -0.30901697, 5.6687387e-8),
                              m_radius * GfVec3f(-0.769421, -0.30901697, 0.5590171),
                              m_radius * GfVec3f(-0.29389262, -0.30901697, 0.9045086),
                              m_radius * GfVec3f(0.29389268, -0.30901697, 0.90450853),
                              m_radius * GfVec3f(0.7694209, -0.30901697, 0.559017),
                              m_radius * GfVec3f(0.95105654, -0.30901697, 0),
                              m_radius * GfVec3f(0.80901706, 0, -0.5877854),
                              m_radius * GfVec3f(0.30901694, 0, -0.9510567),
                              m_radius * GfVec3f(-0.30901715, 0, -0.9510566),
                              m_radius * GfVec3f(-0.8090172, 0, -0.58778524),
                              m_radius * GfVec3f(-1.0000001, 0, 5.9604645e-8),
                              m_radius * GfVec3f(-0.80901706, 0, 0.58778536),
                              m_radius * GfVec3f(-0.30901697, 0, 0.9510566),
                              m_radius * GfVec3f(0.30901703, 0, 0.95105654),
                              m_radius * GfVec3f(0.809017, 0, 0.58778524),
                              m_radius * GfVec3f(1, 0, 0),
                              m_radius * GfVec3f(0.769421, 0.30901697, -0.5590172),
                              m_radius * GfVec3f(0.2938926, 0.30901697, -0.9045087),
                              m_radius * GfVec3f(-0.29389277, 0.30901697, -0.9045086),
                              m_radius * GfVec3f(-0.7694211, 0.30901697, -0.559017),
                              m_radius * GfVec3f(-0.95105666, 0.30901697, 5.6687387e-8),
                              m_radius * GfVec3f(-0.769421, 0.30901697, 0.5590171),
                              m_radius * GfVec3f(-0.29389262, 0.30901697, 0.9045086),
                              m_radius * GfVec3f(0.29389268, 0.30901697, 0.90450853),
                              m_radius * GfVec3f(0.7694209, 0.30901697, 0.559017),
                              m_radius * GfVec3f(0.95105654, 0.30901697, 0),
                              m_radius * GfVec3f(0.65450853, 0.58778524, -0.4755284),
                              m_radius * GfVec3f(0.24999996, 0.58778524, -0.76942104),
                              m_radius * GfVec3f(-0.25000012, 0.58778524, -0.769421),
                              m_radius * GfVec3f(-0.65450865, 0.58778524, -0.47552827),
                              m_radius * GfVec3f(-0.8090171, 0.58778524, 4.822117e-8),
                              m_radius * GfVec3f(-0.65450853, 0.58778524, 0.47552836),
                              m_radius * GfVec3f(-0.24999999, 0.58778524, 0.769421),
                              m_radius * GfVec3f(0.25000003, 0.58778524, 0.7694209),
                              m_radius * GfVec3f(0.65450853, 0.58778524, 0.47552827),
                              m_radius * GfVec3f(0.809017, 0.58778524, 0),
                              m_radius * GfVec3f(0.4755283, 0.809017, -0.3454916),
                              m_radius * GfVec3f(0.1816356, 0.809017, -0.5590171),
                              m_radius * GfVec3f(-0.18163572, 0.809017, -0.55901706),
                              m_radius * GfVec3f(-0.47552836, 0.809017, -0.3454915),
                              m_radius * GfVec3f(-0.5877853, 0.809017, 3.503473e-8),
                              m_radius * GfVec3f(-0.4755283, 0.809017, 0.34549156),
                              m_radius * GfVec3f(-0.18163562, 0.809017, 0.55901706),
                              m_radius * GfVec3f(0.18163565, 0.809017, 0.559017),
                              m_radius * GfVec3f(0.47552827, 0.809017, 0.3454915),
                              m_radius * GfVec3f(0.58778524, 0.809017, 0),
                              m_radius * GfVec3f(0.25000003, 0.95105654, -0.1816357),
                              m_radius * GfVec3f(0.09549149, 0.95105654, -0.2938927),
                              m_radius * GfVec3f(-0.09549155, 0.95105654, -0.29389265),
                              m_radius * GfVec3f(-0.25000006, 0.95105654, -0.18163563),
                              m_radius * GfVec3f(-0.30901703, 0.95105654, 1.8418849e-8),
                              m_radius * GfVec3f(-0.25000003, 0.95105654, 0.18163568),
                              m_radius * GfVec3f(-0.0954915, 0.95105654, 0.29389265),
                              m_radius * GfVec3f(0.09549151, 0.95105654, 0.29389265),
                              m_radius * GfVec3f(0.25, 0.95105654, 0.18163563),
                              m_radius * GfVec3f(0.309017, 0.95105654, 0),
                              m_radius * GfVec3f(0, -1, 0),
                              m_radius * GfVec3f(0, 1, 0) };
    m_bbox = { { -m_radius, -m_radius, -m_radius }, { m_radius, m_radius, m_radius } };
}

const bool SphereLightLocatorRenderData::is_double_sided() const
{
    return true;
}

const PXR_NS::GfRange3d& SphereLightLocatorRenderData::bbox() const
{
    return m_bbox;
}

DiskLightLocatorRenderData::DiskLightLocatorRenderData()
{
    update_points();
}

void DiskLightLocatorRenderData::update(const std::unordered_map<std::string, VtValue>& data)
{
    auto radius_iter = data.find("radius");
    if (radius_iter != data.end())
    {
        m_radius = radius_iter->second.Get<float>();
    }
    update_points();
}

const PXR_NS::VtArray<int>& DiskLightLocatorRenderData::vertex_per_curve() const
{
    static VtArray<int> vpc = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 };
    return vpc;
}

const PXR_NS::VtArray<int>& DiskLightLocatorRenderData::vertex_indexes() const
{
    static VtArray<int> indexes = { 9,  5,  6,  10, 6,  0,  7,  10, 7,  3,  8,  10, 18, 19, 20, 21, 8,  3,  11, 14, 11, 1,  12, 14,
                                    12, 4,  13, 14, 22, 19, 18, 23, 13, 4,  15, 17, 15, 2,  16, 17, 16, 5,  9,  17, 20, 19, 22, 24,
                                    9,  10, 21, 20, 10, 8,  18, 21, 8,  14, 23, 18, 14, 13, 22, 23, 13, 17, 24, 22, 17, 9,  20, 24 };
    return indexes;
}

const PXR_NS::VtVec3fArray& DiskLightLocatorRenderData::vertex_positions() const
{
    return m_points;
}

void DiskLightLocatorRenderData::update_points()
{
    m_points = VtVec3fArray { GfVec3f(-0.8660254, -0.5, 0),
                              GfVec3f(0.86602545, -0.4999999, 2.9802322e-8),
                              GfVec3f(-1.6292068e-7, 1, 0),
                              GfVec3f(5.9604645e-8, -1, 0),
                              GfVec3f(0.8660253, 0.5000002, 0),
                              GfVec3f(-0.8660255, 0.4999999, -2.9802322e-8),
                              GfVec3f(-1, -1.0323827e-7, 7.1054274e-15),
                              GfVec3f(-0.49999997, -0.86602545, 0),
                              GfVec3f(-1.1603905e-8, -0.059984714, 0),
                              GfVec3f(-0.0519484, 0.029992357, 0),
                              GfVec3f(-0.05194843, -0.029992357, 0),
                              GfVec3f(0.50000006, -0.8660254, 0),
                              GfVec3f(1, 1.5485742e-7, 0),
                              GfVec3f(0.0519484, 0.029992357, 0),
                              GfVec3f(0.05194837, -0.029992357, 1.4901161e-8),
                              GfVec3f(0.49999982, 0.86602557, 0),
                              GfVec3f(-0.5000001, 0.8660254, 0),
                              GfVec3f(-2.308478e-8, 0.059984714, 0),
                              GfVec3f(-1.1917095e-7, -0.059984706, -0.5),
                              GfVec3f(-8.89564e-8, 4.1402703e-8, -0.5),
                              GfVec3f(-0.051948447, 0.029992383, -0.5),
                              GfVec3f(-0.051948525, -0.029992413, -0.5),
                              GfVec3f(0.05194834, 0.029992446, -0.5),
                              GfVec3f(0.05194825, -0.029992286, -0.5),
                              GfVec3f(-2.308478e-8, 0.059984826, -0.5) };

    for (GfVec3f& item : m_points)
    {
        item[0] *= m_radius;
        item[1] *= m_radius;
        item[2] *= m_radius;
    }
    m_bbox = { { -m_radius, -m_radius, -m_radius }, { m_radius, m_radius, m_radius } };
}

const bool DiskLightLocatorRenderData::as_mesh() const
{
    return true;
}

const bool DiskLightLocatorRenderData::is_double_sided() const
{
    return true;
}

const PXR_NS::GfRange3d& DiskLightLocatorRenderData::bbox() const
{
    return m_bbox;
}

CylinderLightLocatorRenderData::CylinderLightLocatorRenderData()
{
    update_points();
}

void CylinderLightLocatorRenderData::update(const std::unordered_map<std::string, VtValue>& data)
{
    auto radius_iter = data.find("radius");
    auto length_iter = data.find("length");
    if (radius_iter != data.end() && length_iter != data.end())
    {
        m_radius = radius_iter->second.Get<float>();
        m_length = length_iter->second.Get<float>();
    }

    update_points();
}

const PXR_NS::VtArray<int>& CylinderLightLocatorRenderData::vertex_per_curve() const
{
    static VtArray<int> vpc = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };
    return vpc;
}

const PXR_NS::VtArray<int>& CylinderLightLocatorRenderData::vertex_indexes() const
{
    static VtArray<int> indexes = { 0,  1,  11, 10, 1,  2,  12, 11, 2,  3,  13, 12, 3,  4,  14, 13, 4,  5,  15, 14, 5,  6,  16, 15, 6,
                                    7,  17, 16, 7,  8,  18, 17, 8,  9,  19, 18, 9,  0,  10, 19, 1,  0,  20, 2,  1,  20, 3,  2,  20, 4,
                                    3,  20, 5,  4,  20, 6,  5,  20, 7,  6,  20, 8,  7,  20, 9,  8,  20, 0,  9,  20, 10, 11, 21, 11, 12,
                                    21, 12, 13, 21, 13, 14, 21, 14, 15, 21, 15, 16, 21, 16, 17, 21, 17, 18, 21, 18, 19, 21, 19, 10, 21 };
    return indexes;
}

const PXR_NS::VtVec3fArray& CylinderLightLocatorRenderData::vertex_positions() const
{
    return m_points;
}

void CylinderLightLocatorRenderData::update_points()
{
    m_points = VtVec3fArray { GfVec3f(0.80901706, -0.5, -0.5877854),
                              GfVec3f(0.30901694, -0.5, -0.9510567),
                              GfVec3f(-0.30901715, -0.5, -0.9510566),
                              GfVec3f(-0.8090172, -0.5, -0.58778524),
                              GfVec3f(-1.0000001, -0.5, 5.9604645e-8),
                              GfVec3f(-0.80901706, -0.5, 0.58778536),
                              GfVec3f(-0.30901697, -0.5, 0.9510566),
                              GfVec3f(0.30901703, -0.5, 0.95105654),
                              GfVec3f(0.809017, -0.5, 0.58778524),
                              GfVec3f(1, -0.5, 0),
                              GfVec3f(0.80901706, 0.5, -0.5877854),
                              GfVec3f(0.30901694, 0.5, -0.9510567),
                              GfVec3f(-0.30901715, 0.5, -0.9510566),
                              GfVec3f(-0.8090172, 0.5, -0.58778524),
                              GfVec3f(-1.0000001, 0.5, 5.9604645e-8),
                              GfVec3f(-0.80901706, 0.5, 0.58778536),
                              GfVec3f(-0.30901697, 0.5, 0.9510566),
                              GfVec3f(0.30901703, 0.5, 0.95105654),
                              GfVec3f(0.809017, 0.5, 0.58778524),
                              GfVec3f(1, 0.5, 0),
                              GfVec3f(0, -0.5, 0),
                              GfVec3f(0, 0.5, 0) };

    for (GfVec3f& item : m_points)
    {
        item[0] *= m_radius;
        item[1] *= m_length;
        item[2] *= m_radius;
    }
    m_bbox = { { -m_radius, -m_length / 2.f, -m_radius }, { m_radius, m_length / 2.f, m_radius } };
}

const bool CylinderLightLocatorRenderData::as_mesh() const
{
    return true;
}

const bool CylinderLightLocatorRenderData::is_double_sided() const
{
    return true;
}

const PXR_NS::GfRange3d& CylinderLightLocatorRenderData::bbox() const
{
    return m_bbox;
}

void LightBlockerLocatorRenderData::update(const std::unordered_map<std::string, VtValue>& data)
{
    auto type_iter = data.find("geometry_type");
    if (type_iter != data.end())
    {
        m_type = type_iter->second.Get<TfToken>();
    }
    assert(!m_type.IsEmpty());
}

const PXR_NS::VtArray<int>& LightBlockerLocatorRenderData::vertex_per_curve() const
{
    static const std::unordered_map<std::string, VtArray<int>> vpc = {
        { "box", { 4, 4, 4, 4, 4, 4 } },
        { "plane", { 4 } },
        { "cylinder", { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 } },
        { "sphere", { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 } }
    };

    return vpc.at(m_type.GetString());
}

const PXR_NS::VtArray<int>& LightBlockerLocatorRenderData::vertex_indexes() const
{
    static const std::unordered_map<std::string, VtArray<int>> indexes = {
        { "box", { 0, 1, 3, 2, 2, 3, 5, 4, 4, 5, 7, 6, 6, 7, 1, 0, 1, 7, 5, 3, 6, 0, 2, 4 } },
        { "plane", { 0, 1, 2, 3 } },
        { "cylinder",
          { 0,  1,  11, 10, 1,  2,  12, 11, 2,  3,  13, 12, 3,  4,  14, 13, 4,  5,  15, 14, 5,  6,  16, 15, 6,  7,  17, 16, 7,  8,  18, 17, 8,  9,
            19, 18, 9,  0,  10, 19, 1,  0,  20, 2,  1,  20, 3,  2,  20, 4,  3,  20, 5,  4,  20, 6,  5,  20, 7,  6,  20, 8,  7,  20, 9,  8,  20, 0,
            9,  20, 10, 11, 21, 11, 12, 21, 12, 13, 21, 13, 14, 21, 14, 15, 21, 15, 16, 21, 16, 17, 21, 17, 18, 21, 18, 19, 21, 19, 10, 21 } },
        { "sphere", { 0,  1,  11, 10, 1,  2,  12, 11, 2,  3,  13, 12, 3,  4,  14, 13, 4,  5,  15, 14, 5,  6,  16, 15, 6,  7,  17, 16, 7,  8,  18, 17,
                      8,  9,  19, 18, 9,  0,  10, 19, 10, 11, 21, 20, 11, 12, 22, 21, 12, 13, 23, 22, 13, 14, 24, 23, 14, 15, 25, 24, 15, 16, 26, 25,
                      16, 17, 27, 26, 17, 18, 28, 27, 18, 19, 29, 28, 19, 10, 20, 29, 20, 21, 31, 30, 21, 22, 32, 31, 22, 23, 33, 32, 23, 24, 34, 33,
                      24, 25, 35, 34, 25, 26, 36, 35, 26, 27, 37, 36, 27, 28, 38, 37, 28, 29, 39, 38, 29, 20, 30, 39, 30, 31, 41, 40, 31, 32, 42, 41,
                      32, 33, 43, 42, 33, 34, 44, 43, 34, 35, 45, 44, 35, 36, 46, 45, 36, 37, 47, 46, 37, 38, 48, 47, 38, 39, 49, 48, 39, 30, 40, 49,
                      40, 41, 51, 50, 41, 42, 52, 51, 42, 43, 53, 52, 43, 44, 54, 53, 44, 45, 55, 54, 45, 46, 56, 55, 46, 47, 57, 56, 47, 48, 58, 57,
                      48, 49, 59, 58, 49, 40, 50, 59, 50, 51, 61, 60, 51, 52, 62, 61, 52, 53, 63, 62, 53, 54, 64, 63, 54, 55, 65, 64, 55, 56, 66, 65,
                      56, 57, 67, 66, 57, 58, 68, 67, 58, 59, 69, 68, 59, 50, 60, 69, 60, 61, 71, 70, 61, 62, 72, 71, 62, 63, 73, 72, 63, 64, 74, 73,
                      64, 65, 75, 74, 65, 66, 76, 75, 66, 67, 77, 76, 67, 68, 78, 77, 68, 69, 79, 78, 69, 60, 70, 79, 70, 71, 81, 80, 71, 72, 82, 81,
                      72, 73, 83, 82, 73, 74, 84, 83, 74, 75, 85, 84, 75, 76, 86, 85, 76, 77, 87, 86, 77, 78, 88, 87, 78, 79, 89, 88, 79, 70, 80, 89,
                      1,  0,  90, 2,  1,  90, 3,  2,  90, 4,  3,  90, 5,  4,  90, 6,  5,  90, 7,  6,  90, 8,  7,  90, 9,  8,  90, 0,  9,  90, 80, 81,
                      91, 81, 82, 91, 82, 83, 91, 83, 84, 91, 84, 85, 91, 85, 86, 91, 86, 87, 91, 87, 88, 91, 88, 89, 91, 89, 80, 91 } }
    };

    return indexes.at(m_type.GetString());
}

const PXR_NS::VtVec3fArray& LightBlockerLocatorRenderData::vertex_positions() const
{
    static const std::unordered_map<std::string, VtArray<GfVec3f>> points = {
        { "box",
          { GfVec3f(-0.5, -0.5, 0.5), GfVec3f(0.5, -0.5, 0.5), GfVec3f(-0.5, 0.5, 0.5), GfVec3f(0.5, 0.5, 0.5), GfVec3f(-0.5, 0.5, -0.5),
            GfVec3f(0.5, 0.5, -0.5), GfVec3f(-0.5, -0.5, -0.5), GfVec3f(0.5, -0.5, -0.5) } },
        { "plane", { GfVec3f(-0.5, -0.5, 0), GfVec3f(0.5, -0.5, 0), GfVec3f(0.5, 0.5, 0), GfVec3f(-0.5, 0.5, 0) } },
        { "cylinder",
          { GfVec3f(0.40450853, -0.5, -0.2938927),
            GfVec3f(0.15450847, -0.5, -0.47552836),
            GfVec3f(-0.15450858, -0.5, -0.4755283),
            GfVec3f(-0.4045086, -0.5, -0.29389262),
            GfVec3f(-0.50000006, -0.5, 2.9802322e-8),
            GfVec3f(-0.40450853, -0.5, 0.29389268),
            GfVec3f(-0.15450849, -0.5, 0.4755283),
            GfVec3f(0.15450852, -0.5, 0.47552827),
            GfVec3f(0.4045085, -0.5, 0.29389262),
            GfVec3f(0.5, -0.5, 0),
            GfVec3f(0.40450853, 0.5, -0.2938927),
            GfVec3f(0.15450847, 0.5, -0.47552836),
            GfVec3f(-0.15450858, 0.5, -0.4755283),
            GfVec3f(-0.4045086, 0.5, -0.29389262),
            GfVec3f(-0.50000006, 0.5, 2.9802322e-8),
            GfVec3f(-0.40450853, 0.5, 0.29389268),
            GfVec3f(-0.15450849, 0.5, 0.4755283),
            GfVec3f(0.15450852, 0.5, 0.47552827),
            GfVec3f(0.4045085, 0.5, 0.29389262),
            GfVec3f(0.5, 0.5, 0),
            GfVec3f(0, -0.5, 0),
            GfVec3f(0, 0.5, 0) } },
        { "sphere",
          { GfVec3f(0.12500001, -0.47552827, -0.09081785),
            GfVec3f(0.047745746, -0.47552827, -0.14694636),
            GfVec3f(-0.047745775, -0.47552827, -0.14694633),
            GfVec3f(-0.12500003, -0.47552827, -0.09081782),
            GfVec3f(-0.15450852, -0.47552827, 9.209424e-9),
            GfVec3f(-0.12500001, -0.47552827, 0.09081784),
            GfVec3f(-0.04774575, -0.47552827, 0.14694633),
            GfVec3f(0.047745757, -0.47552827, 0.14694633),
            GfVec3f(0.125, -0.47552827, 0.09081782),
            GfVec3f(0.1545085, -0.47552827, 0),
            GfVec3f(0.23776415, -0.4045085, -0.1727458),
            GfVec3f(0.0908178, -0.4045085, -0.27950856),
            GfVec3f(-0.09081786, -0.4045085, -0.27950853),
            GfVec3f(-0.23776418, -0.4045085, -0.17274575),
            GfVec3f(-0.29389265, -0.4045085, 1.7517365e-8),
            GfVec3f(-0.23776415, -0.4045085, 0.17274578),
            GfVec3f(-0.09081781, -0.4045085, 0.27950853),
            GfVec3f(0.090817824, -0.4045085, 0.2795085),
            GfVec3f(0.23776414, -0.4045085, 0.17274575),
            GfVec3f(0.29389262, -0.4045085, 0),
            GfVec3f(0.32725427, -0.29389262, -0.2377642),
            GfVec3f(0.12499998, -0.29389262, -0.38471052),
            GfVec3f(-0.12500006, -0.29389262, -0.3847105),
            GfVec3f(-0.32725433, -0.29389262, -0.23776414),
            GfVec3f(-0.40450856, -0.29389262, 2.4110586e-8),
            GfVec3f(-0.32725427, -0.29389262, 0.23776418),
            GfVec3f(-0.12499999, -0.29389262, 0.3847105),
            GfVec3f(0.12500001, -0.29389262, 0.38471046),
            GfVec3f(0.32725427, -0.29389262, 0.23776414),
            GfVec3f(0.4045085, -0.29389262, 0),
            GfVec3f(0.3847105, -0.15450849, -0.2795086),
            GfVec3f(0.1469463, -0.15450849, -0.45225435),
            GfVec3f(-0.14694639, -0.15450849, -0.4522543),
            GfVec3f(-0.38471055, -0.15450849, -0.2795085),
            GfVec3f(-0.47552833, -0.15450849, 2.8343694e-8),
            GfVec3f(-0.3847105, -0.15450849, 0.27950856),
            GfVec3f(-0.14694631, -0.15450849, 0.4522543),
            GfVec3f(0.14694634, -0.15450849, 0.45225427),
            GfVec3f(0.38471046, -0.15450849, 0.2795085),
            GfVec3f(0.47552827, -0.15450849, 0),
            GfVec3f(0.40450853, 0, -0.2938927),
            GfVec3f(0.15450847, 0, -0.47552836),
            GfVec3f(-0.15450858, 0, -0.4755283),
            GfVec3f(-0.4045086, 0, -0.29389262),
            GfVec3f(-0.50000006, 0, 2.9802322e-8),
            GfVec3f(-0.40450853, 0, 0.29389268),
            GfVec3f(-0.15450849, 0, 0.4755283),
            GfVec3f(0.15450852, 0, 0.47552827),
            GfVec3f(0.4045085, 0, 0.29389262),
            GfVec3f(0.5, 0, 0),
            GfVec3f(0.3847105, 0.15450849, -0.2795086),
            GfVec3f(0.1469463, 0.15450849, -0.45225435),
            GfVec3f(-0.14694639, 0.15450849, -0.4522543),
            GfVec3f(-0.38471055, 0.15450849, -0.2795085),
            GfVec3f(-0.47552833, 0.15450849, 2.8343694e-8),
            GfVec3f(-0.3847105, 0.15450849, 0.27950856),
            GfVec3f(-0.14694631, 0.15450849, 0.4522543),
            GfVec3f(0.14694634, 0.15450849, 0.45225427),
            GfVec3f(0.38471046, 0.15450849, 0.2795085),
            GfVec3f(0.47552827, 0.15450849, 0),
            GfVec3f(0.32725427, 0.29389262, -0.2377642),
            GfVec3f(0.12499998, 0.29389262, -0.38471052),
            GfVec3f(-0.12500006, 0.29389262, -0.3847105),
            GfVec3f(-0.32725433, 0.29389262, -0.23776414),
            GfVec3f(-0.40450856, 0.29389262, 2.4110586e-8),
            GfVec3f(-0.32725427, 0.29389262, 0.23776418),
            GfVec3f(-0.12499999, 0.29389262, 0.3847105),
            GfVec3f(0.12500001, 0.29389262, 0.38471046),
            GfVec3f(0.32725427, 0.29389262, 0.23776414),
            GfVec3f(0.4045085, 0.29389262, 0),
            GfVec3f(0.23776415, 0.4045085, -0.1727458),
            GfVec3f(0.0908178, 0.4045085, -0.27950856),
            GfVec3f(-0.09081786, 0.4045085, -0.27950853),
            GfVec3f(-0.23776418, 0.4045085, -0.17274575),
            GfVec3f(-0.29389265, 0.4045085, 1.7517365e-8),
            GfVec3f(-0.23776415, 0.4045085, 0.17274578),
            GfVec3f(-0.09081781, 0.4045085, 0.27950853),
            GfVec3f(0.090817824, 0.4045085, 0.2795085),
            GfVec3f(0.23776414, 0.4045085, 0.17274575),
            GfVec3f(0.29389262, 0.4045085, 0),
            GfVec3f(0.12500001, 0.47552827, -0.09081785),
            GfVec3f(0.047745746, 0.47552827, -0.14694636),
            GfVec3f(-0.047745775, 0.47552827, -0.14694633),
            GfVec3f(-0.12500003, 0.47552827, -0.09081782),
            GfVec3f(-0.15450852, 0.47552827, 9.209424e-9),
            GfVec3f(-0.12500001, 0.47552827, 0.09081784),
            GfVec3f(-0.04774575, 0.47552827, 0.14694633),
            GfVec3f(0.047745757, 0.47552827, 0.14694633),
            GfVec3f(0.125, 0.47552827, 0.09081782),
            GfVec3f(0.1545085, 0.47552827, 0),
            GfVec3f(0, -0.5, 0),
            GfVec3f(0, 0.5, 0) } }
    };

    return points.at(m_type.GetString());
}

const bool LightBlockerLocatorRenderData::is_double_sided() const
{
    return true;
}

const PXR_NS::GfRange3d& LightBlockerLocatorRenderData::bbox() const
{
    static const GfRange3d bbox = { { -0.5, -0.5, -0.5 }, { 0.5, 0.5, 0.5 } };
    return bbox;
}

const PXR_NS::TfToken& LightBlockerLocatorRenderData::topology() const
{
    return HdTokens->periodic;
}

OPENDCC_NAMESPACE_CLOSE