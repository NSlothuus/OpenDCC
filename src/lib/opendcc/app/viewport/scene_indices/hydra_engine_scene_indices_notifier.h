/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include <pxr/imaging/hd/sceneIndex.h>
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"

OPENDCC_NAMESPACE_OPEN

class HydraEngineSceneIndicesNotifier
{
public:
    enum class IndexType
    {
        Prune
    };
    using Dispatcher = eventpp::EventDispatcher<IndexType, void(PXR_NS::TfRefPtr<PXR_NS::HdSceneIndexBase>)>;
    using Handle = Dispatcher::Handle;

    static void on_index_created(IndexType index_type, PXR_NS::TfRefPtr<PXR_NS::HdSceneIndexBase> index);
    static Handle register_index_created(IndexType index_type, std::function<void(PXR_NS::TfRefPtr<PXR_NS::HdSceneIndexBase>)> callback);
    static void unregister_index_created(IndexType index_type, Handle handle);

private:
    HydraEngineSceneIndicesNotifier() = default;

    static Dispatcher m_dispatcher;
};

OPENDCC_NAMESPACE_CLOSE
