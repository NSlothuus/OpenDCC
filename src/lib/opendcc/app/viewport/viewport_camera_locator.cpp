// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_camera_locator.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE;

void CameraLocatorRenderData::update(const std::unordered_map<std::string, PXR_NS::VtValue>& data)
{
    auto frustum_iter = data.find("frustum");

    if (frustum_iter != data.end())
    {
        const auto& frustum = frustum_iter->second.Get<GfFrustum>();
        m_proj_type = frustum.GetProjectionType();
        m_near_far_corners = frustum.ComputeCorners();

        update_points();
    }
}

const PXR_NS::VtArray<int>& CameraLocatorRenderData::vertex_per_curve() const
{
    static const VtArray<int> vpc_persp = {
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, // camera mesh
        4, // near plane mesh
        3, 3, // frustum bottom and top
        2, 2 // frustum left and right
    };
    static const VtArray<int> vpc_orth = {
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, // camera mesh
        4, // near plane mesh
        4, 4, // frustum bottom and top
        4, 4 // frustum left and right
    };
    return m_proj_type == GfFrustum::Orthographic ? vpc_orth : vpc_persp;
}

const PXR_NS::VtArray<int>& CameraLocatorRenderData::vertex_indexes() const
{
    static const VtArray<int> indexes_persp = {
        1,  2,  3,  0,  7,  5,  0,  3,  0,  5,  4,  1,  6,  2,  1,  4,  7,  6,  4,  5,  7,  3,  2,  6,  8,  10, 11, 9,  12, 13,
        15, 14, 8,  9,  13, 12, 9,  11, 15, 13, 11, 10, 14, 15, 10, 8,  12, 14, 16, 17, 27, 26, 28, 27, 17, 18, 18, 19, 29, 28,
        19, 20, 30, 29, 52, 53, 31, 21, 22, 23, 33, 32, 23, 24, 34, 33, 24, 25, 35, 34, 25, 16, 26, 35, 21, 31, 37, 36, 36, 37,
        39, 38, 38, 39, 41, 40, 40, 41, 43, 42, 42, 43, 45, 44, 44, 45, 47, 46, 46, 47, 49, 48, 48, 49, 51, 50, 22, 32, 53, 52,
        54, 56, 57, 55, 58, 59, 61, 60, 54, 55, 59, 58, 55, 57, 61, 59, 57, 56, 60, 61, 56, 54, 58, 60, 62, 63, 71, 70, 63, 64,
        72, 71, 64, 65, 73, 72, 65, 66, 74, 73, 66, 67, 75, 74, 67, 68, 76, 75, 68, 69, 77, 76, 69, 62, 70, 77, // camera indexes
        78, 79, 81, 80, // near plane
        86, 82, 83, // frustum bottom
        86, 84, 85, // frustum top
        82, 84, // frustum left
        83, 85 // frustum right
    };

    static const VtArray<int> indexes_orth = {
        1,  2,  3,  0,  7,  5,  0,  3,  0,  5,  4,  1,  6,  2,  1,  4,  7,  6,  4,  5,  7,  3,  2,  6,  8,  10, 11, 9,  12, 13,
        15, 14, 8,  9,  13, 12, 9,  11, 15, 13, 11, 10, 14, 15, 10, 8,  12, 14, 16, 17, 27, 26, 28, 27, 17, 18, 18, 19, 29, 28,
        19, 20, 30, 29, 52, 53, 31, 21, 22, 23, 33, 32, 23, 24, 34, 33, 24, 25, 35, 34, 25, 16, 26, 35, 21, 31, 37, 36, 36, 37,
        39, 38, 38, 39, 41, 40, 40, 41, 43, 42, 42, 43, 45, 44, 44, 45, 47, 46, 46, 47, 49, 48, 48, 49, 51, 50, 22, 32, 53, 52,
        54, 56, 57, 55, 58, 59, 61, 60, 54, 55, 59, 58, 55, 57, 61, 59, 57, 56, 60, 61, 56, 54, 58, 60, 62, 63, 71, 70, 63, 64,
        72, 71, 64, 65, 73, 72, 65, 66, 74, 73, 66, 67, 75, 74, 67, 68, 76, 75, 68, 69, 77, 76, 69, 62, 70, 77, // camera indexes
        78, 79, 81, 80, // near plane
        78, 79, 83, 82, // frustum bottom
        80, 81, 85, 84, // frustum top
        78, 80, 84, 82, // frustum left
        81, 79, 83, 85 // frustum right
    };
    return m_proj_type == GfFrustum::Orthographic ? indexes_orth : indexes_persp;
}

const PXR_NS::VtVec3fArray& CameraLocatorRenderData::vertex_positions() const
{
    return m_points;
}

const PXR_NS::GfRange3d& CameraLocatorRenderData::bbox() const
{
    static const GfRange3d bbox = { GfVec3f(-0.21595395, -0.44294232, -0.45109898), GfVec3f(0.32576174, 1.0769494, 1.985254) };
    return bbox;
}

const PXR_NS::TfToken& CameraLocatorRenderData::topology() const
{

    return HdTokens->periodic;
}

void CameraLocatorRenderData::update_points()
{
    static const int frustum_start_ind = m_points.size() - m_near_far_corners.size() - 1;

    for (int i = 0; i < m_near_far_corners.size(); i++)
    {
        m_points[frustum_start_ind + i] = GfVec3f(m_near_far_corners[i]);
    }
}

CameraLocatorRenderData::CameraLocatorRenderData()
{
    m_points = { GfVec3f(0.20221844, 0.44294232, 1.3530993),
                 GfVec3f(0.20221865, 0.44294232, -9.6425396e-8),
                 GfVec3f(-0.20241198, 0.4429419, 9.8352887e-17),
                 GfVec3f(-0.20241034, 0.4429422, 1.3530993),
                 GfVec3f(0.2022183, -0.4429422, -4.8213266e-8),
                 GfVec3f(0.20221809, -0.44294173, 1.3530993),
                 GfVec3f(-0.20241201, -0.44294214, 1.6874466e-7),
                 GfVec3f(-0.20241195, -0.44294202, 1.3530993),
                 GfVec3f(0.32576174, -0.44294196, 0.23708329),
                 GfVec3f(0.32576168, -0.44294232, 1.2547089),
                 GfVec3f(0.32576174, 0.19401172, 0.23708329),
                 GfVec3f(0.32575798, 0.19401136, 1.027571),
                 GfVec3f(0.20221838, -0.44294232, 0.2370832),
                 GfVec3f(0.20221835, -0.44294202, 1.2547089),
                 GfVec3f(0.20221856, 0.19401172, 0.23708323),
                 GfVec3f(0.20221806, 0.19401154, 1.027571),
                 GfVec3f(-0.20241201, 0.0371134, 1.9852538),
                 GfVec3f(-0.20241034, -0.17325565, 1.9214218),
                 GfVec3f(-0.20241201, -0.33888054, 1.7723937),
                 GfVec3f(-0.20241034, -0.42417336, 1.5689058),
                 GfVec3f(-0.20241034, -0.4130404, 1.3530993),
                 GfVec3f(-0.20241034, 0.6398301, 1.3136928),
                 GfVec3f(-0.20241034, 0.569958, 1.3986535),
                 GfVec3f(-0.20241216, 0.54691607, 1.6382532),
                 GfVec3f(-0.20241195, 0.4335292, 1.8266602),
                 GfVec3f(-0.20241204, 0.25215033, 1.9528282),
                 GfVec3f(0.20221817, 0.03711386, 1.985254),
                 GfVec3f(0.20221823, -0.17325565, 1.9214218),
                 GfVec3f(0.20221856, -0.33888054, 1.7723937),
                 GfVec3f(0.20221806, -0.42417383, 1.5689058),
                 GfVec3f(0.20221856, -0.4130404, 1.3530993),
                 GfVec3f(0.20221812, 0.63983, 1.3136928),
                 GfVec3f(0.2022183, 0.569958, 1.3986411),
                 GfVec3f(0.20221809, 0.54691607, 1.6382532),
                 GfVec3f(0.2022185, 0.4335292, 1.8266602),
                 GfVec3f(0.2022183, 0.2521504, 1.9528282),
                 GfVec3f(-0.20241034, 0.83528244, 1.2458061),
                 GfVec3f(0.20221832, 0.83528244, 1.2458061),
                 GfVec3f(-0.20241034, 0.9907068, 1.1036619),
                 GfVec3f(0.2022183, 0.9907068, 1.1036619),
                 GfVec3f(-0.20241195, 1.0769494, 0.91158986),
                 GfVec3f(0.20221835, 1.0769494, 0.9115901),
                 GfVec3f(-0.20241195, 1.0769494, 0.6975349),
                 GfVec3f(0.20221856, 1.0769494, 0.6975349),
                 GfVec3f(-0.20241034, 0.99365103, 0.50427926),
                 GfVec3f(0.20221835, 0.99365103, 0.50427926),
                 GfVec3f(-0.20241034, 0.8448913, 0.36039627),
                 GfVec3f(0.20221838, 0.8448913, 0.3603962),
                 GfVec3f(-0.20241034, 0.65627414, 0.29252732),
                 GfVec3f(0.2022183, 0.65627414, 0.29252857),
                 GfVec3f(-0.20241198, 0.44294232, 0.30369177),
                 GfVec3f(0.20221835, 0.44294173, 0.303692),
                 GfVec3f(-0.20241195, 0.5883954, 1.3408777),
                 GfVec3f(0.2022183, 0.5883954, 1.3408777),
                 GfVec3f(-0.20241216, -0.20119959, -0.3139274),
                 GfVec3f(0.2022183, -0.2011995, -0.31392688),
                 GfVec3f(-0.21595216, -0.21466708, -0.45109898),
                 GfVec3f(0.2157599, -0.21466702, -0.45109898),
                 GfVec3f(-0.20241198, 0.20119956, -0.3139273),
                 GfVec3f(0.20221841, 0.20119956, -0.313927),
                 GfVec3f(-0.21595395, 0.21466666, -0.45109898),
                 GfVec3f(0.21576, 0.21466646, -0.45109898),
                 GfVec3f(0.20221868, 7.142651e-8, -4.8213266e-8),
                 GfVec3f(0.1432882, -0.14226942, 4.821252e-8),
                 GfVec3f(-1.205313e-8, -0.20119956, -6.0265735e-9),
                 GfVec3f(-0.14348303, -0.14226942, -3.1590157e-17),
                 GfVec3f(-0.20241216, 2.4741624e-8, 5.493744e-24),
                 GfVec3f(-0.14348303, 0.14226954, -2.1093083e-8),
                 GfVec3f(0, 0.20119956, 6.0265735e-9),
                 GfVec3f(0.1432882, 0.14226948, 1.2053192e-8),
                 GfVec3f(0.20221841, -3.40699e-8, -0.3139271),
                 GfVec3f(0.1432882, -0.14226954, -0.31392777),
                 GfVec3f(-3.615949e-8, -0.20119971, -0.31392688),
                 GfVec3f(-0.14348303, -0.14226957, -0.3139277),
                 GfVec3f(-0.20241216, -6.8701404e-8, -0.3139273),
                 GfVec3f(-0.14348303, 0.14226948, -0.31392735),
                 GfVec3f(1.08477835e-7, 0.20119944, -0.3139264),
                 GfVec3f(0.1432882, 0.14226903, -0.31392705) }; // camera mesh

    m_near_far_corners.resize(8); // 4 per each plane
    for (int i = 0; i < m_near_far_corners.size() + 1; i++) // + 1 vertex for origin where frustum begins
    {
        m_points.push_back(GfVec3f(0, 0, 0));
    }
}

OPENDCC_NAMESPACE_CLOSE