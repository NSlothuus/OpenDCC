// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/anim_engine/core/anim_engine_curve.h"
#include "opendcc/anim_engine/core/session.h"
#include "opendcc/anim_engine/core/stage_listener.h"
#include <pxr/usd/usd/editContext.h>
#include "opendcc/anim_engine/schema/tokens.h"

PXR_NAMESPACE_USING_DIRECTIVE

OPENDCC_NAMESPACE_OPEN

const uint32_t AnimEngineCurve::undefined_id = 0;

AnimEngineCurve::AnimEngineCurve(PXR_NS::UsdAttribute attribute, uint32_t component_idx, SdfLayerHandle current_layer)
    : m_attribute(attribute)
    , m_component_idx(component_idx)
    , m_current_layer(current_layer)
{
    if (attribute && !current_layer)
    {
        m_current_layer = m_attribute.GetStage()->GetEditTarget().GetLayer();
    }
}

namespace
{
    static const auto default_infinity_type = adsk::InfinityType::Constant;
    static const auto default_tangent_type = adsk::TangentType::Auto;

    const std::map<adsk::InfinityType, TfToken> infinity_type_to_token = { { adsk::InfinityType::Constant, UsdAnimEngineTokens->constant },
                                                                           { adsk::InfinityType::Linear, UsdAnimEngineTokens->linear },
                                                                           { adsk::InfinityType::Cycle, UsdAnimEngineTokens->cycle },
                                                                           { adsk::InfinityType::CycleRelative, UsdAnimEngineTokens->cycleRelative },
                                                                           { adsk::InfinityType::Oscillate, UsdAnimEngineTokens->oscillate } };

    const std::map<adsk::TangentType, TfToken> tangent_type_to_token = {
        { adsk::TangentType::Global, UsdAnimEngineTokens->global_ },   { adsk::TangentType::Fixed, UsdAnimEngineTokens->fixed },
        { adsk::TangentType::Linear, UsdAnimEngineTokens->linear },    { adsk::TangentType::Flat, UsdAnimEngineTokens->flat },
        { adsk::TangentType::Step, UsdAnimEngineTokens->step },        { adsk::TangentType::Slow, UsdAnimEngineTokens->slow },
        { adsk::TangentType::Fast, UsdAnimEngineTokens->fast },        { adsk::TangentType::Smooth, UsdAnimEngineTokens->smooth },
        { adsk::TangentType::Clamped, UsdAnimEngineTokens->clamped },  { adsk::TangentType::Auto, UsdAnimEngineTokens->auto_ },
        { adsk::TangentType::Sine, UsdAnimEngineTokens->sine },        { adsk::TangentType::Parabolic, UsdAnimEngineTokens->parabolic },
        { adsk::TangentType::Log, UsdAnimEngineTokens->log },          { adsk::TangentType::Plateau, UsdAnimEngineTokens->plateau },
        { adsk::TangentType::StepNext, UsdAnimEngineTokens->stepNext }
    };

    std::map<TfToken, adsk::InfinityType> compute_token_to_infinity_type()
    {
        std::map<TfToken, adsk::InfinityType> result;
        for (auto it : infinity_type_to_token)
        {
            result[it.second] = it.first;
        }
        return result;
    }

    std::map<TfToken, adsk::TangentType> compute_token_to_tangen_type()
    {
        std::map<TfToken, adsk::TangentType> result;
        for (auto it : tangent_type_to_token)
        {
            result[it.second] = it.first;
        }
        return result;
    }

    adsk::InfinityType token_to_infinity_type(const TfToken& token)
    {
        static const std::map<TfToken, adsk::InfinityType> token_to_infinity_type_map = compute_token_to_infinity_type();
        if (token.IsEmpty())
        {
            return default_infinity_type;
        }

        auto it = token_to_infinity_type_map.find(token);
        if (it != token_to_infinity_type_map.end())
        {
            return it->second;
        }
        OPENDCC_WARN("Invalid infinity type {}", token.GetText());

        return default_infinity_type;
    }

    adsk::TangentType token_to_tangen_type(const TfToken& token)
    {
        if (token.IsEmpty())
        {
            return default_tangent_type;
        }
        static const std::map<TfToken, adsk::TangentType> token_to_tangen_type_map = compute_token_to_tangen_type();
        auto it = token_to_tangen_type_map.find(token);
        if (it != token_to_tangen_type_map.end())
        {
            return it->second;
        }
        OPENDCC_WARN("Invalid tangent type {}", token.GetText());

        return default_tangent_type;
    }

}

void AnimEngineCurve::save_to_attribute_metadata(StageListener& stage_listener, bool save_on_current_layer) const
{
    ANIM_CURVES_CHECK_AND_RETURN(m_attribute);
    auto scope = stage_listener.create_mute_scope();
    auto num_keyframes = keyframeCount();

    VtArray<double> times(num_keyframes);
    VtArray<double> values(num_keyframes);
    VtArray<GfVec2d> in_tangents(num_keyframes);
    VtArray<GfVec2d> out_tangents(num_keyframes);
    VtArray<TfToken> in_tangents_types(num_keyframes);
    VtArray<TfToken> out_tangents_types(num_keyframes);

    for (size_t i = 0; i < num_keyframes; ++i)
    {
        const auto& key = at(i);
        times[i] = key.time;
        values[i] = key.value;
        in_tangents[i][0] = key.tanIn.x;
        in_tangents[i][1] = key.tanIn.y;
        out_tangents[i][0] = key.tanOut.x;
        out_tangents[i][1] = key.tanOut.y;
        in_tangents_types[i] = tangent_type_to_token.at(key.tanIn.type);
        out_tangents_types[i] = tangent_type_to_token.at(key.tanOut.type);
    }

    VtDictionary curve_data;
    curve_data["time"] = times;
    curve_data["value"] = values;
    curve_data["inTangent"] = in_tangents;
    curve_data["outTangent"] = out_tangents;
    curve_data["inTangentType"] = in_tangents_types;
    curve_data["outTangentType"] = out_tangents_types;
    curve_data["preInfinityType"] = infinity_type_to_token.at(preInfinityType());
    curve_data["postInfinityType"] = infinity_type_to_token.at(postInfinityType());

    SdfAttributeSpecHandle attr_spec;

    if (save_on_current_layer || !m_current_layer)
    {
        attr_spec = m_attribute.GetStage()->GetEditTarget().GetLayer()->GetAttributeAtPath(m_attribute.GetPath());
    }
    else
    {
        attr_spec = m_current_layer->GetAttributeAtPath(m_attribute.GetPath());
    }

    if (attr_spec)
    {
        VtDictionary metadata = attr_spec->GetFieldAs<VtDictionary>(UsdAnimEngineTokens->anim);
        metadata[std::to_string(m_component_idx)] = curve_data;
        attr_spec->SetField(UsdAnimEngineTokens->anim, VtValue(metadata));
    }
}

void AnimEngineCurve::remove_from_metadata(StageListener& stage_listener) const
{
    ANIM_CURVES_CHECK_AND_RETURN(m_attribute);

    auto scope = stage_listener.create_mute_scope();
    VtDictionary metadata;
    m_attribute.GetMetadata(UsdAnimEngineTokens->anim, &metadata);
    metadata.erase(std::to_string(m_component_idx));
    if (metadata.size() > 0)
    {
        m_attribute.SetMetadata(UsdAnimEngineTokens->anim, metadata);
    }
    else
    {
        m_attribute.ClearMetadata(UsdAnimEngineTokens->anim);
    }
}

std::vector<AnimEngineCurve> AnimEngineCurve::create_from_metadata(UsdAttribute& attr)
{
    if (!attr || !attr.HasMetadata(UsdAnimEngineTokens->anim))
    {
        return std::vector<AnimEngineCurve>();
    }

    std::vector<AnimEngineCurve> result;
    VtArray<double> times;
    VtArray<double> values;
    VtArray<GfVec2d> in_tangents;
    VtArray<GfVec2d> out_tangents;
    VtArray<TfToken> in_tangents_types;
    VtArray<TfToken> out_tangents_types;

    TfToken pre_infinity_type;
    TfToken post_infinity_type;

    std::set<uint32_t> loaded_components;

    SdfPropertySpecHandleVector property_stack = attr.GetPropertyStack();
    for (const SdfPropertySpecHandle& property_spec : property_stack)
    {
        if (!property_spec->HasField(UsdAnimEngineTokens->anim))
        {
            continue;
        }

        auto metadata = property_spec->GetFieldAs<VtDictionary>(UsdAnimEngineTokens->anim);
        for (auto it : metadata)
        {
            uint32_t component_idx;
            try
            {
                component_idx = std::stoi(it.first);
            }
            catch (...)
            {
                continue;
            }

            if (loaded_components.find(component_idx) != loaded_components.end())
            {
                continue;
            }
            else
            {
                loaded_components.insert(component_idx);
            }

            auto current_layer = property_spec->GetLayer();

            AnimEngineCurve curve(attr, component_idx, current_layer);
            VtDictionary curve_data;

            curve_data = it.second.Get<VtDictionary>();

            times = curve_data["time"].Get<VtArray<double>>();
            values = curve_data["value"].Get<VtArray<double>>();
            in_tangents = curve_data["inTangent"].Get<VtArray<GfVec2d>>();
            out_tangents = curve_data["outTangent"].Get<VtArray<GfVec2d>>();

            in_tangents_types = curve_data["inTangentType"].Get<VtArray<TfToken>>();
            out_tangents_types = curve_data["outTangentType"].Get<VtArray<TfToken>>();

            pre_infinity_type = curve_data["preInfinityType"].Get<TfToken>();
            post_infinity_type = curve_data["postInfinityType"].Get<TfToken>();
            auto num_keys = times.size();
            if (values.size() != num_keys || in_tangents.size() != num_keys || out_tangents.size() != num_keys ||
                in_tangents_types.size() != num_keys || out_tangents_types.size() != num_keys)
            {
                continue;
            }

            for (size_t i = 0; i < num_keys; ++i)
            {
                adsk::Keyframe key;
                key.value = values[i];
                key.time = times[i];
                key.tanIn.x = in_tangents[i][0];
                key.tanIn.y = in_tangents[i][1];
                key.tanOut.x = out_tangents[i][0];
                key.tanOut.y = out_tangents[i][1];
                key.tanIn.type = token_to_tangen_type(in_tangents_types[i]);
                key.tanOut.type = token_to_tangen_type(in_tangents_types[i]);

                curve.add_key(key, true);
            }

            curve.set_pre_infinity_type(token_to_infinity_type(pre_infinity_type));
            curve.set_post_infinity_type(token_to_infinity_type(post_infinity_type));

            result.push_back(curve);
        }
    }

    return result;
}

OPENDCC_NAMESPACE_CLOSE
