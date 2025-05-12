// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <random>
#include <vector>
#include <pxr/pxr.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/transform.h>
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/vec3f.h>
#include "pxr/usd/usdGeom/mesh.h"
#include "opendcc/usd_editor/sculpt_tool/sculpt_functions.h"
#include "opendcc/usd_editor/sculpt_tool/mesh_manipulation_data.h"
#include "opendcc/usd_editor/sculpt_tool/utils.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    static const float strength_scale = 0.3f;

    float falloff_function(float falloff, float normalize_radius)
    {
        if (falloff < 0.05)
            return 1.0;
        else if (falloff > 0.51)
            return (1 - normalize_radius) * std::exp(-(falloff - 0.5f) * 10 * normalize_radius);
        else if (falloff < 0.49)
            return (1 - normalize_radius * normalize_radius) * (1 - std::exp((falloff - 0.5f) * 30 * (1 - normalize_radius)));
        else
            return 1 - normalize_radius;
    };

    uint32_t hash(uint32_t x)
    {
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        return x;
    }

    float noise(uint32_t x)
    {
        return (float)(hash(x)) / (float)std::numeric_limits<uint32_t>::max();
    }

    VtVec3fArray smooth(const SculptIn& in, const std::vector<int>& indices, bool relax)
    {
        if (in.properties.strength <= 0)
            return VtVec3fArray();

        VtVec3fArray prev_points;
        in.mesh_data->mesh.GetPointsAttr().Get(&prev_points);
        const VtVec3fArray& normals = in.mesh_data->initial_world_normals;
        const VtIntArray& adjacency_table = in.mesh_data->adjacency.GetAdjacencyTable();
        auto inv_r = 1.0f / in.properties.radius;
        VtVec3fArray result = prev_points;

        float scaled_strengeth = in.properties.strength;
        size_t num_cycles = 1;
        if (in.properties.strength > 1)
        {
            num_cycles = (int)in.properties.strength;
            scaled_strengeth /= num_cycles;
        }

        const GfVec3f* points_data = in.mesh_data->initial_world_points.cdata();
        const GfVec3f* normals_data = normals.cdata();
        // do smooth in local space
        for (int cycle = 0; cycle < num_cycles; ++cycle)
        {
            const GfVec3f* prev_points_data = prev_points.cdata();
            GfVec3f* result_data = result.data();

            PARALLEL_FOR(i, 0, indices.size())
            {
                auto idx = indices[i];

                auto falloff = scaled_strengeth * falloff_function(in.properties.falloff, (points_data[idx] - in.hit_point).GetLength() * inv_r);

                int offsetIdx = idx * 2;
                int offset = adjacency_table[offsetIdx];
                int valence = adjacency_table[offsetIdx + 1];
                const int* e = &adjacency_table[offset];
                GfVec3f S(prev_points_data[idx]);
                for (int j = 0; j < valence; ++j)
                    S += prev_points_data[e[j * 2]];
                if (relax)
                {
                    GfVec3f delta = falloff * (S / (valence + 1) - prev_points_data[idx]);
                    result_data[idx] = prev_points_data[idx] + delta - normals_data[idx] * GfDot(delta, normals_data[idx]);
                }
                else
                {
                    result_data[idx] = prev_points_data[idx] * (1 - falloff) + falloff * S / (valence + 1);
                }
            }
            END_FOR;
            prev_points = result;
        }

        return result;
    }
}

VtVec3fArray sculpt(const SculptIn& in, const VtVec3fArray& prev_points, const std::vector<int>& indices)
{
    if (indices.size() == 0)
        return prev_points;

    if (in.properties.mode == Mode::Relax)
    {
        return smooth(in, indices, true);
    }
    else if (in.properties.mode == Mode::Smooth)
    {
        return smooth(in, indices, false);
    }

    const VtVec3fArray& initial_world_points = in.mesh_data->initial_world_points;
    const VtVec3fArray& initial_world_normals = in.mesh_data->initial_world_normals;
    VtVec3fArray next_values_vec3f = prev_points;

    const UsdTimeCode time = UsdTimeCode::Default();
    UsdGeomXformCache xform_cache(time);
    const GfMatrix4d local2world = xform_cache.GetLocalToWorldTransform(in.mesh_data->mesh.GetPrim());
    const GfMatrix4d world2local = local2world.GetInverse();

    std::vector<int> filtered_indices;
    GfVec3f mean_normal(0.0f);
    for (size_t i : indices)
    {
        if (GfDot(initial_world_normals[i], in.hit_normal) > 0)
        {
            mean_normal += initial_world_normals[i];
            filtered_indices.push_back(i);
        }
    }
    mean_normal.Normalize();

    float direction_length = in.direction.GetLength();
    const float max_direction_gap = in.properties.radius * 0.2f;
    if (direction_length > max_direction_gap)
        direction_length = max_direction_gap;

    GfVec3f mean_point(0.0f);
    if (in.properties.mode == Mode::Flatten)
    {
        for (auto i : filtered_indices)
            mean_point += initial_world_points[i];
    }
    mean_point /= filtered_indices.size();

    float total_strength = strength_scale * in.properties.strength * direction_length;
    if (in.inverts)
        total_strength = -total_strength;

    GfVec3f* next_values_vec3f_data = next_values_vec3f.data();
    const GfVec3f* prev_points_data = prev_points.cdata();

    const float inv_r = 1.0f / in.properties.radius;

    PARALLEL_FOR(i, 0, filtered_indices.size())
    {
        const auto idx = filtered_indices[i];
        float falloff = total_strength * falloff_function(in.properties.falloff, (initial_world_points[idx] - in.hit_point).GetLength() * inv_r);
        const auto point = GfVec3f(local2world.Transform(prev_points_data[idx]));
        GfVec3f next_point;

        if (in.properties.mode == Mode::Flatten)
        {
            GfVec3f dellta = point - mean_point;
            if (falloff > 1)
                falloff = 1;
            next_point = point - falloff * mean_normal * GfDot(dellta, mean_normal);
        }
        else if (in.properties.mode == Mode::Sculpt)
        {
            next_point = point + falloff * mean_normal;
        }
        else if (in.properties.mode == Mode::Move)
        {
            next_point = point + falloff * in.direction;
        }
        else if (in.properties.mode == Mode::Noise)
        {
            next_point = point + noise(idx) * falloff * mean_normal;
        }
        next_values_vec3f_data[idx] = GfVec3f(world2local.Transform(next_point));
    }
    END_FOR;

    return next_values_vec3f;
}

OPENDCC_NAMESPACE_CLOSE
