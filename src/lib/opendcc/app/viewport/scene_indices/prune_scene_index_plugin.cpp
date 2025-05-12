// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/scene_indices/prune_scene_index_plugin.h"
#include "opendcc/usd_editor/scene_indices/prune_scene_index.h"

#include <pxr/imaging/hd/sceneIndexPluginRegistry.h>
#include "opendcc/app/viewport/scene_indices/hydra_engine_scene_indices_notifier.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

TF_REGISTRY_FUNCTION(TfType)
{
    HdSceneIndexPluginRegistry::Define<PruneSceneIndexPlugin>();
}

TF_REGISTRY_FUNCTION(HdSceneIndexPlugin)
{
    HdSceneIndexPluginRegistry::GetInstance().RegisterSceneIndexForRenderer(std::string(), TfToken("OpenDCC::PruneSceneIndexPlugin"), nullptr, 0,
                                                                            HdSceneIndexPluginRegistry::InsertionOrderAtStart);
}

PXR_NS::HdSceneIndexBaseRefPtr PruneSceneIndexPlugin::_AppendSceneIndex(const PXR_NS::HdSceneIndexBaseRefPtr &input_scene,
                                                                        const PXR_NS::HdContainerDataSourceHandle &input_args)
{
    auto result = HdSceneIndexBaseRefPtr(new PruneSceneIndex(input_scene));
    HydraEngineSceneIndicesNotifier::on_index_created(HydraEngineSceneIndicesNotifier::IndexType::Prune, result);
    return result;
}

OPENDCC_NAMESPACE_CLOSE
