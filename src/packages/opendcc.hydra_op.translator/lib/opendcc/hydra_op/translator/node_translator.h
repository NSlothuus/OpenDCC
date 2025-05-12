/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include "opendcc/hydra_op/schema/baseNode.h"

OPENDCC_NAMESPACE_OPEN

class HydraOpNodeTranslator
{
public:
    enum DirtyType
    {
        Clean = 0,
        DirtyArgs = 0b1,
        DirtyInput = 0b10,
        DirtyNode = DirtyArgs | DirtyInput
    };
    using DirtyTypeFlags = uint32_t;

    struct NodeInterface
    {
        std::vector<PXR_NS::TfToken> inputs;
        PXR_NS::TfToken output;
    };

    virtual ~HydraOpNodeTranslator() {}

    virtual DirtyTypeFlags get_dirty_flags(const PXR_NS::UsdPrim& prim, const PXR_NS::TfToken& property_name) = 0;
    virtual PXR_NS::HdSceneIndexBaseRefPtr populate(const PXR_NS::UsdPrim& prim, const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& inputs) = 0;
    virtual void process_args_change(const PXR_NS::UsdPrim& prim, const PXR_NS::TfTokenVector& property_names,
                                     const PXR_NS::HdSceneIndexBaseRefPtr& scene_index) = 0;
    virtual bool is_time_dependent() const { return false; };
    virtual void on_time_changed(const PXR_NS::UsdPrim& prim, const PXR_NS::HdSceneIndexBaseRefPtr& scene_index, PXR_NS::UsdTimeCode time) {};
};

template <class T>
class HydraOpNodeTranslatorTyped : public HydraOpNodeTranslator
{
public:
    using TUsdPrimType = T;
    static_assert(std::is_base_of_v<PXR_NS::UsdAPISchemaBase, T> || std::is_base_of_v<PXR_NS::UsdHydraOpBaseNode, T>,
                  "T must be derived from UsdAPISchemaBase or UsdHydraOpBaseNode");

    DirtyTypeFlags get_dirty_flags(const PXR_NS::UsdPrim& prim, const PXR_NS::TfToken& property_name) final override
    {
        if (auto typed_prim = TUsdPrimType(prim))
        {
            return get_dirty_flags_impl(typed_prim, property_name);
        }

        return DirtyType::DirtyNode;
    }
    PXR_NS::HdSceneIndexBaseRefPtr populate(const PXR_NS::UsdPrim& prim, const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& inputs) final override
    {
        if (auto typed_prim = TUsdPrimType(prim))
        {
            return populate_impl(typed_prim, inputs);
        }
        return nullptr;
    }

    void process_args_change(const PXR_NS::UsdPrim& prim, const PXR_NS::TfTokenVector& property_names,
                             const PXR_NS::HdSceneIndexBaseRefPtr& scene_index) final override
    {
        if (auto typed_prim = TUsdPrimType(prim))
        {
            process_args_change_impl(typed_prim, property_names, scene_index);
        }
    }

    void on_time_changed(const PXR_NS::UsdPrim& prim, const PXR_NS::HdSceneIndexBaseRefPtr& scene_index, PXR_NS::UsdTimeCode time) final override
    {
        if (auto typed_prim = TUsdPrimType(prim))
        {
            on_time_changed_impl(typed_prim, scene_index, time);
        }
    };

protected:
    virtual DirtyTypeFlags get_dirty_flags_impl(const TUsdPrimType& prim, const PXR_NS::TfToken& property_name) = 0;
    virtual PXR_NS::HdSceneIndexBaseRefPtr populate_impl(const TUsdPrimType& prim, const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& inputs) = 0;
    virtual void process_args_change_impl(const TUsdPrimType& prim, const PXR_NS::TfTokenVector& property_names,
                                          const PXR_NS::HdSceneIndexBaseRefPtr& scene_index) = 0;
    virtual void on_time_changed_impl(const TUsdPrimType& prim, const PXR_NS::HdSceneIndexBaseRefPtr& scene_index, PXR_NS::UsdTimeCode time) {};
};

OPENDCC_NAMESPACE_CLOSE
