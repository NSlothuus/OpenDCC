/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/imaging/hd/sceneDelegate.h>
#include <pxr/imaging/hd/selection.h>
#include "opendcc/app/viewport/viewport_scene_context.h"
#include "opendcc/app/core/selection_list.h"

OPENDCC_NAMESPACE_OPEN

struct ViewportHydraEngineParams;

class OPENDCC_API ViewportSceneDelegate : public PXR_NS::HdSceneDelegate
{
public:
    ViewportSceneDelegate(PXR_NS::HdRenderIndex* render_index, const PXR_NS::SdfPath& delegate_id);
    virtual ~ViewportSceneDelegate() = default;

    virtual void update(const ViewportHydraEngineParams& engine_params) = 0;
    virtual void populate_selection(const SelectionList& selection_list, const PXR_NS::HdSelectionSharedPtr& result) = 0;
    PXR_NS::SdfPath convert_index_path_to_stage_path(const PXR_NS::SdfPath& index_path) const;
    PXR_NS::SdfPath convert_stage_path_to_index_path(const PXR_NS::SdfPath& stage_path) const;

protected:
    PXR_NS::HdSelection::HighlightMode m_selection_mode;
    friend class ViewportHydraEngine;
    void set_selection_mode(PXR_NS::HdSelection::HighlightMode selection_mode);
};
using ViewportSceneDelegateUPtr = std::unique_ptr<ViewportSceneDelegate>;
using ViewportSceneDelegateSPtr = std::shared_ptr<ViewportSceneDelegate>;

class OPENDCC_API ViewportSceneDelegateFactoryBase : public PXR_NS::TfType::FactoryBase
{
public:
    virtual ViewportSceneDelegateUPtr create(PXR_NS::HdRenderIndex* render_index, const PXR_NS::SdfPath& delegate_id) const = 0;
    virtual PXR_NS::TfToken get_context_type() const = 0;
};

template <class TDelegate>
class ViewportSceneDelegateFactory : public ViewportSceneDelegateFactoryBase
{
public:
    ViewportSceneDelegateFactory(const PXR_NS::TfToken& context_type)
        : m_context_type(context_type)
    {
    }
    virtual ViewportSceneDelegateUPtr create(PXR_NS::HdRenderIndex* render_index, const PXR_NS::SdfPath& delegate_id) const override
    {
        return std::make_unique<TDelegate>(render_index, delegate_id);
    }
    virtual PXR_NS::TfToken get_context_type() const override { return m_context_type; }

private:
    PXR_NS::TfToken m_context_type;
};

#define OPENDCC_REGISTER_SCENE_DELEGATE(DelegateType, ContextType)                       \
    TF_REGISTRY_FUNCTION(TfType)                                                         \
    {                                                                                    \
        TfType::Define<DelegateType, TfType::Bases<ViewportSceneDelegate>>().SetFactory( \
            std::make_unique<ViewportSceneDelegateFactory<DelegateType>>(ContextType));  \
    }

OPENDCC_NAMESPACE_CLOSE
