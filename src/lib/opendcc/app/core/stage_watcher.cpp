// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/core/stage_watcher.h"
#include <iostream>
OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

StageObjectChangedWatcher::StageObjectChangedWatcher(const PXR_NS::UsdStageRefPtr &stage, StageObjectChangedWatcherCallback callback_fn)
{
    m_stage = stage;
    m_callback_fn = callback_fn;
    m_object_change_key = TfNotice::Register(TfWeakPtr<StageObjectChangedWatcher>(this), &StageObjectChangedWatcher::onObjectsChanged, m_stage);
}

StageObjectChangedWatcher::~StageObjectChangedWatcher()
{
    TfNotice::Revoke(m_object_change_key);
}

void StageObjectChangedWatcher::block_notifications(bool enable)
{
    m_block_notifications = enable;
}

void StageObjectChangedWatcher::onObjectsChanged(PXR_NS::UsdNotice::ObjectsChanged const &notice, PXR_NS::UsdStageWeakPtr const &sender)
{
    if (!sender || sender != m_stage || m_block_notifications)
        return;

    m_callback_fn(notice);
}

StageEditTargetChangedWatcher::StageEditTargetChangedWatcher(const PXR_NS::UsdStageRefPtr &stage, StageEditTargetChangedWatcherCallback callback_fn)
{
    m_stage = stage;
    m_callback_fn = callback_fn;
    m_edit_target_Change_key =
        TfNotice::Register(TfWeakPtr<StageEditTargetChangedWatcher>(this), &StageEditTargetChangedWatcher::onStageEditTargetChanged, m_stage);
}

void StageEditTargetChangedWatcher::onStageEditTargetChanged(PXR_NS::UsdNotice::StageEditTargetChanged const &notice,
                                                             PXR_NS::UsdStageWeakPtr const &sender)
{
    if (!sender || sender != m_stage)
        return;

    m_callback_fn(notice);
}

StageEditTargetChangedWatcher::~StageEditTargetChangedWatcher() {}

SdfLayerDirtinessChangedWatcher::SdfLayerDirtinessChangedWatcher(PXR_NS::SdfLayerHandle &layer, SdfLayerDirtinessChangedWatcherCallback callback_fn)
{
    m_layer = layer;
    m_callback_fn = callback_fn;

    m_layer_dirty_change_key =
        TfNotice::Register(TfWeakPtr<SdfLayerDirtinessChangedWatcher>(this), &SdfLayerDirtinessChangedWatcher::OnChangeNotice, m_layer);
}

SdfLayerDirtinessChangedWatcher::~SdfLayerDirtinessChangedWatcher() {}

void SdfLayerDirtinessChangedWatcher::OnChangeNotice(const PXR_NS::SdfNotice::LayerDirtinessChanged &notice, const PXR_NS::SdfLayerHandle &sender)
{
    if (!sender || sender != m_layer)
        return;

    m_callback_fn(notice);
}

OPENDCC_NAMESPACE_CLOSE
