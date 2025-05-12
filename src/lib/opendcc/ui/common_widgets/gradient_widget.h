/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"

#include <QWidget>
#include <QPushButton>
#include <pxr/base/gf/vec3f.h>
#include "opendcc/ui/common_widgets/ramp_widget.h"
#include "opendcc/ui/common_widgets/color_widget.h"

OPENDCC_NAMESPACE_OPEN
/**
 * @brief The GradientWidget class is a custom widget for displaying and editing color gradients.
 */
class OPENDCC_UI_COMMON_WIDGETS_API GradientWidget : public QWidget
{
    Q_OBJECT;
    using RampV3f = Ramp<PXR_NS::GfVec3f>;

public:
    GradientWidget();
    /**
     * @brief Constructs a GradientWidget object with the specified color ramp.
     *
     * @param color_ramp The color ramp to set.
     */
    GradientWidget(std::shared_ptr<RampV3f> color_ramp);

    /**
     * @brief Gets the index of the selected point.
     *
     * @return The index of the selected point.
     */
    int selected() const { return m_selected; }
    /**
     * @brief Gets the color ramp.
     *
     * @return The color ramp.
     */
    const RampV3f& get_color_ramp() const { return *m_color_ramp; }
    /**
     * @brief Removes the point at the specified index.
     *
     * @param ind The index of the point to remove.
     */
    void remove_point(int ind);
    /**
     * @brief Adds a point at the specified position.
     *
     * @param pos The position of the point to add.
     */
    void add_point(double pos);
    /**
     * @brief Adds a point at the specified position with the specified
     * color and interpolation type.
     *
     * @param pos The position of the point to add.
     * @param color The color of the point to add.
     * @param interp The interpolation type of the point to add.
     */
    void add_point(double pos, const PXR_NS::GfVec3f& color, RampV3f::InterpType interp);
    /**
     * @brief Gets the point at the specified index.
     *
     * @param id The index of the point to get.
     * @return The point at the specified index.
     */
    const RampV3f::CV& get_point(int id) const;
    /**
     * @brief Gets the point at the specified index.
     *
     * @param id The index of the point to get.
     * @return The point at the specified index.
     */
    RampV3f::CV& get_point(int id);
    /**
     * @brief Clears the gradient widget.
     */
    void clear();

protected:
    virtual void paintEvent(QPaintEvent*) override;
    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void mouseMoveEvent(QMouseEvent*) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;
    virtual void leaveEvent(QEvent*) override;
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    int m_selected = -1;
    int m_hovered = -1; // id of a point to highlight
    int m_active = -1; // id of the Ramp's active point

    QRectF m_gradient_rect;
    std::shared_ptr<RampV3f> m_color_ramp = nullptr;

    static QSizeF const s_point_size; // both round point and square remove point
    static int const s_point_border;
    static int const s_hovered_point_border; // wider than normal
    static int const s_point_active_zone; // surrounding area width

    int find_point(float point_x, float point_y);
    int find_point_to_remove(float point_x, float point_y);
    int find_point_helper(float x, float y, int h);

    void fill_image(QImage& dest, int w, int h);
    void update_gradient_rect();

Q_SIGNALS:

    /**
     * @brief Signal emitted when a point is selected.
     *
     * @param index The index of the selected point.
     */
    void point_selected(int index);
    /**
     * @brief Signal emitted when the user starts changing a point.
     */
    void start_changing();
    /**
     * @brief Signal emitted while the user is changing a point.
     */
    void changing();
    /**
     * @brief Signal emitted when the user finishes changing a point.
     */
    void end_changing();
};

/**
 * @brief The GradientEditor class is a custom widget for editing color gradients.
 */
class OPENDCC_UI_COMMON_WIDGETS_API GradientEditor : public QWidget
{
    Q_OBJECT;

    using RampV3f = Ramp<PXR_NS::GfVec3f>;
    GradientWidget* m_gradient_widget;
    FloatWidget* m_pos_editor;
    CanvasWidget* m_value_editor;
    int m_selected = -1;
    PXR_NS::GfVec3f m_select_color = PXR_NS::GfVec3f(100 / 255.0, 175 / 255.0, 234 / 255.0);
    QComboBox* m_combo_box;
    bool m_changing = false;

public:
    /**
     * @brief Constructs a GradientEditor object with an optional ColorPickDialog.
     *
     * @param color_dialog The ColorPickDialog to set.
     */
    GradientEditor(ColorPickDialog* color_dialog = nullptr);
    /**
     * @brief Constructs a GradientEditor object with the specified color ramp
     * and an optional ColorPickDialog.
     *
     * @param color_ramp The color ramp to set.
     * @param color_dialog The ColorPickDialog to set.
     */
    GradientEditor(std::shared_ptr<RampV3f> color_ramp, ColorPickDialog* color_dialog = nullptr);

    /**
     * @brief Clears the gradient editor.
     */
    void clear();
    /**
     * @brief Adds a point to the gradient editor with the specified position,
     * color, and interpolation type.
     *
     * @param pos The position of the point to add.
     * @param color The color of the point to add.
     * @param interp The interpolation type of the point to add.
     */
    void add_point(double pos, const PXR_NS::GfVec3f& color, RampV3f::InterpType interp);
    /**
     * @brief Adds a point to the gradient editor with the specified position,
     * color, and interpolation type.
     *
     * @param pos The position of the point to add.
     * @param r The red component of the color.
     * @param g The green component of the color.
     * @param b The blue component of the color.
     * @param interp The interpolation type of the point to add.
     */
    void add_point(double pos, float r, float g, float b, int interp);
    /**
     * @brief Gets the color ramp.
     *
     * @return The color ramp.
     */
    const RampV3f& get_color_ramp() const;
    /**
     * @brief Gets the number of points in the gradient editor.
     *
     * @return The number of points.
     */
    int points_count() const;
    /**
     * @brief Gets the point at the specified index.
     *
     * @param i The index of the point to get.
     * @return The point at the specified index.
     */
    const RampV3f::CV& get_point(int i) const;

private Q_SLOTS:
    void slot_point_selected(int index) { point_select(index); };
    void slot_start_changing()
    {
        m_changing = true;
        start_changing();
    };
    void slot_changing() { changing(); };
    void slot_end_changing()
    {
        m_changing = false;
        end_changing();
    };

protected Q_SLOTS:
    void position_changed(double val);
    void point_selected(int ind);
    void point_update();

Q_SIGNALS:
    /**
     * @brief Signal emitted when a point is selected.
     *
     * @param index The index of the selected point.
     */
    void point_select(int index);
    /**
     * @brief Signal emitted when the user starts changing a point.
     */
    void start_changing();
    /**
     * @brief Signal emitted while the user is changing a point.
     */
    void changing();
    /**
     * @brief Signal emitted when the user finishes changing a point.
     */
    void end_changing();

private:
    void setup_position_editor(QBoxLayout* layout);
    void setup_value_editor(QBoxLayout* layout, ColorPickDialog* color_dialog);
    void setup_interpolation_widget(QBoxLayout* layout);
};

OPENDCC_NAMESPACE_CLOSE
