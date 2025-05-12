// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>

#include "opendcc/anim_engine/curve/curve.h"
#include <iostream>
#include <cmath>
#include <limits>

OPENDCC_NAMESPACE_OPEN

using namespace adsk;

namespace
{
    static const float infinity = std::numeric_limits<float>::infinity();
    static const float epsilon = std::numeric_limits<float>::epsilon();
    static const double max_tangent = 5729577.9485111479f;
    static const double clamped_tolerance = 0.05f;
}

template <typename T>
T clamp(const T value, const T min_value, const T max_value)
{
    return std::max(std::min(value, max_value), min_value);
}

void AnimCurve::clump_spline(const Keyframe& prev, const Keyframe& key, const Keyframe& next, bool hasPrev, bool hasNext, double& in_tangent_x,
                             double& in_tangent_y, double& out_tangent_x, double& out_tangent_y)
{
    TangentType in_tangent = TangentType::Clamped;
    TangentType out_tangent = TangentType::Clamped;
    if ((key.tanIn.type == TangentType::Clamped) && hasPrev)
    {
        auto py = prev.value - key.value;
        if (py < 0.0)
            py = -py;
        auto ny = (!hasNext ? py : next.value - key.value);
        if (ny < 0.0)
            ny = -ny;
        if ((ny <= 0.05) || (py <= 0.05))
        {
            in_tangent = TangentType::Flat;
        }
    }

    if ((key.tanOut.type == TangentType::Clamped) && (hasNext))
    {
        auto ny = next.value - key.value;
        if (ny < 0.0)
            ny = -ny;
        auto py = (!hasPrev ? ny : prev.value - key.value);
        if (py < 0.0)
            py = -py;
        if ((ny <= 0.05) || (py <= 0.05))
        {
            out_tangent = TangentType::Flat;
        }
    }

    if (in_tangent == TangentType::Flat || (!hasPrev && out_tangent == TangentType::Flat))
    {
        if (!hasPrev)
        {
            in_tangent_x = hasNext ? next.time - key.time : 0;
            in_tangent_y = 0;
        }
        else
        {
            in_tangent_x = key.time - prev.time;
            in_tangent_y = 0;
        }
    }

    if (out_tangent == TangentType::Flat || (!hasNext && in_tangent == TangentType::Flat))
    {
        if (!hasNext)
        {
            out_tangent_x = hasPrev ? key.time - prev.time : 0;
            out_tangent_y = 0;
        }
        else
        {
            out_tangent_x = next.time - key.time;
            out_tangent_y = 0;
        }
    }
}

template <typename T>
int signNoZero(T val)
{
    return (T(0) < val) ? -1 : 1;
}

void AnimCurve::clump_spline_auto(const Keyframe& prev, const Keyframe& key, const Keyframe& next, bool hasPrev, bool hasNext, double& in_tangent_x,
                                  double& in_tangent_y, double& out_tangent_x, double& out_tangent_y)
{
    assert((key.tanOut.type == TangentType::Auto) || (key.tanIn.type == TangentType::Auto));
    if ((!hasPrev || !hasNext))
    {
        if (key.tanIn.type == TangentType::Auto)
        {
            in_tangent_x = 1;
            in_tangent_y = 0;
        }
        if (key.tanOut.type == TangentType::Auto)
        {
            out_tangent_x = 1;
            out_tangent_y = 0;
        }
        return;
    }

    const double x = key.time;
    const double xp = prev.time;
    const double xn = next.time;
    const double y = key.value;
    const double yn = next.value;
    const double yp = prev.value;

    const double source_tangent = (key.tanIn.type == TangentType::Auto) ? in_tangent_y / in_tangent_x : out_tangent_y / out_tangent_x;

    const double in_tg_limit = 3 * (y - yp) / (x - xp);
    const double out_tg_limit = 3 * (yn - y) / (xn - x);

    double tg;

    if ((yp <= y) && (y < yn))
    {
        const double in_tg_clumped = clamp<double>(source_tangent, 0, in_tg_limit);
        const double out_tg_clumped = clamp<double>(source_tangent, 0, out_tg_limit);
        tg = std::min(in_tg_clumped, out_tg_clumped);
    }
    else if ((yp > y) && (y > yn))
    {
        const double in_tg_clumped = clamp<double>(source_tangent, in_tg_limit, 0);
        const double out_tg_clumped = clamp<double>(source_tangent, out_tg_limit, 0);
        tg = std::max(in_tg_clumped, out_tg_clumped);
    }
    else
    {
        tg = 0;
    }
    if (key.tanIn.type == TangentType::Auto)
    {
        in_tangent_x = 1;
        in_tangent_y = tg;
    }

    if (key.tanOut.type == TangentType::Auto)
    {
        out_tangent_x = 1;
        out_tangent_y = tg;
    }
}

void AnimCurve::clump_spline_plateau(const Keyframe& prev, const Keyframe& key, const Keyframe& next, bool hasPrev, bool hasNext,
                                     double& in_tangent_x, double& in_tangent_y, double& out_tangent_x, double& out_tangent_y)
{
    TangentType in_tangent;
    TangentType out_tangent;
    if (key.tanIn.type == TangentType::Plateau)
    {

        if (!hasPrev || !hasNext)
        {
            /* No previous or next key, first and last keys are always flat */
            in_tangent = TangentType::Flat;
        }
        else
        {
            /* Compute control point (tangent end) Y position because if this point goes beyond the
               previous Y key position then the tangent must be readjusted                           */
            auto py = prev.value - key.value;
            auto ny = next.value - key.value;

            if (py * ny >= 0.0)
            {
                /* both py and ny have the same sign, key is a maxima or a minima */
                in_tangent = TangentType::Flat;
            }
            else
            {
                /*
                Compute the smooth tangent control point (tangent end) Y position(cpy) because if
                this point goes beyond the previous Y or next -Y key position then the tangent
                must be readjusted/clamped
                */

                auto cpy = (py - ny) * (key.time - prev.time) / (3 * (next.time - prev.time));

                /*
                Check if the slope to the next key is more gentle than the slope to
                previous key
                */
                if ((-ny / py) < ((next.time - key.time) / (key.time - prev.time)))
                {
                    /* Adjust previous key value to match the slope of the out tangent */
                    py = -ny * (key.time - prev.time) / (next.time - key.time);
                }

                /*
                Clamp the computed smooth tangent control point value if it goes above the absolute
                previous key value
                */
                if ((py >= 0.0 && cpy > py) || (py <= 0.0 && cpy < py))
                {
                    if (key.tanIn.type == TangentType::Plateau)
                    {
                        in_tangent = TangentType::Flat;
                    }
                    else /*(key.tanIn.type == kTangentAuto)*/
                    {
                        in_tangent_y = -py * 3.0;
                        in_tangent_x = key.time - prev.time;
                    }
                }
                else
                {
                    /* by default these tangents behave as smooth tangents */
                    in_tangent = TangentType::Smooth;
                }
            }
        }
    }

    if (key.tanOut.type == TangentType::Plateau)
    {
        if (!hasPrev || !hasNext)
        {
            /* First and last keys are always flat */
            out_tangent = TangentType::Flat;
        }
        else
        {
            auto py = prev.value - key.value;
            auto ny = next.value - key.value;

            if (py * ny >= 0.0)
            {
                /* both py and ny have the same sign, key is a maxima or a minima */
                out_tangent = TangentType::Flat;
            }
            else
            {
                /*
                Compute the smooth tangent control point (tangent end) Y position(cpy) because if
                this point goes beyond the previous Y or next -Y key position then the tangent
                must be readjusted/clamped
                */

                auto cpy = (ny - py) * (next.time - key.time) / (3 * (next.time - prev.time));

                if ((-py / ny) < ((key.time - prev.time) / (next.time - key.time)))
                {
                    /* Adjust next key value to match the slope of the in tangent */
                    ny = -py * (next.time - key.time) / (key.time - prev.time);
                }

                if ((ny >= 0.0 && cpy > ny) || (ny <= 0.0 && cpy < ny))
                {
                    if (key.tanOut.type == TangentType::Plateau)
                    {
                        out_tangent = TangentType::Flat;
                    }
                    else /*(key.tanOut.type == kTangentAuto)*/
                    {
                        out_tangent_y = ny * 3.0;
                        out_tangent_x = next.time - key.time;
                    }
                }
                else
                {
                    /* by default these tangents behave as smooth tangents */
                    out_tangent = TangentType::Smooth;
                }
            }
        }
    }

    if (in_tangent == TangentType::Flat)
    {
        if (!hasPrev)
        {
            in_tangent_x = hasNext ? next.time - key.time : 0;
            in_tangent_y = 0;
        }
        else
        {
            in_tangent_x = key.time - prev.time;
            in_tangent_y = 0;
        }
    }

    if (out_tangent == TangentType::Flat)
    {
        if (!hasNext)
        {
            out_tangent_x = hasPrev ? key.time - prev.time : 0;
            out_tangent_y = 0;
        }
        else
        {
            out_tangent_x = next.time - key.time;
            out_tangent_y = 0;
        }
    }

    if (in_tangent == TangentType::Smooth)
    { /*do nothing because already calculated in compute_tangents for this case*/
    }

    if (out_tangent == TangentType::Smooth)
    { /*do nothing because already calculated in compute_tangents for this case*/
    }
}

void AnimCurve::compute_tangent(int index)
{
    Keyframe& key = m_sorted_keys[index];
    bool hasPrev = index > 0;
    bool hasNext = index < m_sorted_keys.size() - 1;
    Keyframe prev, next;
    if (hasPrev)
    {
        prev = m_sorted_keys[index - 1];
    }
    if (hasNext)
    {
        next = m_sorted_keys[index + 1];
    }

    const TangentType in_tangent = key.tanIn.type;
    const TangentType out_tangent = key.tanOut.type;

    double in_tangent_x = infinity;
    double in_tangent_y = infinity;
    double out_tangent_x = infinity;
    double out_tangent_y = infinity;

    bool compute_spline = false;

    switch (in_tangent)
    {
    case TangentType::Fixed:
        if (key.tanIn.x == 0)
        {
            in_tangent_x = 1;
            in_tangent_y = 0;
        }
        else
        {
            in_tangent_x = key.tanIn.x;
            in_tangent_y = key.tanIn.y;
        }
        break;
    case TangentType::Linear:
        if (!hasPrev)
        {
            in_tangent_x = 1.0f;
            in_tangent_y = 0;
        }
        else
        {
            in_tangent_x = key.time - prev.time;
            in_tangent_y = key.value - prev.value;
        }
        break;
    case TangentType::Flat:
        if (!hasPrev)
        {
            in_tangent_x = hasNext ? next.time - key.time : 0;
            in_tangent_y = 0;
        }
        else
        {
            in_tangent_x = key.time - prev.time;
            in_tangent_y = 0;
        }
        break;
    case TangentType::Step:
        in_tangent_x = in_tangent_y = 0;
        break;
    case TangentType::StepNext:
        in_tangent_x = in_tangent_y = 0;
        break;
    case TangentType::Plateau:
    case TangentType::Clamped:
    case TangentType::Smooth:
    case TangentType::Auto:
        compute_spline = true;
        break;
    }

    switch (out_tangent)
    {
    case TangentType::Fixed:
        if (key.tanOut.x == 0)
        {
            out_tangent_x = 1;
            out_tangent_y = 0;
        }
        else
        {
            out_tangent_x = key.tanOut.x;
            out_tangent_y = key.tanOut.y;
        }
        break;
    case TangentType::Linear:
        if (!hasNext)
        {
            out_tangent_x = 1.0f;
            out_tangent_y = 0;
        }
        else
        {
            out_tangent_x = next.time - key.time;
            out_tangent_y = next.value - key.value;
        }
        break;
    case TangentType::Flat:
        if (!hasNext)
        {
            out_tangent_x = hasPrev ? key.time - prev.time : 0;
            out_tangent_y = 0;
        }
        else
        {
            out_tangent_x = next.time - key.time;
            out_tangent_y = 0;
        }
        break;
    case TangentType::Step:
        out_tangent_x = out_tangent_y = 0;
        break;
    case TangentType::StepNext:
        out_tangent_x = out_tangent_y = 0;
        break;
    case TangentType::Plateau:
    case TangentType::Clamped:
    case TangentType::Smooth:
    case TangentType::Auto:
        compute_spline = true;
        break;
    }

    if (compute_spline)
    {
        double in_tangent_xs = 1.0f;
        double in_tangent_ys = 0;
        double out_tangent_xs = 1.0f;
        double out_tangent_ys = 0;

        if (!hasPrev && hasNext)
        {
            out_tangent_xs = next.time - key.time;
            out_tangent_ys = next.value - key.value;
            in_tangent_xs = out_tangent_xs;
            in_tangent_ys = out_tangent_ys;
        }
        else if (hasPrev && !hasNext)
        {
            out_tangent_xs = key.time - prev.time;
            out_tangent_ys = key.value - prev.value;
            in_tangent_xs = out_tangent_xs;
            in_tangent_ys = out_tangent_ys;
        }
        else if (hasPrev && hasNext)
        {
            const auto dx = next.time - prev.time;

            if (dx < epsilon)
            {
                out_tangent_ys = max_tangent;
            }
            else
            {
                out_tangent_ys = (next.value - prev.value) / dx;
            }

            out_tangent_xs = next.time - key.time;
            in_tangent_xs = key.time - prev.time;
            in_tangent_ys = out_tangent_ys * in_tangent_xs;
            out_tangent_ys *= out_tangent_xs;
        }

        if (in_tangent == TangentType::Smooth || in_tangent == TangentType::Auto || in_tangent == TangentType::Plateau ||
            in_tangent == TangentType::Clamped)
        {
            in_tangent_x = out_tangent_xs;
            in_tangent_y = out_tangent_ys;
        }

        if (out_tangent == TangentType::Smooth || out_tangent == TangentType::Auto || out_tangent == TangentType::Plateau ||
            out_tangent == TangentType::Clamped)
        {
            out_tangent_x = out_tangent_xs;
            out_tangent_y = out_tangent_ys;
        }
    }

    if (in_tangent == TangentType::Auto || out_tangent == TangentType::Auto)
    {
        clump_spline_auto(prev, key, next, hasPrev, hasNext, in_tangent_x, in_tangent_y, out_tangent_x, out_tangent_y);
    }

    if (in_tangent == TangentType::Plateau || out_tangent == TangentType::Plateau)
    {
        clump_spline_plateau(prev, key, next, hasPrev, hasNext, in_tangent_x, in_tangent_y, out_tangent_x, out_tangent_y);
    }

    if (in_tangent == TangentType::Clamped || out_tangent == TangentType::Clamped)
    {
        clump_spline(prev, key, next, hasPrev, hasNext, in_tangent_x, in_tangent_y, out_tangent_x, out_tangent_y);
    }

    // Normalize tangents
    if (!(in_tangent_x == infinity && in_tangent_y == infinity) && !(out_tangent_x == infinity && out_tangent_y == infinity))
    {
        double len = std::sqrt(in_tangent_x * in_tangent_x + in_tangent_y * in_tangent_y);

        if (len != 0)
        {
            len = 1.0f / len;

            in_tangent_x *= len;
            in_tangent_y *= len;
        }

        len = std::sqrt(out_tangent_x * out_tangent_x + out_tangent_y * out_tangent_y);

        if (len != 0)
        {
            len = 1.0f / len;

            out_tangent_x *= len;
            out_tangent_y *= len;
        }
    }
    assert(in_tangent_x != infinity);
    assert(in_tangent_y != infinity);
    assert(out_tangent_x != infinity);
    assert(out_tangent_y != infinity);

    key.tanIn.x = in_tangent_x;
    key.tanIn.y = in_tangent_y;

    key.tanOut.x = out_tangent_x;
    key.tanOut.y = out_tangent_y;
}

void AnimCurve::compute_tangents()
{
    // !!!!!!!!
    std::stable_sort(m_sorted_keys.begin(), m_sorted_keys.end(), [](const Keyframe& k0, const Keyframe& k1) { return k0.time < k1.time; });

    for (int index = 0; index < m_sorted_keys.size(); ++index)
    {
        m_sorted_keys[index].index = index;
    }

    // end !!!!!!!
    for (int index = 0; index < m_sorted_keys.size(); ++index)
    {
#ifdef USE_ANIMX_AUTO_TANGENT
        auto& key = m_sorted_keys[index];
        // for auto tangents, calculate the value on the fly based on the previous and the next key
        if (key.tanIn.type == TangentType::Auto || key.tanOut.type == TangentType::Auto)
        {
            bool hasPrev = index > 0;
            bool hasNext = index < m_sorted_keys.size() - 1;
            Keyframe prev, next;
            if (hasPrev)
            {
                prev = m_sorted_keys[index - 1];
            }
            if (hasNext)
            {
                next = m_sorted_keys[index + 1];
            }

            CurveInterpolatorMethod interp = key.curveInterpolationMethod(isWeighted());

            if (key.tanIn.type == TangentType::Auto)
                autoTangent(true, key, hasPrev ? &prev : nullptr, hasNext ? &next : nullptr, interp, key.tanIn.x, key.tanIn.y);

            if (key.tanOut.type == TangentType::Auto)
                autoTangent(false, key, hasPrev ? &prev : nullptr, hasNext ? &next : nullptr, interp, key.tanOut.x, key.tanOut.y);

            if (key.tanOut.type == TangentType::Linear && hasNext && m_sorted_keys[index + 1].tanIn.type == TangentType::Linear)
            {
                key.linearInterpolation = true;
            }

            if (key.tanIn.type == TangentType::Linear && hasPrev && m_sorted_keys[index - 1].tanOut.type == TangentType::Linear)
            {
                key.linearInterpolation = true;
            }
        }
#else
        compute_tangent(index);
#endif
    }
}

bool AnimCurve::keyframeAtIndex(int index, Keyframe& key) const
{
    if (index >= 0 && index < m_sorted_keys.size())
    {
        key = m_sorted_keys[index];

        return true;
    }
    else
        return false;
};

bool AnimCurve::keyframe(double time, Keyframe& key) const
{
    if (m_sorted_keys.size() == 0)
        return false;

    auto it = std::lower_bound(m_sorted_keys.begin(), m_sorted_keys.end(), time, [](const Keyframe& key, double time) { return key.time < time; });

    if (it == m_sorted_keys.end())
        key = m_sorted_keys.back();
    else
        key = *it;

    return true;
}

adsk::KeyId AnimCurve::add_key(const Keyframe& key, bool reset_id)
{
    auto insert_key = key;
    if (reset_id)
        insert_key.id = generate_unique_key_id();

    if (m_sorted_keys.size() == 0)
    {
        insert_key.index = 0;
        m_sorted_keys.push_back(insert_key);
        return insert_key.id;
    }

    auto it = std::lower_bound(m_sorted_keys.begin(), m_sorted_keys.end(), insert_key.time,
                               [](const Keyframe& key, double time) { return key.time < time; });
    if (it == m_sorted_keys.begin())
    {
        insert_key.index = m_sorted_keys.size();
        m_sorted_keys.push_back(insert_key);
    }
    else
    {
        insert_key.index = it - m_sorted_keys.begin();
        m_sorted_keys.insert(it, insert_key);
    }
    compute_tangents();

    return insert_key.id;
}

adsk::KeyId AnimCurve::add_key(double time, double value)
{
    Keyframe key;
    key.time = time;
    key.value = value;
    key.tanIn = { TangentType::Auto, 1, 0 };
    key.tanOut = { TangentType::Auto, 1, 0 };
    key.linearInterpolation = false;
    key.quaternionW = 1.0;
    return add_key(key);
}

void AnimCurve::remove_keys_by_ids(const std::set<adsk::KeyId>& keys_ids)
{
    if (keys_ids.size() == 0)
        return;
    auto map = compute_id_to_idx_map();

    size_t j = 0;
    for (size_t i = 0; i < m_sorted_keys.size(); ++i)
    {
        if (keys_ids.find(m_sorted_keys[i].id) == keys_ids.cend())
        {
            m_sorted_keys[j] = m_sorted_keys[i];
            ++j;
        }
    }
    m_sorted_keys.resize(j);

    compute_tangents();
}

bool AnimCurve::remove_key(int index)
{
    if (index >= 0 && index < m_sorted_keys.size())
    {
        m_sorted_keys.erase(m_sorted_keys.begin() + index);
        return true;
    }
    else
        return false;
}

double AnimCurve::evaluate(double time) const
{
    return evaluateCurve(time, *this);
}

std::map<adsk::KeyId, size_t> AnimCurve::compute_id_to_idx_map() const
{
    std::map<adsk::KeyId, size_t> result;
    for (size_t i = 0; i < m_sorted_keys.size(); ++i)
        result[m_sorted_keys[i].id] = i;
    return result;
}

adsk::KeyId AnimCurve::generate_unique_key_id()
{
    static std::atomic<uint64_t> current_key_id;
    return adsk::KeyId(current_key_id.fetch_add(1));
}

void AnimCurve::clear()
{
    m_sorted_keys.clear();
    m_pre_infinity = InfinityType::Constant;
    m_post_infinity = InfinityType::Constant;
}

OPENDCC_NAMESPACE_CLOSE
