// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/scene_indices/hydra_engine_scene_indices_notifier.h"

OPENDCC_NAMESPACE_OPEN

HydraEngineSceneIndicesNotifier::Dispatcher HydraEngineSceneIndicesNotifier::m_dispatcher;

void HydraEngineSceneIndicesNotifier::on_index_created(IndexType index_type, PXR_NS::TfRefPtr<PXR_NS::HdSceneIndexBase> index)
{
    m_dispatcher.dispatch(index_type, index);
}

HydraEngineSceneIndicesNotifier::Handle HydraEngineSceneIndicesNotifier::register_index_created(
    IndexType index_type, std::function<void(PXR_NS::TfRefPtr<PXR_NS::HdSceneIndexBase>)> callback)
{
    return m_dispatcher.appendListener(index_type, callback);
}

void HydraEngineSceneIndicesNotifier::unregister_index_created(IndexType index_type, Handle handle)
{
    m_dispatcher.removeListener(index_type, handle);
}

OPENDCC_NAMESPACE_CLOSE
