/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/base/qt_python.h"
#include "opendcc/ui/node_editor/graph_model.h"
#include <pxr/usd/usd/prim.h>
#include <unordered_set>
#include <QVector>
#include <pxr/usd/usd/notice.h>
#include "opendcc/app/core/stage_watcher.h"
#include "opendcc/app/core/application.h"
#include "opendcc/usd_editor/common_cmds/rename_prim.h"
#include "opendcc/usd_editor/common_cmds/parent_prim.h"
#include "opendcc/usd_editor/usd_node_editor/graph_model.h"

OPENDCC_NAMESPACE_OPEN

class UsdGraphModel;

class OPENDCC_USD_NODE_EDITOR_API NodeProvider
{
public:
    NodeProvider(UsdGraphModel& model);
    virtual ~NodeProvider();

    PXR_NS::UsdStageRefPtr get_stage() const { return m_stage; }
    std::weak_ptr<StageObjectChangedWatcher> get_watcher() { return m_stage_watcher; }
    UsdGraphModel& get_model() { return m_model; }
    bool should_perform_rename() const { return m_perform_rename; }
    const PXR_NS::SdfPathVector& get_old_rename_paths() const { return m_old_rename_paths; }
    const PXR_NS::SdfPathVector& get_new_rename_paths() const { return m_new_rename_paths; }
    void rename_performed();
    bool is_valid() const { return m_stage; }
    void block_notifications(bool block);

protected:
    void on_prim_resynced(const PXR_NS::SdfPath& path);
    void on_prop_changed(const PXR_NS::SdfPath& path);

private:
    void init_stage_listeners(const PXR_NS::UsdStageRefPtr& stage);
    void on_stage_object_changed(const PXR_NS::UsdNotice::ObjectsChanged& notice);

    PXR_NS::UsdStageRefPtr m_stage;

    Application::CallbackHandle m_stage_changed_cid;
    std::shared_ptr<StageObjectChangedWatcher> m_stage_watcher;
    UsdGraphModel& m_model;
    commands::RenameCommandNotifier::Handle m_rename_cid;
    commands::ParentCommandNotifier::Handle m_reparent_cid;
    PXR_NS::SdfPathVector m_old_rename_paths;
    PXR_NS::SdfPathVector m_new_rename_paths;
    bool m_perform_rename = false;
};

OPENDCC_NAMESPACE_CLOSE
