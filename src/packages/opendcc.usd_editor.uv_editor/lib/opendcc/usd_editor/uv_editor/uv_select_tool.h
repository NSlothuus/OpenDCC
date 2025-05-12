/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/usd_editor/uv_editor/uv_tool.h"
#include "opendcc/app/core/application.h"
#include "opendcc/ui/common_widgets/ramp.h"

#include <QMouseEvent>
#include <QMenu>

OPENDCC_NAMESPACE_OPEN

class RoundMarkingMenu;

class UvSelectTool : public UvTool
{
public:
    UvSelectTool(UVEditorGLWidget* widget);
    ~UvSelectTool();

    virtual bool on_mouse_press(QMouseEvent* event) override;
    virtual bool on_mouse_move(QMouseEvent* event) override;
    virtual bool on_mouse_release(QMouseEvent* event) override;

    virtual void draw(ViewportUiDrawManager* draw_manager) override;

    virtual bool is_working() const override;

private:
    enum class SelectionMode
    {
        Add,
        Remove,
        Replace,
        None,
    };
    static std::string selection_mode_to_string(const SelectionMode mode);

    int m_start_pos_x = 0;
    int m_start_pos_y = 0;

    int m_current_pos_x = 0;
    int m_current_pos_y = 0;

    SelectionMode m_mode = SelectionMode::None;

    RoundMarkingMenu* m_selection_mode_marking_menu = nullptr;
    QActionGroup* m_selection_mode_action_group = nullptr;
    QMenu* m_selection_mode_menu = nullptr;
    std::shared_ptr<Ramp<float>> m_falloff_curve_ramp;
    std::shared_ptr<Ramp<PXR_NS::GfVec3f>> m_falloff_color_ramp;
    std::function<float(float)> m_falloff_fn;
    std::function<PXR_NS::GfVec3f(float)> m_falloff_color_fn;

    Application::CallbackHandle m_selection_mode_changed_cid;
};

OPENDCC_NAMESPACE_CLOSE
