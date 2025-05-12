/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"

#include <iostream>
#include <unordered_set>

#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usd/attribute.h"

#include "pxr/usd/usd/notice.h"
#include "pxr/imaging/hd/sortedIds.h"

OPENDCC_NAMESPACE_OPEN

class IStageListenerClient
{
public:
    virtual void update(const std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash>& attrs_to_update,
                        const std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash>& attrs_to_remove) = 0;
};

class StageListener : public PXR_NS::TfWeakBase
{
    friend class MuteScope;

public:
    class MuteScope
    {
        friend class StageListener;

        MuteScope(const StageListener* self)
            : m_listener(self)
        {
            if (!m_listener->m_change_block)
            {
                m_listener->m_change_block = std::make_unique<PXR_NS::SdfChangeBlock>();
            }
            ++m_listener->m_mute_recursion_depth;
        }

    public:
        ~MuteScope()
        {
            if (m_listener->m_mute_recursion_depth == 1)
            {
                m_listener->m_change_block.reset();
            }
            --m_listener->m_mute_recursion_depth;
        }

        MuteScope(const MuteScope& other)
        {
            m_listener = other.m_listener;
            ++m_listener->m_mute_recursion_depth;
        }
        const MuteScope& operator=(MuteScope& other)
        {
            m_listener = other.m_listener;
            ++m_listener->m_mute_recursion_depth;
            return *this;
        }

    private:
        const StageListener* m_listener = nullptr;
    };

    MuteScope create_mute_scope() const { return MuteScope(this); }

    StageListener() {}

    void init(PXR_NS::UsdStageRefPtr stage, const PXR_NS::TfTokenVector& filds, IStageListenerClient* client);

    virtual ~StageListener()
    {
        if (m_objects_changed_notice_key)
            PXR_NS::TfNotice::Revoke(m_objects_changed_notice_key);
    }

    const PXR_NS::UsdStageRefPtr stage() const { return m_stage; }

private:
    bool is_attribute_has_required_metadata(const PXR_NS::UsdAttribute& attr) const;
    void on_objects_changed(PXR_NS::UsdNotice::ObjectsChanged const& notice);
    mutable std::unique_ptr<PXR_NS::SdfChangeBlock> m_change_block;
    PXR_NS::UsdStageRefPtr m_stage;
    PXR_NS::TfNotice::Key m_objects_changed_notice_key;
    PXR_NS::Hd_SortedIds m_sorted_paths;
    mutable size_t m_mute_recursion_depth = 0;
    IStageListenerClient* m_client = nullptr;
    PXR_NS::TfTokenVector m_filds;
};

OPENDCC_NAMESPACE_CLOSE