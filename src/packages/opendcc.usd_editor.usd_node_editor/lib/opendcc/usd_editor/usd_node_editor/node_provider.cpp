// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "node_provider.h"
#include "opendcc/usd_editor/usd_node_editor/graph_model.h"
#include <iostream>

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

NodeProvider::NodeProvider(UsdGraphModel& model)
    : m_model(model)
{
    m_stage_changed_cid = Application::instance().register_event_callback(Application::EventType::CURRENT_STAGE_CHANGED, [this] {
        init_stage_listeners(Application::instance().get_session()->get_current_stage());
        m_model.stage_changed_impl();
    });
    m_reparent_cid = commands::ParentPrimCommand::get_notifier().register_handle([this](SdfPathVector old_names, SdfPathVector new_names) {
        m_perform_rename = true;
        m_old_rename_paths = old_names;
        m_new_rename_paths = new_names;
    });
    m_rename_cid = commands::RenamePrimCommand::get_notifier().register_handle([this](SdfPath old_name, SdfPath new_name) {
        m_perform_rename = true;
        m_old_rename_paths = { old_name };
        m_new_rename_paths = { new_name };
    });
    init_stage_listeners(Application::instance().get_session()->get_current_stage());
}

void NodeProvider::rename_performed()
{
    m_perform_rename = false;
    m_old_rename_paths.clear();
    m_new_rename_paths.clear();
}

void NodeProvider::block_notifications(bool block)
{
    if (m_stage_watcher)
        m_stage_watcher->block_notifications(block);
}

void NodeProvider::on_prim_resynced(const PXR_NS::SdfPath& path)
{
    assert(get_stage());

    if (should_perform_rename())
        get_model().on_rename();
    if (get_model().get_root().IsEmpty())
        return;

    const auto prim = get_stage()->GetPrimAtPath(path);
    if (prim)
        get_model().try_add_prim(path);
    else
        get_model().try_remove_prim(path);
}

void NodeProvider::on_prop_changed(const PXR_NS::SdfPath& path)
{
    assert(get_stage());
    assert(path.IsPropertyPath());

    if (should_perform_rename())
        get_model().on_rename();
    if (get_model().get_root().IsEmpty())
        return;

    get_model().try_update_prop(path);
}

void NodeProvider::init_stage_listeners(const PXR_NS::UsdStageRefPtr& stage)
{
    m_stage = stage;
    if (m_stage)
    {
        m_stage_watcher = std::make_unique<StageObjectChangedWatcher>(
            m_stage, [this](PXR_NS::UsdNotice::ObjectsChanged const& notice) { on_stage_object_changed(notice); });
    }
    else
    {
        m_stage_watcher.reset();
    }
}

void NodeProvider::on_stage_object_changed(const PXR_NS::UsdNotice::ObjectsChanged& notice)
{
    for (const auto path : notice.GetResyncedPaths())
    {
        if (path.IsPrimPath())
            on_prim_resynced(path);
        else if (path.IsPropertyPath())
            on_prop_changed(path);
    }
    for (const auto path : notice.GetChangedInfoOnlyPaths())
    {
        if (path.IsPropertyPath())
            on_prop_changed(path);
    }
}

NodeProvider::~NodeProvider()
{
    Application::instance().unregister_event_callback(Application::EventType::CURRENT_STAGE_CHANGED, m_stage_changed_cid);
    commands::ParentPrimCommand::get_notifier().unregister_handle(m_reparent_cid);
    commands::RenamePrimCommand::get_notifier().unregister_handle(m_rename_cid);
}

OPENDCC_NAMESPACE_CLOSE
