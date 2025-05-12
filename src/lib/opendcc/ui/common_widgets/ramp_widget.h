/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"
#include <QWidget>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QBoxLayout>
#include <memory>

#include "opendcc/ui/common_widgets/ramp.h"

OPENDCC_NAMESPACE_OPEN

/**
 * @brief The RampWidget class represents a custom widget for displaying a ramp.
 *
 * It provides functionality for ramp adding/removing points,
 * also providing access to the selected point and the ramp solver.
 */
class OPENDCC_UI_COMMON_WIDGETS_API RampWidget : public QWidget
{
    Q_OBJECT;

    QRectF m_ramp_rect;
    int m_selected = 0;
    int m_hovered = 0; // id of a point to highlight
    int m_active = 0; // id of the Ramp's active point
    std::shared_ptr<Ramp<float>> m_ramp = 0;

    QColor m_back_color;
    QColor m_ramp_color;
    QColor m_ramp_line;

    static QSizeF const s_point_size; // both round point and square remove point
    static int const s_point_border_width;
    static int const s_point_active_zone; // surrounding area width

public:
    RampWidget();

    /**
     * @brief Gets the index of the selected point on the ramp.
     *
     * @return The index of the selected point.
     */
    int selected() const { return m_selected; }

    /**
     * @brief Gets the ramp solver associated with the widget.
     *
     * @return The ramp solver.
     */
    std::shared_ptr<Ramp<float>> solver() const { return m_ramp; }
    /**
     * @brief Sets the ramp solver associated with the widget.
     *
     * @param val The ramp solver to set.
     */
    void solver(std::shared_ptr<Ramp<float>> val) { m_ramp = val; }

    /**
     * @brief Finds the index of the point closest to the specified coordinates.
     *
     * @param point_x The x-coordinate of the point.
     * @param point_y The y-coordinate of the point.
     *
     * @return The index of the closest point.
     */
    int find_point(float point_x, float point_y);
    /**
     * @brief Finds the index of the point closest to the specified coordinates and is marked for removal.
     *
     * @param point_x The x-coordinate of the point.
     * @param point_y The y-coordinate of the point.
     *
     * @return The index of the closest point marked for removal.
     */
    int find_point_to_remove(float point_x, float point_y);

    /**
     * @brief Removes the point at the specified index from the ramp.
     *
     * @param point The index of the point to remove.
     */
    void remove_point(int point);
    /**
     * @brief Adds a point with the specified coordinates to the ramp.
     *
     * @param point_x The x-coordinate of the point to add.
     * @param point_y The y-coordinate of the point to add.
     */
    void add_point(float point_x, float point_y);

protected:
    virtual void paintEvent(QPaintEvent*) override;
    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;
    virtual void mouseMoveEvent(QMouseEvent*) override;
    virtual void leaveEvent(QEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

Q_SIGNALS:

    void point_selected(int index);
    void start_changing();
    void changing();
    void end_changing();

private:
    void update_ramp_rect();
};

class OPENDCC_UI_COMMON_WIDGETS_API FloatWidget : public QDoubleSpinBox
{
    Q_OBJECT;

    bool m_change = false;
    float m_from = 0;

public:
    FloatWidget(QWidget* parent);

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    virtual void focusInEvent(QFocusEvent*) override;
    virtual void focusOutEvent(QFocusEvent*) override;
    virtual void fixup(QString& str) const override;
    QValidator::State validate(QString& input, int& pos) const override;

Q_SIGNALS:
    void focus_in(QFocusEvent*);
    void focus_out(QFocusEvent*);
    void mouseReleaseSignal(QMouseEvent* event);
};

class OPENDCC_UI_COMMON_WIDGETS_API RampEditor : public QWidget
{
    Q_OBJECT;

    std::shared_ptr<Ramp<float>> m_ramp;
    RampWidget* m_ramp_widget;
    QComboBox* m_combo_box;

    int m_selected_point = 0;
    FloatWidget* m_value_editor;
    FloatWidget* m_pos_editor;

protected Q_SLOTS:
    void point_selected(int point);
    void point_update();
    void type_changed(int index);
    void value_changed(double val);
    void position_changed(double val);

public:
    RampEditor(std::shared_ptr<Ramp<float>> ramp);

Q_SIGNALS:
    void value_changed();

private:
    void setup_position_editor(QBoxLayout* layout);
    void setup_value_editor(QBoxLayout* layout);
    void setup_interpolation_widget(QBoxLayout* layout);
};

class OPENDCC_UI_COMMON_WIDGETS_API RampPoint
{
public:
    float pos;
    float val;
    int inter;

    RampPoint(float _pos, float _val, int _inter)
        : pos(_pos)
        , val(_val)
        , inter(_inter)
    {
    }
};

class OPENDCC_UI_COMMON_WIDGETS_API RampEdit : public QWidget
{
    Q_OBJECT;

    std::shared_ptr<Ramp<float>> m_ramp;
    RampWidget* m_ramp_widget;
    QComboBox* m_combo_box;

    int m_selected_point = 0;
    FloatWidget* m_value_editor;
    FloatWidget* m_pos_editor;

protected Q_SLOTS:
    void point_select(int point);
    void point_update();
    void type_changed(int index);
    void value_changed(double val);
    void position_changed(double val);

public:
    RampEdit();

    RampWidget* get_ramp();
    void clear();
    void add_point(RampPoint);
    int points_count();
    RampPoint point(int i);

Q_SIGNALS:
    void point_selected(int index);
    void changed();
};

OPENDCC_NAMESPACE_CLOSE
