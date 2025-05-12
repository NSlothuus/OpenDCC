/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
// workaround
#include "opendcc/base/qt_python.h"

#include <set>
#include "opendcc/vendor/animx/animx.h"
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"
#include "opendcc/anim_engine/core/engine.h"

OPENDCC_NAMESPACE_OPEN

enum class SelectionEvent
{
    SELECTION_CHANGED
};

struct SelectedTangent
{
    enum TangentDirection
    {
        in,
        out
    };
    adsk::KeyId key_id;
    TangentDirection direction;
    inline bool operator<(const SelectedTangent& other) const
    {
        if (key_id < other.key_id)
        {
            return true;
        }
        if (key_id > other.key_id)
        {
            return false;
        }
        return direction < other.direction;
    }
    inline bool operator==(const SelectedTangent& other) const { return (key_id == other.key_id) && (direction == other.direction); }
};

struct SelectionInfo
{
    std::set<adsk::KeyId> selected_keys;
    std::set<SelectedTangent> selected_tangents;
    // std::map<uint64_t, adsk::Keyframe> keys;
    bool operator==(const SelectionInfo& other) const;
    bool operator!=(const SelectionInfo& other) const;
};

using SelectionEventDispatcher =
    eventpp::EventDispatcher<SelectionEvent, void(const std::map<OPENDCC_NAMESPACE::AnimEngine::CurveId, SelectionInfo>&)>;
using SelectionEventDispatcherHandle =
    eventpp::EventDispatcher<SelectionEvent, void(const std::map<OPENDCC_NAMESPACE::AnimEngine::CurveId, SelectionInfo>&)>::Handle;

SelectionEventDispatcher& global_selection_dispatcher();

OPENDCC_NAMESPACE_CLOSE
