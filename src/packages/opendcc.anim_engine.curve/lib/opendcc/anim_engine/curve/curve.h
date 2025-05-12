/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <stdint.h>
#include <assert.h>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <atomic>
#include <memory>

#include "opendcc/vendor/animx/animx.h"

#define ANIM_CURVES_CHECK_AND_RETURN(cond)        \
    if (!(cond))                                  \
    {                                             \
        OPENDCC_ERROR("Coding error: {}", #cond); \
        return;                                   \
    };
#define ANIM_CURVES_CHECK_AND_CONTINUE(cond)      \
    if (!(cond))                                  \
    {                                             \
        OPENDCC_ERROR("Coding error: {}", #cond); \
        continue;                                 \
    };
#define ANIM_CURVES_CHECK_AND_RETURN_VAL(cond, val) \
    if (!(cond))                                    \
    {                                               \
        OPENDCC_ERROR("Coding error: {}", #cond);   \
        return (val);                               \
    };

OPENDCC_NAMESPACE_OPEN

class AnimCurve : public adsk::ICurve
{
public:
    // ICurve methods
    virtual bool keyframeAtIndex(int index, adsk::Keyframe& key) const override;
    virtual bool keyframe(double time, adsk::Keyframe& key) const override;

    virtual bool first(adsk::Keyframe& key) const override { return keyframeAtIndex(0, key); }
    virtual bool last(adsk::Keyframe& key) const override
    {
        if (m_sorted_keys.size() > 0)
        {
            key = m_sorted_keys.back();
            return true;
        }
        else
            return false;
    }

    virtual adsk::InfinityType preInfinityType() const override { return m_pre_infinity; }
    virtual adsk::InfinityType postInfinityType() const override { return m_post_infinity; }
    // !!!! temporary
    virtual bool isWeighted() const override { return false; }

    virtual unsigned int keyframeCount() const override { return (unsigned int)m_sorted_keys.size(); }
    // !!!! may degrade performance slightly at static curves
    virtual bool isStatic() const override { return false; }
    // additional methods

    void set_pre_infinity_type(const adsk::InfinityType& infinity_type) { m_pre_infinity = infinity_type; }
    void set_post_infinity_type(const adsk::InfinityType& infinity_type) { m_post_infinity = infinity_type; }

    adsk::InfinityType pre_infinity_type() const { return m_pre_infinity; }
    adsk::InfinityType post_infinity_type() const { return m_post_infinity; }

    adsk::KeyId add_key(const adsk::Keyframe& key, bool reset_id = true);
    adsk::KeyId add_key(double time, double value);
    void remove_keys_by_ids(const std::set<adsk::KeyId>& keys_ids);

    bool remove_key(int index);

    double evaluate(double time) const;

    const adsk::Keyframe& operator[](size_t index) const { return m_sorted_keys[index]; }
    const adsk::Keyframe& at(size_t index) const { return m_sorted_keys.at(index); }

    adsk::Keyframe& operator[](size_t index) { return m_sorted_keys[index]; }
    adsk::Keyframe& at(size_t index) { return m_sorted_keys.at(index); }
    std::map<adsk::KeyId, size_t> compute_id_to_idx_map() const;
    static adsk::KeyId generate_unique_key_id();
    void clear();
    void compute_tangents();

private:
    void clump_spline(const adsk::Keyframe& prev, const adsk::Keyframe& key, const adsk::Keyframe& next, bool hasPrev, bool hasNext,
                      double& in_tangent_x, double& in_tangent_y, double& out_tangent_x, double& out_tangent_y);
    void clump_spline_auto(const adsk::Keyframe& prev, const adsk::Keyframe& key, const adsk::Keyframe& next, bool hasPrev, bool hasNext,
                           double& in_tangent_x, double& in_tangent_y, double& out_tangent_x, double& out_tangent_y);

    void clump_spline_plateau(const adsk::Keyframe& prev, const adsk::Keyframe& key, const adsk::Keyframe& next, bool hasPrev, bool hasNext,
                              double& in_tangent_x, double& in_tangent_y, double& out_tangent_x, double& out_tangent_y);

    void compute_tangent(int index);
    adsk::InfinityType m_pre_infinity = adsk::InfinityType::Constant;
    adsk::InfinityType m_post_infinity = adsk::InfinityType::Constant;
    std::vector<adsk::Keyframe> m_sorted_keys;
};

typedef std::shared_ptr<AnimCurve> AnimCurvePtr;
typedef std::shared_ptr<const AnimCurve> AnimCurveCPtr;

OPENDCC_NAMESPACE_CLOSE
