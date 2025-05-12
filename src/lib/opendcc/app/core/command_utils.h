/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/usd/sdf/namespaceEdit.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include "opendcc/base/vendor/eventpp/callbacklist.h"

OPENDCC_NAMESPACE_OPEN

namespace commands
{
    namespace utils
    {
        template <class TCommand, class... TArgs>
        class CommandExecNotifier
        {
        public:
            using CallbackSignature = void(TArgs... args);
            using Dispatcher = eventpp::CallbackList<CallbackSignature>;
            using Handle = typename Dispatcher::Handle;

            void notify(TArgs... args) { m_dispatcher(args...); }
            Handle register_handle(const std::function<CallbackSignature>& callback) { return m_dispatcher.append(callback); }
            void unregister_handle(Handle handle) { m_dispatcher.remove(handle); }

        private:
            Dispatcher m_dispatcher;
        };

        OPENDCC_API PXR_NS::TfToken get_new_name_for_prim(const PXR_NS::TfToken& name_candidate, const PXR_NS::UsdPrim& parent_prim,
                                                          const PXR_NS::SdfPathVector& additional_paths = {});
        OPENDCC_API PXR_NS::TfToken get_new_name(const PXR_NS::TfToken& name_candidate, const PXR_NS::TfTokenVector& existing_names);
        OPENDCC_API void rename_targets(const PXR_NS::UsdStageRefPtr& stage, const PXR_NS::SdfPath& old_path, const PXR_NS::SdfPath& new_path);
        OPENDCC_API void delete_targets(const PXR_NS::UsdStageRefPtr& stage, const PXR_NS::SdfPath& remove_path);
        OPENDCC_API void preserve_transform(const PXR_NS::UsdPrim& prim, const PXR_NS::UsdPrim& parent);
        OPENDCC_API PXR_NS::SdfPath get_common_parent(const PXR_NS::SdfPathVector& paths);
        OPENDCC_API void apply_schema_to_spec(const std::string& schema_name, const std::vector<PXR_NS::SdfPrimSpecHandle>& prim_specs);
        OPENDCC_API void select_prims(const PXR_NS::SdfPathVector& new_selection);
        OPENDCC_API void flatten_prim(const PXR_NS::UsdPrim& src_prim, const PXR_NS::SdfPath& dst_path, const PXR_NS::SdfLayerHandle layer,
                                      bool copy_children = true);
        OPENDCC_API bool copy_spec_without_children(const PXR_NS::SdfLayerHandle& srcLayer, const PXR_NS::SdfPath& srcPath,
                                                    const PXR_NS::SdfLayerHandle& dstLayer, const PXR_NS::SdfPath& dstPath);
    }
}

OPENDCC_NAMESPACE_CLOSE
