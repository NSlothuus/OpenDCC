/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "api.h"
#include <functional>
#include <tuple>
#include <memory>
#include <vector>

#include <QApplication>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QWindow>

#include "opendcc/ui/common_widgets/ramp_widget.h"
#include "opendcc/ui/common_widgets/canvas_widget.h"

OPENDCC_NAMESPACE_OPEN
/**
 * @brief This class represents a custom QComboBox that does not receive focus.
 *
 * It also includes overridden methods for showing and hiding the popup. Additionally,
 * the class defines two signals: ``focus_in()`` and ``focus_out()``, which are emitted when
 * the ComboBox receives or loses focus, respectively.
 */
class OPENDCC_UI_COMMON_WIDGETS_API ComboBoxNoFocus : public QComboBox
{
    Q_OBJECT;

    bool m_popuped = false; /**< Flag to track if the popup is shown or hidden. */
public:
    /**
     * @brief Constructs a ComboBoxNoFocus widget.
     *
     * @param parent The parent widget.
     */
    ComboBoxNoFocus(QWidget* parent = 0)
        : QComboBox(parent)
    {
        setFocusPolicy(Qt::FocusPolicy::NoFocus);
    }

    /**
     * @brief Overridden method to show the popup.
     */
    virtual void showPopup() override;
    /**
     * @brief Overridden method to hide the popup.
     */
    virtual void hidePopup() override;

Q_SIGNALS:
    void focus_in(); /**< Signal emitted when the ComboBox receives focus. */
    void focus_out(); /**< Signal emitted when the ComboBox loses focus. */
};

/**
 * @brief Widget for selecting colors
 *
 */
class OPENDCC_UI_COMMON_WIDGETS_API ColorWidget : public QWidget
{
    Q_OBJECT;

    /**
     * @brief Event filter for color picking
     */
    class ColorPickingEventFilter : public QObject
    {
    public:
        /**
         * @brief Constructs a ColorPickingEventFilter for the specified color widget.
         *
         * @param color_widget The color widget.
         */
        ColorPickingEventFilter(ColorWidget* color_widget);

        /**
         * @brief Overridden event filter method.
         *
         * @param obj The object being filtered.
         * @param event The event being filtered.
         * @return True if the event is handled, false otherwise.
         */
        bool eventFilter(QObject* obj, QEvent* event) override;

    private:
        ColorWidget* m_color_widget;
    };
    /**
     * @brief Widget for screen color picking
     */
    class ScreenColorPickingWidget : public QWidget
    {
    public:
        /**
         * @brief Constructs a ScreenColorPickingWidget for the specified QWidget.
         *
         * @param parent The parent widget.
         */
        ScreenColorPickingWidget(QWidget* parent = nullptr);

        /**
         * @brief Sets the current color.
         *
         * @param current_color The current color.
         */
        void set_current_color(const QColor& current_color);
        /**
         * @brief Set the previous color.
         *
         * @param previous_color The previous color.
         */
        void set_previous_color(const QColor& previous_color);

    private:
        QColor m_current_color;
        QColor m_previous_color;
        QLabel* m_current_r;
        QLabel* m_current_g;
        QLabel* m_current_b;
        QLabel* m_previous_r;
        QLabel* m_previous_g;
        QLabel* m_previous_b;
    };

protected:
    enum ControlVariant
    {
        HSV,
        RGB,
        BOX
    };
    qreal H = 212.f / 360.f, S = 1, V = 0.46;

    qreal R = 0., G = 0.21, B = 0.48;

    qreal A = 1;
    QColor m_prev_color = QColor::fromRgbF(0.2, 0.67, 0.39, 1);
    CanvasWidget* m_current_palette = nullptr;
    bool m_achromatic = false; // shades of gray
    QColor m_prev_chromatic = QColor::fromHsvF(H, S, V, A);

private:
    bool m_enable_alpha = false;
    std::weak_ptr<QColor> m_palette_color_ptr;

    std::function<void(double val)> m_HUE_changed;
    std::function<void(double val)> m_SAT_changed;
    std::function<void(double val)> m_VAL_changed;

    QWindow m_dummy_transparent_window;
    QColor m_before_screen_color_picking;
    ColorPickingEventFilter* m_color_picking_event_filter = nullptr;
    QTimer* m_eye_dropper_timer = nullptr;
    QScreen* m_prev_screen = nullptr;
    ScreenColorPickingWidget m_color_picking_widget;

protected:
    ComboBoxNoFocus* m_pick_variant;

    std::tuple<CanvasWidget*, FloatWidget*, QLabel*> m_HUE_pair;
    std::tuple<CanvasWidget*, FloatWidget*, QLabel*> m_SAT_pair;
    std::tuple<CanvasWidget*, FloatWidget*, QLabel*> m_VAL_pair;
    std::tuple<CanvasWidget*, FloatWidget*, QLabel*> m_ALP_pair;
    CanvasWidget* m_box_alpha;
    CanvasWidget* m_box_sat_val;
    CanvasWidget* m_box_hue;
    CanvasWidget* m_color_box;
    CanvasWidget* m_prev_color_box;

    static std::vector<QColor> m_palette;
    std::vector<CanvasWidget*> m_palette_boxes;

    bool m_change_in_progress = false;

private:
    std::tuple<CanvasWidget*, FloatWidget*, QLabel*> create_color_bar(QVBoxLayout* layout, const QString& label);

    void setup_hue();
    void setup_sat();
    void setup_val();

    void setup_R();
    void setup_G();
    void setup_B();
    void setup_alpha();

    void update_color();

    void init_box(QLayout* layout);
    void init_box_sat_val(QLayout* layout);
    void init_box_alpha(QLayout* layout);

    void show_box();
    void show_line_controls();

    void setup_palette(QBoxLayout* main_layout);
    void setup_preview(QBoxLayout* layout);

    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;

    bool on_color_picking_mouse_move(QMouseEvent* event);
    bool on_color_picking_mouse_button_release(QMouseEvent* event);
    bool on_color_picking_key_press(QKeyEvent* event);
    void update_color_picking();
    QColor grab_screen_color(const QPoint& p);
    bool is_achromatic(const QColor& color);

private Q_SLOTS:
    void HUE_changed(double val);
    void SAT_changed(double val);
    void VAL_changed(double val);
    void ALP_changed(double val);
    void change_color();
    void pick_screen_color();

public:
    ColorWidget(QWidget* parent, bool enable_alpha = false);

    void color(const QColor& val, bool update_prev = true);
    QColor color();

    bool in_popup() { return false; }

Q_SIGNALS:
    void changing_color();
    void color_changed();
    void focus_in();
    void focus_out();
};

class OPENDCC_UI_COMMON_WIDGETS_API ColorPickDialog : public QWidget
{
    Q_OBJECT;
    bool m_in_popup = false;
    ColorWidget* m_color_editor;
    QTimer* m_close_timer = nullptr;

public:
    ColorPickDialog(QWidget* parent = 0, bool enable_alpha = false, ColorWidget* color_editor = nullptr);
    QColor color();
    void color(const QColor& val);

    virtual void paintEvent(QPaintEvent*) override;
    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void showEvent(QShowEvent* event) override;

Q_SIGNALS:
    void changing_color();
    void color_changed();
};

class OPENDCC_UI_COMMON_WIDGETS_API ColorButton : public QWidget
{
    Q_OBJECT;

    CanvasWidget* m_value_editor = 0;
    QColor m_select_color = { 0, 0, 0 };

public:
    ColorButton(QWidget* parent, bool enable_alpha = false, ColorPickDialog* dialog = nullptr);
    QColor color() const;
    double red();
    double green();
    double blue();
    double alpha();

public Q_SLOTS:
    void color(QColor val);
    void red(double val);
    void green(double val);
    void blue(double val);
    void alpha(double val);

Q_SIGNALS:
    void changing();
    void color_changed();
};

class OPENDCC_UI_COMMON_WIDGETS_API TinyColorWidget : public QWidget
{
public:
    TinyColorWidget(QWidget* parent = 0, bool enable_alpha = false);
    ColorButton* color_button;
};

OPENDCC_NAMESPACE_CLOSE
