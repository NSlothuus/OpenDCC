/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
// workaround
#include "opendcc/base/qt_python.h"

#include <map>
#include <memory>
#include <QObject>

#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QActionGroup>
#include <QMenuBar>
#include "api.h"
#include "spline_widget.h"
#include "snap_window.h"
#include "channel_editor.h"

#include <qundostack.h>

OPENDCC_NAMESPACE_OPEN

class GRAPH_EDITOR_API GraphEditor : public QWidget
{
    Q_OBJECT

public:
    GraphEditor(QWidget* parent = nullptr);
    ~GraphEditor();

private:
    void create_actions();
    QMenu* create_tangents_menus(bool add_break_tangents_buttons);
    void create_toolbar();
    void create_menubar();
    void create_context_menu();

    QMenuBar* menu_bar;

    ChannelEditor* curves_list_widget;

    SplineWidget* spline_widget;
    SnapWindow* snap_window;

    QLineEdit* line_edit_time;
    QLineEdit* line_edit_value;

    QCheckBox is_draw_infinity;

    QHBoxLayout* main_hbox_layout;
    QActionGroup* mode_tools;
    QToolBar* tool_bar;
    QAction* set_region_tools;
    QAction* set_insert_keys_tools;
    QActionGroup* fit_tools;
    QAction* fit_all_to_widget;
    QAction* fit_selection_to_widget;

    QActionGroup* spline_type_action_group;

    QAction* fixed_type;
    QAction* linear_type;
    QAction* flat_type;
    QAction* spline_type;
    QAction* stepped_type;
    QAction* stepped_next_type;
    QAction* plateau_type;
    QAction* clamped_type;
    QAction* auto_type;
    QAction* mix_type;

    QAction* fixed_type_in;
    QAction* linear_type_in;
    QAction* flat_type_in;
    QAction* spline_type_in;
    QAction* stepped_type_in;
    QAction* stepped_next_type_in;
    QAction* plateau_type_in;
    QAction* clamped_type_in;
    QAction* auto_type_in;
    QAction* mix_type_in;

    QAction* fixed_type_out;
    QAction* linear_type_out;
    QAction* flat_type_out;
    QAction* spline_type_out;
    QAction* stepped_type_out;
    QAction* stepped_next_type_out;
    QAction* plateau_type_out;
    QAction* clamped_type_out;
    QAction* auto_type_out;
    QAction* mix_type_out;

    QActionGroup* break_unify_tangents;
    QAction* break_tangents;
    QAction* unify_tangents;

    QAction* time_snap;
    QAction* value_snap;
    QAction* delete_selection;

    QAction* show_infinity;

    QAction* show_pre_infinity_cycle;
    QAction* show_pre_infinity_cycle_with_offset;
    QAction* show_post_infinity_cycle;
    QAction* show_post_infinity_cycle_with_offset;

    QAction* pre_infinity_cycle;
    QAction* pre_infinity_cycle_with_offset;
    QAction* pre_infinity_oscillate;
    QAction* pre_infinity_linear;
    QAction* pre_infinity_constant;

    QAction* post_infinity_cycle;
    QAction* post_infinity_cycle_with_offset;
    QAction* post_infinity_oscillate;
    QAction* post_infinity_linear;
    QAction* post_infinity_constant;

    QAction* snap_selection;
    QAction* euler_filter;
    QAction* save_on_current_layer;
    QAction* set_key;
    QAction* set_key_on_translate;
    QAction* set_key_on_rotate;
    QAction* set_key_on_scale;
    QMenu* context_menu;

Q_SIGNALS:

public Q_SLOTS:
    void set_current_time(double);
    void clear();

private Q_SLOTS:

    void curves_selection_changed();
    void test();
    void splines_selection_changed();
    void time_line_edit_editing_finished();
    void value_line_edit_editing_finished();
    // void load_curves_from_file();
    void set_region_tools_mode(QAction* emitor);
    void update_tangents_type(QAction* emitor);
    void update_in_tangents_type(QAction* emitor);
    void update_out_tangents_type(QAction* emitor);
    void update_break_unify_mod(QAction* emitor);
    void show_context_menu(QContextMenuEvent* event);

    void set_pre_infinity_cycle();
    void set_pre_infinity_cycle_with_offset();
    void set_pre_infinity_oscillate();
    void set_pre_infinity_linear();
    void set_pre_infinity_constant();

    void set_post_infinity_cycle();
    void set_post_infinity_cycle_with_offset();
    void set_post_infinity_oscillate();
    void set_post_infinity_linear();
    void set_post_infinity_constant();

    void set_and_show_pre_infinity_cycle();
    void set_and_show_pre_infinity_cycle_with_offset();
    void set_and_show_post_infinity_cycle();
    void set_and_show_post_infinity_cycle_with_offset();
    void snap_selection_slot();

    void fit_all_to_widget_slot();
    void fit_selection_to_widget_slot();

    void attach_to_application();
    void update_content();

protected:
    bool event(QEvent* event) override;

private:
    std::set<OPENDCC_NAMESPACE::AnimEngine::CurveId> selected_curves();
    void current_stage_changed();
    void current_time_changed();
    void current_stage_closed();
    void create_anim_engine_events();
    void clear_anim_engine_events();
    OPENDCC_NAMESPACE::AnimEnginePtr m_current_engine;
    AnimEngineOptionChanged::Handle m_anim_engine_option_changed;
    std::map<QAction*, adsk::TangentType> spline_type_action_ptr_to_type_ind;
    std::map<adsk::TangentType, QAction*> spline_type_ind_to_type_action_ptr;
    std::map<OPENDCC_NAMESPACE::AnimEngine::EventType, AnimEngine::CurveUpdateCallbackHandle> m_events;
    std::map<Application::EventType, Application::CallbackHandle> m_application_events_handles;
};

OPENDCC_NAMESPACE_CLOSE
