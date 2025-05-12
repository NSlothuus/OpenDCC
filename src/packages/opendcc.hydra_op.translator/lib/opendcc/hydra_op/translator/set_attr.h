/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include "opendcc/hydra_op/translator/network.h"
#include <pxr/usdImaging/usdImaging/sceneIndices.h>
#include <pxr/usd/sdf/pathExpression.h>
#include <pxr/imaging/hd/filteringSceneIndex.h>
#include "opendcc/hydra_op/schema/setAttribute.h"

OPENDCC_NAMESPACE_OPEN

class SetAttrSceneIndex final : public PXR_NS::HdSingleInputFilteringSceneIndexBase
{
public:
    static PXR_NS::TfRefPtr<SetAttrSceneIndex> New(const PXR_NS::HdSceneIndexBaseRefPtr& inputSceneIndex, const PXR_NS::SdfPathExpression& path,
                                                   const PXR_NS::TfToken& attr, const PXR_NS::VtValue& val);

public: // HdSceneIndex overrides
    PXR_NS::HdSceneIndexPrim GetPrim(const PXR_NS::SdfPath& primPath) const override;
    PXR_NS::SdfPathVector GetChildPrimPaths(const PXR_NS::SdfPath& primPath) const override;

    void set_args(const PXR_NS::SdfPathExpression& prim_path, const PXR_NS::TfToken& attr, const PXR_NS::VtValue& val);
    void set_path_expression(const PXR_NS::SdfPathExpression& prim_path);
    void set_attr_name(const PXR_NS::TfToken& attr_name);
    void set_value(const PXR_NS::VtValue& value);
    const PXR_NS::VtValue& get_value() const;
    const PXR_NS::TfToken& get_attr_name() const;
    const PXR_NS::SdfPathExpression& get_path_expression() const;

protected: // HdSingleInputFilteringSceneIndexBase overrides
    void _PrimsAdded(const PXR_NS::HdSceneIndexBase& sender, const PXR_NS::HdSceneIndexObserver::AddedPrimEntries& entries) override;
    void _PrimsRemoved(const PXR_NS::HdSceneIndexBase& sender, const PXR_NS::HdSceneIndexObserver::RemovedPrimEntries& entries) override;
    void _PrimsDirtied(const PXR_NS::HdSceneIndexBase& sender, const PXR_NS::HdSceneIndexObserver::DirtiedPrimEntries& entries) override;

private:
    SetAttrSceneIndex(const PXR_NS::HdSceneIndexBaseRefPtr& inputSceneIndex, const PXR_NS::SdfPathExpression& prim_path, const PXR_NS::TfToken& attr,
                      const PXR_NS::VtValue& val);

    PXR_NS::SdfPathExpression m_path_expression;
    PXR_NS::TfToken m_attr;
    PXR_NS::VtValue m_value;
};

class SetAttrTranslator final : public HydraOpNodeTranslatorTyped<PXR_NS::UsdHydraOpSetAttribute>
{
protected:
    DirtyTypeFlags get_dirty_flags_impl(const PXR_NS::UsdHydraOpSetAttribute& prim, const PXR_NS::TfToken& property_name) override;
    PXR_NS::HdSceneIndexBaseRefPtr populate_impl(const PXR_NS::UsdHydraOpSetAttribute& prim,
                                                 const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& inputs) override;
    void process_args_change_impl(const PXR_NS::UsdHydraOpSetAttribute& prim, const PXR_NS::TfTokenVector& property_names,
                                  const PXR_NS::HdSceneIndexBaseRefPtr& scene_index) override;
};

OPENDCC_NAMESPACE_CLOSE
