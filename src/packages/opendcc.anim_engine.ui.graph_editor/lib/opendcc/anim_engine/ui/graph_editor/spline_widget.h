/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/qt_python.h"

#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QUndoStack>
#include <QUndoCommand>

#include <set>
#include <map>
#include <vector>
#include <memory>

#include "opendcc/anim_engine/core/anim_engine_curve.h"
#include "opendcc/anim_engine/core/engine.h"

#include "selection_event_dispatcher.h"

OPENDCC_NAMESPACE_OPEN

class SplineWidgetCommand;
class ChangeSelectionCommand;

namespace math
{
    struct vec2
    {
        double x;
        double y;
    };
}

class SplineWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    enum class Mode
    {
        RegionTools,
        InsertKeys
    };

    enum class ApplyGroup : uint32_t
    {
        Selected,
        All
    };

    struct CurveData
    {
        std::set<adsk::KeyId> selected_keys;
        std::set<SelectedTangent> selected_tangents;
        std::map<SelectedTangent, math::vec2> tangent_pivots;
        std::map<adsk::KeyId, math::vec2> key_pivots;
        QColor color;
    };

    SplineWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~SplineWidget();

    void set_displayed_curves(const std::set<AnimEngine::CurveId>& curves_ids);
    void clear();

    void update_tangents_type(adsk::TangentType type, bool update_in, bool update_out);
    void set_mode(Mode mode);
    void get_selection_info(double& time, double& value, bool& is_time_define, bool& is_value_define, std::set<adsk::TangentType>& tangents_types);
    bool have_selection() const;

Q_SIGNALS:
    void context_menu_event_signal(QContextMenuEvent* event);
    void keyframe_moved();
    void selection_changed();

public Q_SLOTS:
    void set_time_to_selection(double time);
    void set_value_to_selection(double value);
    void set_is_tangents_break(bool is_tangents_break_);
    void set_is_draw_infinity(bool is_draw_infinity_);
    void set_auto_snap_time_interval(double spun_time_interval_);
    void set_auto_snap_value_interval(double spun_value_interval_);
    void set_is_auto_snap_time(bool is_auto_snap_time_);
    void set_is_auto_snap_value(bool is_auto_snap_value_);
    void set_pre_infinity(adsk::InfinityType pre_inf, ApplyGroup apply_group = ApplyGroup::Selected);
    void set_post_infinity(adsk::InfinityType post_inf, ApplyGroup apply_group = ApplyGroup::Selected);
    void set_current_time(double current_time_);
    void set_current_engine(AnimEnginePtr m_current_engine);

    void add_keyframes(double time, adsk::TangentType in_tangent_type, adsk::TangentType out_tangent_type);
    void delete_selected_keyframes();
    void snap_selection(bool is_snap_time, bool is_snap_value);
    void fit_to_widget(bool fit_all);

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    enum class SelectedState : uint32_t
    {
        no_selected_area,
        start_selected,
        start_moving
    };

    void set_infinity(adsk::InfinityType pre_inf, bool is_pre_infinity, ApplyGroup apply_group = ApplyGroup::Selected);
    void set_attribute_to_selection(double attr_value, bool attr_is_time);

    void draw();
    void draw_splines();
    void draw_grid();
    void draw_key_points();
    void draw_tangents();
    void draw_line_in_pixels(math::vec2 pos0, math::vec2 pos1, float z, QColor color);
    void draw_point_on_screen(math::vec2 screen_coordinate, float z, QColor color);
    void draw_rectangle_on_screen(math::vec2 left_down, math::vec2 right_up, float z, QColor color, bool edge = false);
    void draw_selected_area();
    void draw_text(double x, double y, const QString& text, QColor color);
    void draw_strip_line(const std::vector<float>& data, float z, QColor color, bool dotted = false);
    void draw_line(const std::vector<float>& data, float z, QColor color, GLenum mode);

    math::vec2 world_to_screen(math::vec2 world);
    math::vec2 screen_to_world(math::vec2 screen);
    math::vec2 screen_to_fragment(math::vec2 screen);
    float world_x_to_screen_x(float world_x);
    float world_y_to_screen_y(float world_y);
    float screen_x_to_world_x(float screen_x);
    float screen_y_to_world_y(float screen_y);
    float screen_x_to_fragment_x(float screen_x);
    float screen_y_to_fragment_y(float screen_y);

    void move_selected_keys();
    void move_selected_tangents(float pos_x, float pos_y);

    void change_zoom(float value);
    bool curve_is_selected(AnimEngine::CurveId id, float x_left_selected, float x_right_selected, float y_bottom_selected, float y_top_selected,
                           float dx);

    void update_selection(QMouseEvent* event);
    void update_pivots(AnimEngineCurveCPtr curve, CurveData& curve_data);
    AnimEngine::CurveIdToKeyframesMap get_selected_keys();
    AnimEngine::CurveIdToKeysIdsMap get_selected_keys_ids();

    bool is_cursor_on_key_or_tangent_pivot(AnimEngine::CurveId curve_id, float pos_x, float pos_y);

    math::vec2 in_tangent_pos(const adsk::Keyframe& keyframe);
    math::vec2 out_tangent_pos(const adsk::Keyframe& keyframe);

    float compute_grid_step(float t_min, float t_max, uint64_t num_pixels);

    void start_command(const std::string& command_name);
    void end_command();

    void enable_gl_debug();

    void initialize_grid();

    void initialize_screen_rectangle();

    void initialize_line();

    std::map<AnimEngine::CurveId, CurveData> m_displayed_curves_map;
    std::map<AnimEngine::EventType, AnimEngine::CurveUpdateCallbackHandle> m_events;
    std::map<AnimEngine::EventType, AnimEngine::KeysListUpdateCallbackHandle> m_keys_events;
    std::map<Application::EventType, Application::CallbackHandle> m_app_events_handles;

    SelectionEventDispatcherHandle m_selection_callback_handle;

    SelectedState m_selected_state = SelectedState::no_selected_area;

    Qt::KeyboardModifiers m_current_modifiers;
    Qt::MouseButtons m_current_mouse_buttons;
    QPoint m_current_mouse_pos;

    double m_snap_x_interval = 1.0f;
    double m_snap_y_interval = 1.0f;

    double m_x_left_selected;
    double m_x_right_selected;
    double m_y_bottom_selected;
    double m_y_top_selected;

    double m_x_left;
    double m_x_right;
    double m_y_bottom;
    double m_y_top;

    double m_last_pos_x;
    double m_last_pos_y;

    bool m_is_tangents_break = false;
    bool m_is_draw_infinity = false;
    bool m_is_insert_key = false;
    bool m_is_auto_snap_value = false;
    bool m_is_auto_snap_time = false;

    float m_current_time = 0;
    float m_insert_key_position = 0;

    Mode m_mode = Mode::RegionTools;
    AnimEnginePtr m_current_engine;
    std::shared_ptr<SplineWidgetCommand> m_current_command;
    std::shared_ptr<ChangeKeyframesCommand> m_key_changed_command;

    // grid
    QOpenGLShaderProgram m_grid_program;

    GLuint m_grid_draw_arrays_count;

    GLuint m_grid_z_location;

    GLuint m_grid_color_location;
    GLuint m_grid_axis_color_location;
    GLuint m_grid_current_time_color_location;
    GLuint m_grid_insert_key_color_location;

    GLuint m_grid_rectangle_size_location;
    GLuint m_grid_origin_location;
    GLuint m_grid_current_time_x_location;
    GLuint m_grid_insert_key_x_location;
    GLuint m_grid_insert_key_location;

    // screen_rectangle
    QOpenGLShaderProgram m_rectangle_program;

    GLuint m_rectangle_draw_arrays_count;

    GLuint m_rectangle_z_location;

    GLuint m_rectangle_color_location;

    GLuint m_rectangle_left_down_coordinate_location;
    GLuint m_rectangle_right_up_coordinate_location;

    GLuint m_rectangle_need_draw_edge_location;

    // line
    QOpenGLShaderProgram m_line_program;
    QOpenGLVertexArrayObject m_line_vao;
    QOpenGLBuffer m_line_vbo;
    GLuint m_line_vbo_capacity = 500;

    GLuint m_line_in_color_location;
    GLuint m_line_z_location;
    GLuint m_line_x_left_location;
    GLuint m_line_x_right_location;
    GLuint m_line_y_bottom_location;
    GLuint m_line_y_top_location;

    GLuint m_line_coord_location;
};

OPENDCC_NAMESPACE_CLOSE
