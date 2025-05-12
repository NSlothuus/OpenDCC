/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <functional>
#include <pxr/base/tf/weakBase.h>
#include <pxr/pxr.h>
#include <pxr/usd/usd/notice.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/sdf/notice.h>
#include <pxr/usd/sdf/layer.h>

OPENDCC_NAMESPACE_OPEN

typedef std::function<void(PXR_NS::UsdNotice::ObjectsChanged const &notice)> StageObjectChangedWatcherCallback;
class OPENDCC_API StageObjectChangedWatcher : public PXR_NS::TfWeakBase
{
public:
    StageObjectChangedWatcher(const PXR_NS::UsdStageRefPtr &stage, StageObjectChangedWatcherCallback callback_fn);
    virtual ~StageObjectChangedWatcher();

    void block_notifications(bool enable);

private:
    void onObjectsChanged(PXR_NS::UsdNotice::ObjectsChanged const &notice, PXR_NS::UsdStageWeakPtr const &sender);

    PXR_NS::UsdStageRefPtr m_stage;
    PXR_NS::TfNotice::Key m_object_change_key;
    StageObjectChangedWatcherCallback m_callback_fn;
    bool m_block_notifications = false;
};

typedef std::function<void(PXR_NS::UsdNotice::StageEditTargetChanged const &notice)> StageEditTargetChangedWatcherCallback;
class OPENDCC_API StageEditTargetChangedWatcher : public PXR_NS::TfWeakBase
{
public:
    StageEditTargetChangedWatcher(const PXR_NS::UsdStageRefPtr &stage, StageEditTargetChangedWatcherCallback callback_fn);
    virtual ~StageEditTargetChangedWatcher();

private:
    void onStageEditTargetChanged(PXR_NS::UsdNotice::StageEditTargetChanged const &notice, PXR_NS::UsdStageWeakPtr const &sender);

    PXR_NS::UsdStageRefPtr m_stage;
    PXR_NS::TfNotice::Key m_edit_target_Change_key;
    StageEditTargetChangedWatcherCallback m_callback_fn;
};

typedef std::function<void(PXR_NS::SdfNotice::LayerDirtinessChanged const &notice)> SdfLayerDirtinessChangedWatcherCallback;
class OPENDCC_API SdfLayerDirtinessChangedWatcher : public PXR_NS::TfWeakBase
{
public:
    SdfLayerDirtinessChangedWatcher(PXR_NS::SdfLayerHandle &layer, SdfLayerDirtinessChangedWatcherCallback callback_fn);
    virtual ~SdfLayerDirtinessChangedWatcher();

private:
    void OnChangeNotice(const PXR_NS::SdfNotice::LayerDirtinessChanged &notice, const PXR_NS::SdfLayerHandle &sender);
    PXR_NS::SdfLayerHandle m_layer;
    PXR_NS::TfNotice::Key m_layer_dirty_change_key;
    SdfLayerDirtinessChangedWatcherCallback m_callback_fn;
};

OPENDCC_NAMESPACE_CLOSE
