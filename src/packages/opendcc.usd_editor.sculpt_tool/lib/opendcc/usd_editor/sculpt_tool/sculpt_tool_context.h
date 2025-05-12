/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
// workaround
#include "opendcc/base/qt_python.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/viewport/iviewport_tool_context.h"

#include <pxr/base/gf/vec3f.h>

#include "opendcc/usd_editor/sculpt_tool/sculpt_strategies.h"
#include "opendcc/usd_editor/sculpt_tool/mesh_manipulation_data.h"
#include "opendcc/usd_editor/sculpt_tool/sculpt_properties.h"

OPENDCC_NAMESPACE_OPEN

class SculptToolContext
    : public IViewportToolContext
    , public PXR_NS::TfWeakBase
{
public:
    SculptToolContext();
    virtual ~SculptToolContext();

    virtual bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                               ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) override;
    virtual bool on_key_press(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    virtual bool on_key_release(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

    virtual QCursor* get_cursor() override;

    void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    virtual PXR_NS::TfToken get_name() const override;
    void set_on_mesh_changed_callback(std::function<void()> on_mesh_changed);

    Properties properties() const;
    void set_properties(const Properties& properties);
    static std::string settings_prefix();
    bool empty() const;

private:
    void update_sculpt_strategy(const Mode& mode);

    void adjust_radius();
    void update_context();
    void on_objects_changed(const PXR_NS::UsdNotice::ObjectsChanged& notice, const PXR_NS::UsdStageWeakPtr& sender);

    bool m_ignore_stage_changing = false;
    bool m_is_b_key_pressed = false;
    bool m_is_adjust_radius = false;

    PXR_NS::TfNotice::Key m_objects_changed_notice_key;

    float m_start_radius = 1.0f;
    int m_start_x = 0;
    int m_current_x = 0;

    std::shared_ptr<UndoMeshManipulationData> m_mesh_data;
    std::unique_ptr<SculptStrategy> m_sculpt_strategy;

    QCursor* m_cursor;

    std::function<void()> m_on_mesh_changed = []() {
    };

    Application::CallbackHandle m_selection_event_hndl;
    Application::CallbackHandle m_current_stage_changed_event_hndl;
};

OPENDCC_NAMESPACE_CLOSE
