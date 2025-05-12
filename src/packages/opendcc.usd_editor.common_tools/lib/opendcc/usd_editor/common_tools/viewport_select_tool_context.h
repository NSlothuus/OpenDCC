/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/ramp.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/selection_list.h"
#include "opendcc/app/viewport/iviewport_tool_context.h"

#include <QObject>
#include <QActionGroup>

#include <memory>

namespace PXR_NS
{
    TF_DECLARE_PUBLIC_TOKENS(SelectToolTokens, ((name, "select_tool")));
};

class QMenu;

OPENDCC_NAMESPACE_OPEN

class RoundMarkingMenu;
class HalfEdgeCache;

class ViewportSelectToolContext
    : public QObject
    , public IViewportToolContext
{
    Q_OBJECT;

public:
    ViewportSelectToolContext();
    ~ViewportSelectToolContext();

    virtual bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_double_click(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                       ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                               ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) override;
    virtual bool on_key_press(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    virtual bool on_key_release(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

    virtual PXR_NS::TfToken get_name() const override;
    std::shared_ptr<Ramp<float>> get_falloff_curve_ramp() const;
    std::shared_ptr<Ramp<PXR_NS::GfVec3f>> get_falloff_color_ramp() const;
    void update_falloff_curve_ramp();
    void update_falloff_color_ramp();

    QActionGroup* get_selection_mode_action_group();

    const PXR_NS::TfToken& get_selection_kind() const;
    void set_selection_kind(const PXR_NS::TfToken& selection_kind);

protected:
    bool is_locked() const;
    bool m_select_rect_mode;
    bool m_shift;
    bool m_ctrl;

private:
    bool edge_loop_selection();
    bool topology_selection();

    int m_start_posx = 0;
    int m_start_posy = 0;

    int m_mousex = 0;
    int m_mousey = 0;
    float m_start_falloff_radius = 5.0f;
    float m_cur_falloff_radius = 5.0f;
    std::unique_ptr<PXR_NS::GfVec3f> m_centroid;
    PXR_NS::GfVec3d m_start_world_pos;
    bool m_adjust_soft_selection_radius = false;
    bool m_draw_soft_selection_radius = false;
    bool m_double_click = false;
    unsigned long long m_key_press_timepoint;

    static bool s_factory_registration;

    QMenu* m_selection_mode_menu = nullptr;
    RoundMarkingMenu* m_selection_mode_marking_menu = nullptr;
    SelectionList m_marking_menu_selection;
    Application::CallbackHandle m_selection_mode_changed_cid;
    std::shared_ptr<Ramp<float>> m_falloff_curve_ramp;
    std::shared_ptr<Ramp<PXR_NS::GfVec3f>> m_falloff_color_ramp;
    std::function<float(float)> m_falloff_fn;
    std::function<PXR_NS::GfVec3f(float)> m_falloff_color_fn;

    QActionGroup* m_selection_mode_action_group = nullptr;

    PXR_NS::TfToken m_selection_kind;
    Settings::SettingChangedHandle m_selection_kind_changed;

    SelectionList m_double_click_selection;
    SelectionList m_last_selection;
};

OPENDCC_NAMESPACE_CLOSE
