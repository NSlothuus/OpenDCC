// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "material_assign.h"
#include <pxr/base/tf/refPtr.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/meshSchema.h>
#include <pxr/imaging/hd/collectionExpressionEvaluator.h>
#include "opendcc/base/utils/string_utils.h"
#include <pxr/imaging/hdsi/primTypeAndPathPruningSceneIndex.h>
#include "opendcc/hydra_op/translator/set_attr.h"
#include <pxr/imaging/hd/materialBindingsSchema.h>

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    class MaterialAssignSceneIndex final : public PXR_NS::HdSingleInputFilteringSceneIndexBase
    {
    public:
        static PXR_NS::TfRefPtr<MaterialAssignSceneIndex> New(const PXR_NS::HdSceneIndexBaseRefPtr& inputSceneIndex,
                                                              const PXR_NS::SdfPathExpression& path, const PXR_NS::SdfPath& val)
        {
            return TfCreateRefPtr(new MaterialAssignSceneIndex(inputSceneIndex, path, val));
        }

    public: // HdSceneIndex overrides
        PXR_NS::HdSceneIndexPrim GetPrim(const PXR_NS::SdfPath& prim_path) const override
        {
            HdSceneIndexPrim prim;

            if (HdSceneIndexBaseRefPtr input = _GetInputSceneIndex())
            {
                prim = input->GetPrim(prim_path);
            }
            else
            {
                return prim;
            }

            SdfPathExpressionEval<const SdfPath&> eval =
                SdfMakePathExpressionEval<const SdfPath&>(m_path_expression, SdfPredicateLibrary<const SdfPath&>());
            if (eval.Match(prim_path, [](const SdfPath& p) { return p; }))
            {
                if (!prim.dataSource)
                {
                    return prim;
                }

                std::vector<TfToken> purposes;
                if (auto bind_schema = HdMaterialBindingsSchema::GetFromParent(prim.dataSource))
                {
                    purposes = bind_schema.GetContainer()->GetNames();
                }
                else
                {
                    purposes = { HdMaterialBindingsSchemaTokens->allPurpose };
                }

                auto new_binding = HdMaterialBindingSchema::Builder().SetPath(HdRetainedTypedSampledDataSource<SdfPath>::New(m_value)).Build();

                std::vector<HdDataSourceBaseHandle> paths(purposes.size(), new_binding);
                auto override_ds =
                    HdRetainedContainerDataSource::New(HdMaterialBindingsSchema::GetSchemaToken(),
                                                       HdMaterialBindingsSchema::BuildRetained(purposes.size(), purposes.data(), paths.data()));
                prim.dataSource = HdOverlayContainerDataSource::New(override_ds, prim.dataSource);
            }
            return prim;
        }

        PXR_NS::SdfPathVector GetChildPrimPaths(const PXR_NS::SdfPath& prim_path) const override
        {
            if (HdSceneIndexBaseRefPtr input = _GetInputSceneIndex())
            {
                return input->GetChildPrimPaths(prim_path);
            }
            return {};
        }

        void set_args(const PXR_NS::SdfPathExpression& prim_path, const PXR_NS::SdfPath& val)
        {
            HdSceneIndexObserver::DirtiedPrimEntries dirties;
            if (!m_path_expression.IsEmpty())
            {
                SdfPathVector dirty_paths;
                auto eval = HdCollectionExpressionEvaluator(_GetInputSceneIndex(), m_path_expression);
                eval.PopulateAllMatches(SdfPath::AbsoluteRootPath(), &dirty_paths);

                // old
                std::transform(dirty_paths.begin(), dirty_paths.end(), std::back_inserter(dirties), [this](const auto& p) {
                    return HdSceneIndexObserver::DirtiedPrimEntry { p, HdMaterialBindingsSchema::GetDefaultLocator() };
                });
            }

            if (!prim_path.IsEmpty())
            {
                SdfPathVector dirty_paths;
                auto eval = HdCollectionExpressionEvaluator(_GetInputSceneIndex(), prim_path);
                eval.PopulateAllMatches(SdfPath::AbsoluteRootPath(), &dirty_paths);

                // new
                std::transform(dirty_paths.begin(), dirty_paths.end(), std::back_inserter(dirties), [this](const auto& p) {
                    return HdSceneIndexObserver::DirtiedPrimEntry { p, HdMaterialBindingsSchema::GetDefaultLocator() };
                });
            }

            m_path_expression = prim_path;
            m_value = val;
            _SendPrimsDirtied(dirties);
        }

        void set_path_expression(const PXR_NS::SdfPathExpression& path_expr) { set_args(path_expr, m_value); }
        void set_value(const PXR_NS::SdfPath& value) { set_args(m_path_expression, value); }
        const PXR_NS::SdfPath& get_value() const { return m_value; }
        const PXR_NS::SdfPathExpression& get_path_expression() const { return m_path_expression; }

    protected: // HdSingleInputFilteringSceneIndexBase overrides
        void _PrimsAdded(const PXR_NS::HdSceneIndexBase& sender, const PXR_NS::HdSceneIndexObserver::AddedPrimEntries& entries) override
        {
            _SendPrimsAdded(entries);
        }
        void _PrimsRemoved(const PXR_NS::HdSceneIndexBase& sender, const PXR_NS::HdSceneIndexObserver::RemovedPrimEntries& entries) override
        {
            _SendPrimsRemoved(entries);
        }
        void _PrimsDirtied(const PXR_NS::HdSceneIndexBase& sender, const PXR_NS::HdSceneIndexObserver::DirtiedPrimEntries& entries) override
        {
            _SendPrimsDirtied(entries);
        }

    private:
        MaterialAssignSceneIndex(const PXR_NS::HdSceneIndexBaseRefPtr& inputSceneIndex, const PXR_NS::SdfPathExpression& prim_path,
                                 const PXR_NS::SdfPath& val)
            : HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
            , m_path_expression(prim_path)
            , m_value(val)
        {
        }

        PXR_NS::SdfPathExpression m_path_expression;
        PXR_NS::SdfPath m_value;
    };
};

MaterialAssignTranslator::DirtyTypeFlags MaterialAssignTranslator::get_dirty_flags_impl(const PXR_NS::UsdHydraOpMaterialAssign& prim,
                                                                                        const PXR_NS::TfToken& property_name)
{
    DirtyTypeFlags result { DirtyType::Clean };
    if (property_name == UsdHydraOpTokens->inputsIn)
    {
        result |= DirtyType::DirtyInput;
    }
    else if (property_name == UsdHydraOpTokens->inputsApplyTo || property_name == UsdHydraOpTokens->inputsMaterialAssign)
    {
        result |= DirtyType::DirtyArgs;
    }

    return result;
}

PXR_NS::HdSceneIndexBaseRefPtr MaterialAssignTranslator::populate_impl(const PXR_NS::UsdHydraOpMaterialAssign& prim,
                                                                       const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& inputs)
{
    SdfPathExpression apply_to;
    std::string material_assign;
    prim.GetInputsApplyToAttr().Get(&apply_to);
    prim.GetInputsMaterialAssignAttr().Get(&material_assign);

    return MaterialAssignSceneIndex::New(inputs.empty() ? nullptr : inputs[0], apply_to,
                                         !material_assign.empty() ? SdfPath(material_assign) : SdfPath::EmptyPath());
}

void MaterialAssignTranslator::process_args_change_impl(const PXR_NS::UsdHydraOpMaterialAssign& prim, const PXR_NS::TfTokenVector& property_names,
                                                        const PXR_NS::HdSceneIndexBaseRefPtr& scene_index)
{
    auto assign_si = PXR_NS::TfDynamic_cast<TfRefPtr<MaterialAssignSceneIndex>>(scene_index);
    if (!assign_si)
    {
        return;
    }

    bool changed = false;
    auto cur_expr = assign_si->get_path_expression();
    auto cur_assign = assign_si->get_value();

    for (const auto& name : property_names)
    {
        if (name == UsdHydraOpTokens->inputsApplyTo)
        {
            prim.GetInputsApplyToAttr().Get(&cur_expr);
            changed = true;
        }
        else if (name == UsdHydraOpTokens->inputsMaterialAssign)
        {
            std::string tmp_str_path;
            prim.GetInputsMaterialAssignAttr().Get(&tmp_str_path);
            cur_assign = tmp_str_path.empty() ? SdfPath::EmptyPath() : SdfPath(tmp_str_path);
            changed = true;
        }
    }

    if (changed)
    {
        assign_si->set_args(cur_expr, cur_assign);
    }
}

OPENDCC_NAMESPACE_CLOSE
