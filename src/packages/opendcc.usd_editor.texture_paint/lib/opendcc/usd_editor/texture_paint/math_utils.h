/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include <vector>
#include <algorithm>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/range2f.h>
#include <pxr/base/gf/rect2i.h>
#include <pxr/base/gf/line2d.h>

OPENDCC_NAMESPACE_OPEN

namespace math_utils
{
    PXR_NAMESPACE_USING_DIRECTIVE
    enum OutCodeFlags
    {
        INSIDE = 0,
        LEFT = 1,
        RIGHT = 2,
        BOTTOM = 4,
        TOP = 8
    };

    uint8_t unit_float_to_uchar(float val)
    {
        if (val <= 0)
            return 0;
        if (val > 1 - 0.5f / 255.0f)
            return 255;
        return static_cast<uint8_t>(255 * val + 0.5f);
    }
    uint8_t calc_out_code(const GfVec2f& p, const GfRange2f& box)
    {
        uint8_t result = INSIDE;
        if (p[0] < box.GetMin()[0])
            result |= OutCodeFlags::LEFT;
        else if (p[0] > box.GetMax()[0])
            result |= OutCodeFlags::RIGHT;
        else if (p[1] < box.GetMin()[1])
            result |= OutCodeFlags::BOTTOM;
        else if (p[1] > box.GetMax()[1])
            result |= TOP;
        return result;
    }

    float cross_2d(const GfVec2f& v1, const GfVec2f& v2)
    {
        return v1[0] * v2[1] - v1[1] * v2[0];
    }
    template <class T>
    T bary_interp(const T& a, const T& b, const T& c, const GfVec3f& uvw)
    {
        return a * uvw[0] + b * uvw[1] + c * uvw[2];
    };

    GfVec3f to_bary_2d(const GfVec2f& v1, const GfVec2f& v2, const GfVec2f& v3, const GfVec2f& p)
    {
        GfVec3f result;
        result[0] = cross_2d(v2 - v3, v3 - p);
        result[1] = cross_2d(v3 - v1, v1 - p);
        result[2] = cross_2d(v1 - v2, v2 - p);
        float sum_w = result[0] + result[1] + result[2];
        if (GfIsClose(sum_w, 0.0, 0.00001))
            return GfVec3f(1.0 / 3);
        return result / sum_w;
    }
    GfVec3f to_bary_2d_persp_cor(const GfVec2f& v1, const GfVec2f& v2, const GfVec2f& v3, const GfVec3f& persp_weights, const GfVec2f& p)
    {
        GfVec3f result;
        result[0] = cross_2d(v2 - v3, v3 - p) / persp_weights[0];
        result[1] = cross_2d(v3 - v1, v1 - p) / persp_weights[1];
        result[2] = cross_2d(v1 - v2, v2 - p) / persp_weights[2];
        float sum_w = result[0] + result[1] + result[2];
        if (GfIsClose(sum_w, 0.0, 0.00001))
            return GfVec3f(1.0 / 3);
        return result / sum_w;
    }

    bool left_of_line(const GfVec2f& p, const GfVec2f& l1, const GfVec2f& l2)
    {
        return (l1[0] - p[0]) * (l2[1] - p[1]) > (l2[0] - p[0]) * (l1[1] - p[1]);
    }

    bool is_inside(const GfRange2f& rect, float x, float y)
    {
        return rect.GetMin()[0] <= x && x <= rect.GetMax()[0] && rect.GetMin()[1] <= y && y <= rect.GetMax()[1];
    }
    bool intersect_segment_segment(const GfVec2f& v1, const GfVec2f& v2, const GfVec2f& v3, const GfVec2f& v4)
    {
        return left_of_line(v1, v3, v4) != left_of_line(v2, v3, v4) && left_of_line(v1, v2, v3) != left_of_line(v1, v2, v4);
    }
    void intersect_segment_segment(GfVec2f v0, GfVec2f v1, GfVec2f v2, GfVec2f v3, GfVec2f& result)
    {
        const float endpoint_bias = 1e-6f;
        GfVec2f s10, s32, s30;
        float d;
        const auto eps = endpoint_bias;
        const auto endpoint_min = -endpoint_bias;
        const auto endpoint_max = endpoint_bias + 1.0f;

        s10 = v1 - v0;
        s32 = v3 - v2;
        s30 = v3 - v0;

        d = cross_2d(s10, s32);

        if (d != 0)
        {
            float u, v;
            u = cross_2d(s30, s32) / d;
            v = cross_2d(s10, s30) / d;

            if ((u >= endpoint_min && u <= endpoint_max) && (v >= endpoint_min && v <= endpoint_max))
            {
                GfVec2f vi_test = v0 + s10 * u;
                GfVec2f s_vi_v2 = vi_test - v2;
                v = (s32 * s_vi_v2) / (s32 * s32);
                if (v >= endpoint_min && v <= endpoint_max)
                {
                    result = vi_test;
                    return;
                }
            }

            // out of seg
            return;
        }
        if ((cross_2d(s10, s30) == 0.0f) && (cross_2d(s32, s30) == 0.0))
        {
            GfVec2f s20;
            float u_a, u_b;
            if (v0 == v1)
            {
                if ((v2 - v3).GetLengthSq() > eps * eps)
                {
                    std::swap(v0, v2);
                    std::swap(v1, v3);
                    s10 = v1 - v0;
                    s30 = v3 - v0;
                }
                else
                {
                    if (v0 == v2)
                    {
                        result = v0;
                        // two equal points
                        return;
                    }
                    // two diff points
                    return;
                }
            }
            s20 = v2 - v0;

            u_a = (s20 * s10) / (s10 * s10);
            u_b = (s30 * s10) / (s10 * s10);
            if (u_a > u_b)
                std::swap(u_a, u_b);

            if (u_a > endpoint_max || u_b < endpoint_min)
                // non overlapping segs
                return;
            if (std::max(0.0f, u_a) == std::min(1.0f, u_b))
            {
                // one common point
                result = v0 + s10 * std::max(0.0f, u_a);
                return;
            }
        }
        // collinear
        return;
    }

    bool intersect_point_triangle(const GfVec2f& p, const GfVec2f& v1, const GfVec2f& v2, const GfVec2f& v3)
    {
        if (left_of_line(p, v1, v2))
            return left_of_line(p, v2, v3) && left_of_line(p, v3, v1);
        else
            return !left_of_line(p, v2, v3) && !left_of_line(p, v3, v1);
    }
    bool intersect_point_quad(const GfVec2f& p, const GfVec2f& v1, const GfVec2f& v2, const GfVec2f& v3, const GfVec2f& v4)
    {
        if (left_of_line(p, v1, v2))
            return left_of_line(p, v2, v3) && left_of_line(p, v3, v4) && left_of_line(p, v4, v1);
        else
            return !left_of_line(p, v2, v3) && !left_of_line(p, v3, v4) && !left_of_line(p, v4, v1);
    }
    bool intersect_triangle_rect(const GfVec2f& v1, const GfVec2f& v2, const GfVec2f& v3, const GfRange2f& rect)
    {
        if (rect.Contains(v1) || rect.Contains(v2) || rect.Contains(v3))
            return true;

        const auto p1 = rect.GetCorner(0);
        const auto p2 = rect.GetCorner(1);
        const auto p3 = rect.GetCorner(2);
        const auto p4 = rect.GetCorner(3);
        if (intersect_point_triangle(p1, v1, v2, v3) || intersect_point_triangle(p2, v1, v2, v3) || intersect_point_triangle(p3, v1, v2, v3) ||
            intersect_point_triangle(p4, v1, v2, v3) || intersect_segment_segment(p1, p2, v1, v2) || intersect_segment_segment(p1, p2, v2, v3) ||
            intersect_segment_segment(p2, p3, v1, v2) || intersect_segment_segment(p2, p3, v2, v3) || intersect_segment_segment(p3, p4, v1, v2) ||
            intersect_segment_segment(p3, p4, v2, v3) || intersect_segment_segment(p4, p1, v1, v2) || intersect_segment_segment(p4, p1, v2, v3))
        {
            return true;
        }
        return false;
    }

    float get_uv_point_on_line(const GfVec2i& viewport_dims, const GfMatrix4f& inv_view_proj, const GfVec3f& cam_pos, const GfVec2f& p,
                               const GfVec3f& w1, const GfVec3f& w2)
    {
        GfVec3f dir;
        dir[0] = 2.0f * (p[0] / viewport_dims[0]) - 1.0f;
        dir[1] = 2.0f * (p[1] / viewport_dims[1]) - 1.0f;
        dir[2] = -0.5f;

        GfVec4f tmp(dir[0], dir[1], dir[2], 1);
        tmp = tmp * inv_view_proj;
        dir[0] = tmp[0] / std::abs(tmp[3]);
        dir[1] = tmp[1] / std::abs(tmp[3]);
        dir[2] = tmp[2] / std::abs(tmp[3]);
        dir -= cam_pos;

        auto v1_proj = w1 - cam_pos;
        v1_proj = v1_proj + dir * -((v1_proj * dir) / (dir * dir));
        auto v2_proj = w2 - cam_pos;
        v2_proj = v2_proj + dir * -((v2_proj * dir) / (dir * dir));

        GfVec2f u = GfVec2f(v2_proj[0], v2_proj[1]) - GfVec2f(v1_proj[0], v1_proj[1]);
        GfVec2f h = GfVec2f(0, 0) - GfVec2f(v1_proj[0], v1_proj[1]);
        float dot = GfVec2f(u[0], u[1]) * GfVec2f(u[0], u[1]);
        return dot > 0.0 ? ((u * h) / dot) : 0.0;
    }

    bool intersect_triangle_rect(const GfVec4f& v1, const GfVec4f& v2, const GfVec4f& v3, const GfRange2f& rect)
    {
        return intersect_triangle_rect(GfVec2f(v1[0], v1[1]), GfVec2f(v2[0], v2[1]), GfVec2f(v3[0], v3[1]), rect);
    }
    bool should_cull(const GfVec4f& v1, const GfVec4f& v2, const GfVec4f& v3, const GfRange2f& rect, bool persp)
    {
        if (persp && (std::isnan(v1[0]) || std::isnan(v2[0]) || std::isnan(v3[0])))
            return true;

        return (v1[0] < rect.GetMin()[0] && v2[0] < rect.GetMin()[0] && v3[0] < rect.GetMin()[0]) ||
               (v1[0] > rect.GetMax()[0] && v2[0] > rect.GetMax()[0] && v3[0] > rect.GetMax()[0]) ||
               (v1[1] < rect.GetMin()[1] && v2[1] < rect.GetMin()[1] && v3[1] < rect.GetMin()[1]) ||
               (v1[1] > rect.GetMax()[1] && v2[1] > rect.GetMax()[1] && v3[1] > rect.GetMax()[1]);
    };

    bool intersect_rect_circle(const GfRange2f& rect, const GfVec2f& center, float radius)
    {
        const auto radius_sq = radius * radius;
        if ((rect.GetMin()[0] <= center[0] && center[0] <= rect.GetMax()[0]) || (rect.GetMin()[1] <= center[1] && center[1] <= rect.GetMax()[1]))
            return true;

        if (center[0] < rect.GetMin()[0])
        {
            if (center[1] < rect.GetMin()[1])
                return (rect.GetCorner(0) - center).GetLengthSq() <= radius_sq;
            if (center[1] > rect.GetMax()[1])
                return (rect.GetCorner(2) - center).GetLengthSq() <= radius_sq;
        }
        else if (center[0] > rect.GetMax()[0])
        {
            if (center[1] < rect.GetMin()[1])
                return (rect.GetCorner(1) - center).GetLengthSq() <= radius_sq;
            if (center[1] > rect.GetMax()[1])
                return (rect.GetCorner(3) - center).GetLengthSq() <= radius_sq;
        }
        return false;
    }
    GfRect2i get_bucket_min_max_ids(const GfRange2f& mesh_bbox, const GfVec2i& buckets_dim, const GfRange2f& target_rect)
    {
        GfVec2i bucket_min, bucket_max;
        // due to rounding errors, face_bbox_ss min value might be less than mesh_bbox_ss in
        // in that case we will get negative number for bucket id,
        // in order to workaround it, add 0.5 so it will truncates to 0th index
        bucket_min[0] = (int)((int)((target_rect.GetMin()[0] - mesh_bbox.GetMin()[0]) / mesh_bbox.GetSize()[0] * buckets_dim[0]) + 0.5f);
        bucket_min[1] = (int)((int)((target_rect.GetMin()[1] - mesh_bbox.GetMin()[1]) / mesh_bbox.GetSize()[1] * buckets_dim[1]) + 0.5f);

        bucket_max[0] = (int)((int)((target_rect.GetMax()[0] - mesh_bbox.GetMin()[0]) / mesh_bbox.GetSize()[0] * buckets_dim[0]) + 1.5f);
        bucket_max[1] = (int)((int)((target_rect.GetMax()[1] - mesh_bbox.GetMin()[1]) / mesh_bbox.GetSize()[1] * buckets_dim[1]) + 1.5f);

        bucket_min[0] = GfMin(GfMax(bucket_min[0], 0), buckets_dim[0]);
        bucket_min[1] = GfMin(GfMax(bucket_min[1], 0), buckets_dim[1]);
        bucket_max[0] = GfMin(GfMax(bucket_max[0], 0), buckets_dim[0]);
        bucket_max[1] = GfMin(GfMax(bucket_max[1], 0), buckets_dim[1]);
        return GfRect2i(bucket_min, bucket_max);
    }

    float dist_to_line_sq(const GfVec2f& p, const GfVec2f& l1, const GfVec2f& l2)
    {
        const auto l = l2 - l1;
        const auto closest = GfLine2d(GfVec2d(l1[0], l1[1]), GfVec2d(l[0], l[1])).FindClosestPoint(GfVec2d(p[0], p[1]));
        return (closest - GfVec2d(p[0], p[1])).GetLengthSq();
    }

    float triangle_area_times_2(const GfVec2f& v1, const GfVec2f& v2, const GfVec2f& v3)
    {
        return GfAbs(cross_2d(v2 - v1, v3 - v1));
    }
    bool is_point_inside_tri(const GfVec2f& v1, const GfVec2f& v2, const GfVec2f& v3, const GfVec2f& p)
    {
        return (triangle_area_times_2(p, v1, v2) + triangle_area_times_2(p, v2, v3) + triangle_area_times_2(p, v3, v1)) /
                   triangle_area_times_2(v1, v2, v3) <
               (1.0 + 0.001);
    }

    GfVec2f clamp_vec2(const GfVec2f& val, const GfVec2f& min, const GfVec2f& max)
    {
        return GfVec2f(GfClamp(val[0], min[0], max[0]), GfClamp(val[1], min[1], max[1]));
    }

    bool isect_line_y_axis(const GfVec2f& l1, const GfVec2f& l2, float axis, float& x_isect)
    {
        // touch first point
        if (GfIsClose(axis, l1[1], 0.00001))
        {
            x_isect = l1[0];
            return true;
        }
        // touch second point
        if (GfIsClose(axis, l2[1], 0.00001))
        {
            x_isect = l2[0];
            return true;
        }
        // horizontal line
        if (GfIsClose(l2[1], l1[1], 0.00001))
        {
            x_isect = (l1[0] + l2[0]) * 0.5f;
            return true;
        }

        if (l2[1] < axis && axis < l1[1])
        {
            x_isect = (l2[0] * (l1[1] - axis) + l1[0] * (axis - l2[1])) / (l1[1] - l2[1]);
            return true;
        }
        if (l1[1] < axis && axis < l2[1])
        {
            x_isect = (l2[0] * (axis - l1[1]) + l1[0] * (l2[1] - axis)) / (l2[1] - l1[1]);
            return true;
        }
        return false;
    }
    bool isect_line_x_axis(const GfVec2f& l1, const GfVec2f& l2, float axis, float& y_isect)
    {
        // touch first point
        if (GfIsClose(axis, l1[0], 0.00001))
        {
            y_isect = l1[1];
            return true;
        }
        // touch second point
        if (GfIsClose(axis, l2[0], 0.00001))
        {
            y_isect = l2[1];
            return true;
        }
        // vertical line
        if (GfIsClose(l2[0], l1[0], 0.00001))
        {
            y_isect = (l1[1] + l2[1]) * 0.5f;
            return true;
        }

        if (l2[0] < axis && axis < l1[0])
        {
            y_isect = (l2[1] * (l1[0] - axis) + l1[1] * (axis - l2[0])) / (l1[0] - l2[0]);
            return true;
        }
        if (l1[0] < axis && axis < l2[0])
        {
            y_isect = (l2[1] * (axis - l1[0]) + l1[1] * (l2[0] - axis)) / (l2[0] - l1[0]);
            return true;
        }
        return false;
    }
    bool clip_line(const GfRange2f& clip_rect, const GfRange2f& bucket_rect, const GfVec2f& l1, const GfVec2f& l2, GfVec2f& out_l1, GfVec2f& out_l2)
    {
        const auto pixel_eps = 0.01;
        const auto ver_length = GfAbs(l2[0] - l1[0]);
        const auto hor_length = GfAbs(l2[1] - l1[1]);

        // Check if line is Axis-aligned
        if (ver_length < pixel_eps)
        {
            if (l1[0] < bucket_rect.GetMin()[0] || l2[0] > bucket_rect.GetMax()[0])
                return false;
            if ((l1[1] < bucket_rect.GetMin()[1] && l2[1] < bucket_rect.GetMin()[1]) ||
                (l1[1] > bucket_rect.GetMax()[1] && l2[1] > bucket_rect.GetMax()[1]))
                return false;
            if (hor_length < pixel_eps)
            {
                if (bucket_rect.Contains(l1))
                {
                    out_l1 = l1;
                    out_l2 = l2;
                    return true;
                }
                return false;
            }
            out_l1 = GfVec2f(l1[0], GfClamp(l1[1], bucket_rect.GetMin()[1], bucket_rect.GetMax()[1]));
            out_l2 = GfVec2f(l2[0], GfClamp(l2[1], bucket_rect.GetMin()[1], bucket_rect.GetMax()[1]));
            return true;
        }
        if (hor_length < pixel_eps)
        {
            if (l1[1] < bucket_rect.GetMin()[1] || l2[1] > bucket_rect.GetMax()[1])
                return false;
            if ((l1[0] < bucket_rect.GetMin()[0] && l2[0] < bucket_rect.GetMin()[0]) ||
                (l1[0] > bucket_rect.GetMax()[0] && l2[0] > bucket_rect.GetMax()[0]))
                return false;
            if (ver_length < pixel_eps)
            {
                if (bucket_rect.Contains(l1))
                {
                    out_l1 = l1;
                    out_l2 = l2;
                    return true;
                }
                return false;
            }
            out_l1 = GfVec2f(l1[0], GfClamp(l1[1], bucket_rect.GetMin()[1], bucket_rect.GetMax()[1]));
            out_l2 = GfVec2f(l2[0], GfClamp(l2[1], bucket_rect.GetMin()[1], bucket_rect.GetMax()[1]));
            return true;
        }

        // todo use inside mask from clipping polyline routine
        bool l1_inside = bucket_rect.Contains(l1), l2_inside = bucket_rect.Contains(l2);
        if (l1_inside)
            out_l1 = l1;
        if (l2_inside)
            out_l2 = l2;
        if (l1_inside && l2_inside)
            return true;
        float isect;
        if (isect_line_y_axis(l1, l2, bucket_rect.GetMin()[1], isect) && (clip_rect.GetMin()[0] <= isect && isect <= clip_rect.GetMax()[0]))
        {
            if (l1[1] < l2[1])
            {
                out_l1[0] = isect;
                out_l1[1] = bucket_rect.GetMin()[1];
                l1_inside = true;
            }
            else
            {
                out_l2[0] = isect;
                out_l2[1] = bucket_rect.GetMin()[1];
                l2_inside = true;
            }
        }
        if (l1_inside && l2_inside)
            return true;

        if (isect_line_y_axis(l1, l2, bucket_rect.GetMax()[1], isect) && (clip_rect.GetMin()[0] <= isect && isect <= clip_rect.GetMax()[0]))
        {
            if (l1[1] > l2[1])
            {
                out_l1[0] = isect;
                out_l1[1] = bucket_rect.GetMax()[1];
                l1_inside = true;
            }
            else
            {
                out_l2[0] = isect;
                out_l2[1] = bucket_rect.GetMax()[1];
                l2_inside = true;
            }
        }

        if (l1_inside && l2_inside)
            return true;

        if (isect_line_x_axis(l1, l2, bucket_rect.GetMin()[0], isect) && (clip_rect.GetMin()[1] <= isect && isect <= clip_rect.GetMax()[1]))
        {
            if (l1[0] < l2[0])
            {
                out_l1[0] = bucket_rect.GetMin()[0];
                out_l1[1] = isect;
                l1_inside = true;
            }
            else
            {
                out_l2[0] = bucket_rect.GetMin()[0];
                out_l2[1] = isect;
                l2_inside = true;
            }
        }

        if (l1_inside && l2_inside)
            return true;

        if (isect_line_x_axis(l1, l2, bucket_rect.GetMax()[0], isect) && (clip_rect.GetMin()[1] <= isect && isect <= clip_rect.GetMax()[1]))
        {
            if (l1[0] > l2[0])
            {
                out_l1[0] = bucket_rect.GetMax()[0];
                out_l1[1] = isect;
                l1_inside = true;
            }
            else
            {
                out_l2[0] = bucket_rect.GetMax()[0];
                out_l2[1] = isect;
                l2_inside = true;
            }
        }

        return l1_inside && l2_inside;
    }

    // TODO: handle mesh flipping outside by swapping v1 and v3
    void init_clipping_polyline(const GfVec2f& v1_ss, const GfVec2f& v2_ss, const GfVec2f& v3_ss, const GfVec3f& tri_depths,
                                const GfVec3f& persp_weights, const GfVec2f& v1_uv, const GfVec2f& v2_uv, const GfVec2f& v3_uv,
                                const GfRange2f& clip_rect, const GfRange2f& bucket_rect, bool is_persp, bool backface_cull,
                                std::vector<GfVec2f>& clipping_polyline)
    {
        // TODO: reconsider using Liang-Barsky or Cohen-Sutherland algorithms for this routine
        clipping_polyline.clear();
        bool degenerate = false;
        // detect degenerate case: if area is close to zero
        // TODO: research it later
        // if (GfMin(dist_to_line_sq(v1_ss, v2_ss, v3_ss), GfMin(dist_to_line_sq(v2_ss, v3_ss, v1_ss), dist_to_line_sq(v3_ss, v1_ss, v2_ss))) < 0.01)
        //{
        //    degenerate = true;
        //}

        enum InsideFlags
        {
            OUTSIDE = 0,
            INSIDE_1 = 1,
            INSIDE_2 = 2,
            INSIDE_3 = 4,
            INSIDE_4 = 8,
            INSIDE_RECT = INSIDE_1 | INSIDE_2 | INSIDE_3,
            INSIDE_TRI = INSIDE_1 | INSIDE_2 | INSIDE_3 | INSIDE_4
        };
        uint8_t inside_rect_mask = InsideFlags::OUTSIDE;
        if (bucket_rect.Contains(v1_ss))
            inside_rect_mask |= INSIDE_1;
        if (bucket_rect.Contains(v2_ss))
            inside_rect_mask |= INSIDE_2;
        if (bucket_rect.Contains(v3_ss))
            inside_rect_mask |= INSIDE_3;

        if (inside_rect_mask == INSIDE_RECT)
        {
            bool flip_mesh = false;
            const bool flip = (left_of_line(v3_ss, v1_ss, v2_ss) != flip_mesh) != left_of_line(v3_uv, v1_uv, v2_uv);
            if (flip)
            {
                clipping_polyline.push_back(v3_uv);
                clipping_polyline.push_back(v2_uv);
                clipping_polyline.push_back(v1_uv);
            }
            else
            {
                clipping_polyline.push_back(v1_uv);
                clipping_polyline.push_back(v2_uv);
                clipping_polyline.push_back(v3_uv);
            }
            return;
        }

        if (degenerate)
            return;

        uint8_t inside_tri_mask = InsideFlags::OUTSIDE;
        if (is_point_inside_tri(v1_ss, v2_ss, v3_ss, bucket_rect.GetCorner(0)))
            inside_tri_mask = InsideFlags::INSIDE_1;
        if (is_point_inside_tri(v1_ss, v2_ss, v3_ss, bucket_rect.GetCorner(1)))
            inside_tri_mask |= InsideFlags::INSIDE_2;
        if (is_point_inside_tri(v1_ss, v2_ss, v3_ss, bucket_rect.GetCorner(3)))
            inside_tri_mask |= InsideFlags::INSIDE_3;
        if (is_point_inside_tri(v1_ss, v2_ss, v3_ss, bucket_rect.GetCorner(2)))
            inside_tri_mask |= InsideFlags::INSIDE_4;

        const bool flip = left_of_line(v3_ss, v1_ss, v2_ss) != left_of_line(v3_uv, v1_uv, v2_uv);
        std::array<int, 4> corner_indices = { 0, 1, 3, 2 };
        // bucket is completely inside of the triangle
        // possible micro optimization: we don't need to assign new corner vertex each time
        if (inside_tri_mask == INSIDE_TRI)
        {
            if (flip)
            {
                for (int i = 3; i >= 0; i--)
                {
                    auto bary_weights = to_bary_2d_persp_cor(v1_ss, v2_ss, v3_ss, persp_weights, bucket_rect.GetCorner(corner_indices[i]));
                    clipping_polyline.push_back(bary_interp(v1_uv, v2_uv, v3_uv, bary_weights));
                }
            }
            else
            {
                for (const auto i : corner_indices)
                {
                    auto bary_weights = to_bary_2d_persp_cor(v1_ss, v2_ss, v3_ss, persp_weights, bucket_rect.GetCorner(i));
                    clipping_polyline.push_back(bary_interp(v1_uv, v2_uv, v3_uv, bary_weights));
                }
            }
            return;
        }

        std::vector<std::pair<GfVec2f, float>> candidates;
        candidates.reserve(8);
        if (inside_tri_mask & InsideFlags::INSIDE_1)
            candidates.push_back({ bucket_rect.GetCorner(0), 0 });
        if (inside_tri_mask & InsideFlags::INSIDE_2)
            candidates.push_back({ bucket_rect.GetCorner(1), 0 });
        if (inside_tri_mask & InsideFlags::INSIDE_3)
            candidates.push_back({ bucket_rect.GetCorner(3), 0 });
        if (inside_tri_mask & InsideFlags::INSIDE_4)
            candidates.push_back({ bucket_rect.GetCorner(2), 0 });

        if (inside_rect_mask & InsideFlags::INSIDE_1)
            candidates.push_back({ v1_ss, 0 });
        if (inside_rect_mask & InsideFlags::INSIDE_2)
            candidates.push_back({ v2_ss, 0 });
        if (inside_rect_mask & InsideFlags::INSIDE_3)
            candidates.push_back({ v3_ss, 0 });

        // if v1 and v2 not both inside
        if ((inside_rect_mask & (InsideFlags::INSIDE_1 | InsideFlags::INSIDE_2)) != (InsideFlags::INSIDE_1 | InsideFlags::INSIDE_2))
        {
            GfVec2f v1_clipped, v2_clipped;
            if (clip_line(clip_rect, bucket_rect, v1_ss, v2_ss, v1_clipped, v2_clipped))
            {
                if ((inside_rect_mask & InsideFlags::INSIDE_1) == 0)
                    candidates.push_back({ v1_clipped, 0 });
                if ((inside_rect_mask & InsideFlags::INSIDE_2) == 0)
                    candidates.push_back({ v2_clipped, 0 });
            }
        }
        // if v2 and v3 not both inside
        if ((inside_rect_mask & (InsideFlags::INSIDE_2 | InsideFlags::INSIDE_3)) != (InsideFlags::INSIDE_2 | InsideFlags::INSIDE_3))
        {
            GfVec2f v1_clipped, v2_clipped;
            if (clip_line(clip_rect, bucket_rect, v2_ss, v3_ss, v1_clipped, v2_clipped))
            {
                if ((inside_rect_mask & InsideFlags::INSIDE_2) == 0)
                    candidates.push_back({ v1_clipped, 0 });
                if ((inside_rect_mask & InsideFlags::INSIDE_3) == 0)
                    candidates.push_back({ v2_clipped, 0 });
            }
        }
        // if v3 and v1 not both inside
        if ((inside_rect_mask & (InsideFlags::INSIDE_3 | InsideFlags::INSIDE_1)) != (InsideFlags::INSIDE_3 | InsideFlags::INSIDE_1))
        {
            GfVec2f v1_clipped, v2_clipped;
            if (clip_line(clip_rect, bucket_rect, v3_ss, v1_ss, v1_clipped, v2_clipped))
            {
                if ((inside_rect_mask & InsideFlags::INSIDE_3) == 0)
                    candidates.push_back({ v1_clipped, 0 });
                if ((inside_rect_mask & InsideFlags::INSIDE_1) == 0)
                    candidates.push_back({ v2_clipped, 0 });
            }
        }
        if (candidates.size() < 3)
        {
            return;
        }

        // sort points clockwise
        GfVec2f center(0, 0);
        for (int i = 0; i < candidates.size(); i++)
            center += candidates[i].first;
        center /= candidates.size();

        const GfVec2f t1(center[0], center[1] + 1);
        for (int i = 0; i < candidates.size(); i++)
        {
            GfVec2f t2 = candidates[i].first - center;
            candidates[i].second = std::atan2(cross_2d(t1, t2), GfDot(t1, t2));
        }
        if (flip)
        {
            std::sort(candidates.begin(), candidates.end(),
                      [](const std::pair<GfVec2f, float>& left, const std::pair<GfVec2f, float>& right) { return left.second > right.second; });
        }
        else
        {
            std::sort(candidates.begin(), candidates.end(),
                      [](const std::pair<GfVec2f, float>& left, const std::pair<GfVec2f, float>& right) { return left.second < right.second; });
        }

        // Don't need to remove vertices that are almost equal to each other.
        // In the worst case rarely we will have a few additional vertices more for polyline bbox evaluation
        // On the other side, there can be floating point errors that might clip whole triangle
        // that is perpendicular to view plane. It can results in visual artifacts when one pixel is not painted.

        // auto candidates_end = std::unique(candidates.begin(), candidates.end(), [](const std::pair<GfVec2f, float>& left, const std::pair<GfVec2f,
        // float>& right)
        //{
        //        return GfIsClose(left.first[0], right.first[0], 0.00001) && GfIsClose(left.first[1], right.first[1], 0.00001);
        //});
        // if (std::distance(candidates.begin(), candidates_end) < 3)
        //    return;

        for (auto iter = candidates.begin(); iter != candidates.end(); ++iter)
        {
            auto bary_weights = to_bary_2d_persp_cor(v1_ss, v2_ss, v3_ss, persp_weights, iter->first);
            clipping_polyline.push_back(bary_interp(v1_uv, v2_uv, v3_uv, bary_weights));
        }
    }

    GfRange2f get_bucket_rect(const GfRange2f& mesh_bbox, const GfVec2i& bucket_dims, const GfVec2i& bucket)
    {
        GfVec2f bucket_bbox_min(mesh_bbox.GetMin()[0] + bucket[0] * (mesh_bbox.GetSize()[0] / bucket_dims[0]),
                                mesh_bbox.GetMin()[1] + bucket[1] * (mesh_bbox.GetSize()[1] / bucket_dims[1]));
        GfVec2f bucket_bbox_max(mesh_bbox.GetMin()[0] + (bucket[0] + 1) * (mesh_bbox.GetSize()[0] / bucket_dims[0]),
                                mesh_bbox.GetMin()[1] + (bucket[1] + 1) * (mesh_bbox.GetSize()[1] / bucket_dims[1]));
        return GfRange2f(bucket_bbox_min, bucket_bbox_max);
    }

    bool is_inside_polyline(const std::vector<GfVec2f>& polyline, const GfVec2f& point)
    {
        if (!left_of_line(point, polyline.back(), polyline.front()))
            return false;
        for (int i = 1; i < polyline.size(); i++)
        {
            if (!left_of_line(point, polyline[i - 1], polyline[i]))
                return false;
        }
        return true;
    }
    bool is_inside_polyline_twoside(const std::vector<GfVec2f>& polyline, const GfVec2f& point)
    {
        const bool side = left_of_line(point, polyline.back(), polyline.front());
        for (int i = 1; i < polyline.size(); i++)
        {
            if (left_of_line(point, polyline[i - 1], polyline[i]) != side)
                return false;
        }
        return true;
    }
    bool is_occluded(const GfVec4f& v1_ss, const GfVec4f& v2_ss, const GfVec4f& v3_ss, const GfVec3f& p_ss)
    {
        if (v1_ss[2] > p_ss[2] && v2_ss[2] > p_ss[2] && v3_ss[2] > p_ss[2])
            return false;
        GfVec2f v1(v1_ss[0], v1_ss[1]), v2(v2_ss[0], v2_ss[1]), v3(v3_ss[0], v3_ss[1]), p(p_ss[0], p_ss[1]);
        if (!intersect_point_triangle(p, v1, v2, v3))
            return false;

        if (v1_ss[2] < p_ss[2] && v2_ss[2] < p_ss[2] && v3_ss[2] < p_ss[2])
            return true;

        const auto persp_weights = GfVec3f(v1_ss[3], v2_ss[3], v3_ss[3]);
        auto persp_cor_bary = to_bary_2d_persp_cor(v1, v2, v3, persp_weights, p);
        persp_cor_bary[0] *= persp_weights[0];
        persp_cor_bary[1] *= persp_weights[1];
        persp_cor_bary[2] *= persp_weights[2];
        const auto sum_weight = persp_cor_bary[0] + persp_cor_bary[1] + persp_cor_bary[2];
        if (sum_weight != 0)
            persp_cor_bary /= sum_weight;
        else
            persp_cor_bary = GfVec3f(1 / 3.0);
        return (v1_ss[2] * persp_cor_bary[0] + v2_ss[2] * persp_cor_bary[1] + v3_ss[2] * persp_cor_bary[2]) < p_ss[2];
    }
};

OPENDCC_NAMESPACE_CLOSE
