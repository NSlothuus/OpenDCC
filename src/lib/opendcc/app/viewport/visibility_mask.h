/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/base/tf/token.h>
#include <map>
#include <unordered_set>
#include <pxr/base/tf/staticTokens.h>
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"

PXR_NAMESPACE_OPEN_SCOPE
#define OPENDCC_PRIM_VISIBILITY_TOKENS (mesh)(basisCurves)(camera)(light)

TF_DECLARE_PUBLIC_TOKENS(PrimVisibilityTypes, OPENDCC_API, OPENDCC_PRIM_VISIBILITY_TOKENS);
PXR_NAMESPACE_CLOSE_SCOPE

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API VisibilityMask
{
public:
    bool is_visible(const PXR_NS::TfToken& type, const PXR_NS::TfToken& group = PXR_NS::TfToken()) const;
    void set_visible(bool visible, const PXR_NS::TfToken& type, const PXR_NS::TfToken& group = PXR_NS::TfToken());

    bool is_dirty() const;
    void mark_clean();

private:
    using VisibilitySet = std::unordered_set<PXR_NS::TfToken, PXR_NS::TfToken::HashFunctor>;
    using GroupVisiblityMap = std::map<PXR_NS::TfToken, VisibilitySet>;

    GroupVisiblityMap m_visibility_map;
    bool m_is_dirty = true;
};

class OPENDCC_API PrimVisibilityRegistry
{
public:
    enum class EventType
    {
        VISIBILITY_TYPES_CHANGED
    };
    using EventDispatcher = eventpp::EventDispatcher<EventType, void()>;
    using CallbackHandle = eventpp::EventDispatcher<EventType, void()>::Handle;
    struct PrimVisibilityType
    {
        PXR_NS::TfToken group;
        PXR_NS::TfToken type;
        std::string ui_name;
        bool operator==(const PrimVisibilityType& other) const { return group == other.group && type == other.type; }
        bool operator!=(const PrimVisibilityType& other) const { return !(*this == other); }
    };

    static bool register_prim_type(const PXR_NS::TfToken& type, const PXR_NS::TfToken& group, const std::string& ui_name);
    static bool unregister_prim_type(const PXR_NS::TfToken& type, const PXR_NS::TfToken& group);
    static CallbackHandle register_visibility_types_changes(std::function<void()> callback);
    static void unregister_visibility_types_changes(CallbackHandle handle);
    static const std::vector<PrimVisibilityType>& get_prim_visibility_types();

private:
    PrimVisibilityRegistry();
    PrimVisibilityRegistry(const PrimVisibilityRegistry&) = delete;
    PrimVisibilityRegistry(PrimVisibilityRegistry&&) = delete;

    PrimVisibilityRegistry& operator=(const PrimVisibilityRegistry&) = delete;
    PrimVisibilityRegistry& operator=(PrimVisibilityRegistry&&) = delete;

    static PrimVisibilityRegistry& instance();

    std::vector<PrimVisibilityType> m_prim_visibility_data;
    EventDispatcher m_dispatcher;
};

OPENDCC_NAMESPACE_CLOSE
