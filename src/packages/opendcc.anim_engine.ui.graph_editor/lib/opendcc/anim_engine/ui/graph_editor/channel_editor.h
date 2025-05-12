/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
// workaround
#include "opendcc/base/qt_python.h"

#include <set>
#include <map>
#include <QTreeWidget>
#include "opendcc/anim_engine/core/anim_engine_curve.h"
#include "opendcc/anim_engine/core/engine.h"
#include "opendcc/app/core/undo/block.h"
#include "opendcc/anim_engine/ui/graph_editor/api.h"

OPENDCC_NAMESPACE_OPEN

class ValueEditorDelegate;
class ComponentTreeItem;
class LadderScale;

class GRAPH_EDITOR_API ChannelEditor
    : public QTreeWidget
    , public PXR_NS::TfWeakBase
{
    Q_OBJECT
public:
    ChannelEditor(bool simplified_version, QWidget* parent = nullptr);
    ~ChannelEditor();
    void update_content();
    std::set<OPENDCC_NAMESPACE::AnimEngine::CurveId> selected_curves_ids();

protected:
    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void mouseMoveEvent(QMouseEvent*) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;

private:
    friend ValueEditorDelegate;
    void on_objects_changed(PXR_NS::UsdNotice::ObjectsChanged const& notice, PXR_NS::UsdStageWeakPtr const& sender);
    void update_values();
    void set_value(double value);
    bool m_ignore_stage_changing = false;
    // ladder
    LadderScale* m_ladder = nullptr;
    bool m_activated = false;
    ComponentTreeItem* m_current_item = nullptr;
    QPoint m_pos;
    double m_start_value = 1;
    // end ladder
    PXR_NS::TfNotice::Key m_objects_changed_notice_key;
    std::map<Application::EventType, Application::CallbackHandle> m_application_events_handles;
    bool m_is_simplified_version = false;
    std::unordered_map<PXR_NS::SdfPath, QTreeWidgetItem*, PXR_NS::SdfPath::Hash> m_items_map;
    std::unique_ptr<commands::UsdEditsUndoBlock> m_undo_block;
    static void update_prim_item(QTreeWidgetItem* prim_item, PXR_NS::SdfPath path);
};

OPENDCC_NAMESPACE_CLOSE
