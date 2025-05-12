/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "pxr/usd/usd/attribute.h"
#include "pxr/usd/sdf/layer.h"
#include <atomic>
#include <functional>
#include <memory>
#include <set>
#include <unordered_map>
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"
#include "opendcc/anim_engine/core/api.h"
#include "opendcc/anim_engine/core/anim_engine_curve.h"
#include "opendcc/app/core/application.h"
#include "opendcc/anim_engine/core/stage_listener.h"
#include "opendcc/anim_engine/core/publisher.h"

OPENDCC_NAMESPACE_OPEN

class AddCurvesAndKeysCommand;
class RemoveCurvesAndKeysCommand;
class ChangeKeyframesCommand;
class ChangeInfinityTypeCommand;

enum class AnimEngineOption
{
    isSaveOnCurrentLayer
};
using AnimEngineOptionChanged = Publisher<AnimEngineOption, void()>;

class ANIM_ENGINE_API AnimEngine
    : public AnimEngineOptionChanged
    , IStageListenerClient
{
    friend AddCurvesAndKeysCommand;
    friend ChangeInfinityTypeCommand;
    friend RemoveCurvesAndKeysCommand;
    friend ChangeKeyframesCommand;

public:
    enum class EventType
    {
        CURVES_ADDED,
        CURVES_REMOVED,
        KEYFRAMES_ADDED,
        KEYFRAMES_REMOVED,
        KEYFRAMES_CHANGED,
        INFINITY_CHANGED
    };

    enum class AttributesScope
    {
        TRANSLATE,
        ROTATE,
        SCALE,
        ALL
    };

    class ANIM_ENGINE_API CurveId
    {
    public:
        CurveId() {}
        CurveId(PXR_NS::SdfPath attr_path, uint32_t component_idx)
            : m_attr_path(attr_path)
            , m_component_idx(component_idx)
        {
        }

        bool operator<(const CurveId& other) const
        {
            if (m_attr_path == other.m_attr_path)
            {
                return m_component_idx < other.m_component_idx;
            }
            return m_attr_path < other.m_attr_path;
        }

        bool operator==(const CurveId& other) const { return m_attr_path == other.m_attr_path && m_component_idx == other.m_component_idx; }

        bool operator!=(const CurveId& other) const { return !(*this == other); }

        bool valid() const { return !m_attr_path.IsEmpty(); }

    private:
        PXR_NS::SdfPath m_attr_path;
        uint32_t m_component_idx = 0;
    };

    // using CurveId = std::string;
    using CurveIdsList = std::vector<CurveId>;
    using CurvesList = std::vector<std::shared_ptr<const AnimCurve>>;
    using CurveIdToKeysIdsMap = std::map<CurveId, std::set<adsk::KeyId>>;
    using CurveIdToKeyframesMap = std::map<CurveId, std::vector<adsk::Keyframe>>;

    using EventDispatcherForCurveUpdate = eventpp::EventDispatcher<EventType, void(const CurveIdsList&)>;
    using CurveUpdateCallbackHandle = eventpp::EventDispatcher<EventType, void(const CurveIdsList&)>::Handle;

    using EventDispatcherForKeysListUpdate = eventpp::EventDispatcher<EventType, void(const CurveIdToKeysIdsMap&)>;
    using KeysListUpdateCallbackHandle = eventpp::EventDispatcher<EventType, void(const CurveIdToKeysIdsMap&)>::Handle;

    AnimEngine(PXR_NS::UsdStageRefPtr stage);

    virtual ~AnimEngine();

    virtual void update(const std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash>& attrs_to_update,
                        const std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash>& attrs_to_remove) override;

    CurveIdsList key_attributes(const PXR_NS::UsdAttributeVector& attrs);
    CurveIdsList key_attribute(PXR_NS::UsdAttribute attr);
    CurveIdsList key_attributes(const PXR_NS::UsdAttributeVector& attrs, std::vector<uint32_t> components);
    CurveId create_animation_curve(PXR_NS::UsdAttribute attr, uint32_t components);
    void euler_filter(const std::set<CurveId>& curves_ids);
    bool remove_animation_curves(PXR_NS::UsdAttribute attr);
    bool remove_animation_curves(const PXR_NS::UsdAttributeVector& attrs);
    bool bake(PXR_NS::SdfLayerRefPtr layer, const PXR_NS::SdfPathVector& prim_paths, const PXR_NS::UsdAttributeVector& attrs, double start_frame,
              double end_frame, const std::vector<double>& frame_samples, bool remove_origin);
    bool bake(PXR_NS::SdfLayerRefPtr layer, const CurveIdsList& curves_ids, double start_frame, double end_frame,
              const std::vector<double>& frame_samples, bool remove_origin);
    bool bake_all(PXR_NS::SdfLayerRefPtr layer, double start_frame, double end_frame, const std::vector<double>& frame_samples, bool remove_origin);

    void create_animation_on_selected_prims(AttributesScope attribute_scope);

    void remove_keys(const CurveIdToKeysIdsMap& ids);
    void remove_curves(const CurveIdsList& ids);
    void add_keys(const CurveIdToKeyframesMap& ids);

    void set_infinity_type(const CurveIdsList& curve_ids, adsk::InfinityType infinity, bool is_pre_infinity);

    bool is_attribute_animated(PXR_NS::UsdAttribute attr, uint32_t component) const;
    bool is_attribute_animated(PXR_NS::UsdAttribute attr) const;
    bool is_prim_has_animated_attributes(PXR_NS::SdfPath prim_path) const;
    CurveId get_curve_id(PXR_NS::UsdAttribute attr, uint32_t component) const;
    std::set<float> selected_prims_keys_times() const;
    static void apply_euler_filter(AnimEngine::CurveId curves_id, AnimEngineCurveCPtr curve, AnimEngine::CurveIdToKeyframesMap& start_keyframes_list,
                                   AnimEngine::CurveIdToKeyframesMap& end_keyframes_list);

    CurveUpdateCallbackHandle register_event_callback(EventType type, const std::function<void(const CurveIdsList&)>& callback)
    {
        return m_dispatcher_for_curve_update.appendListener(type, callback);
    }
    void unregister_event_callback(EventType type, CurveUpdateCallbackHandle& handle) { m_dispatcher_for_curve_update.removeListener(type, handle); }

    KeysListUpdateCallbackHandle register_event_callback(EventType type, const std::function<void(const CurveIdToKeysIdsMap&)>& callback)
    {
        return m_dispatcher_for_keys_update.appendListener(type, callback);
    }
    void unregister_event_callback(EventType type, KeysListUpdateCallbackHandle& handle)
    {
        m_dispatcher_for_keys_update.removeListener(type, handle);
    }

    std::shared_ptr<const AnimEngineCurve> get_curve(CurveId curve_id) const;
    CurveIdsList get_ids_for_attr(const PXR_NS::SdfPath& attr_path) const;
    const std::set<CurveId>& curves(const PXR_NS::SdfPath& prim_path) const;
    std::pair<AnimEngine::CurveId, AnimEngineCurveCPtr> id_and_curve(PXR_NS::UsdAttribute attr, uint32_t component);
    void clear();
    void on_changed() const;

    void set_keys_direct(const CurveIdToKeyframesMap& ids, bool send_notification = true);
    CurveId get_or_generate_id(PXR_NS::UsdAttribute attr, uint32_t component) const;
    void set_save_on_current_layer(bool save_on_current_layer);
    bool is_save_on_current_layer() const { return m_save_on_current_layer; }

private:
    CurveIdsList add_curves_direct(const std::vector<AnimEngineCurve>& curves, bool store_to_stage = true);
    void add_curves_direct(const std::vector<AnimEngineCurve>& curves, const CurveIdsList& ids, bool store_to_stage = true);
    void remove_curves_direct(const CurveIdsList& curve_id);

    CurveIdToKeysIdsMap add_keys_direct(const CurveIdToKeyframesMap& keys, bool reset_id, bool send_notification = true);
    void remove_keys_direct(const CurveIdToKeysIdsMap& ids);
    void set_infinity_type_direct(const CurveIdsList& curve_ids, adsk::InfinityType infinity, bool is_pre_infinity);
    void set_infinity_type_direct(const std::map<AnimEngine::CurveId, adsk::InfinityType>& infinity_values, bool is_pre_infinity);

    CurveIdsList create_animation_curve_and_add_keys(std::vector<PXR_NS::UsdAttribute> attrs, std::vector<uint32_t> components,
                                                     CurveIdToKeyframesMap extra_keys);

    PXR_NS::SdfLayerRefPtr get_or_create_anonymous_layer();

    EventDispatcherForCurveUpdate m_dispatcher_for_curve_update;
    EventDispatcherForKeysListUpdate m_dispatcher_for_keys_update;

    std::map<CurveId, std::shared_ptr<AnimEngineCurve>> m_curves;
    std::map<PXR_NS::SdfPath, std::set<CurveId>> m_prim_path_to_curves_map;
    StageListener m_stage_listener;
    bool m_save_on_current_layer = false;
};

using AnimEnginePtr = std::shared_ptr<AnimEngine>;

OPENDCC_NAMESPACE_CLOSE
