// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "range_slider.h"

#include <QPainter>
#include <QVector2D>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDebug>
#include <cmath>

#include "opendcc/ui/color_theme/color_theme.h"

OPENDCC_NAMESPACE_OPEN

static int signum(const double value)
{
    return value >= 0 ? 1 : -1;
}

static const int minimum_width = 200;
static const int minimum_height = 26;

static const int horizontal_margin_in_pixel = 3;
static const int vertical_margin_in_pixel = 3;

static const double slider_x_radius = 5.0f;
static const double slider_y_radius = 5.0f;

static const double visible_timeline_x_radius = 3.5f;
static const double visible_timeline_y_radius = 3.5f;

RangeSlider::RangeSlider(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    setMinimumSize(QSize(minimum_width, minimum_height));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if (get_color_theme() == ColorTheme::DARK)
    {
        m_background_color = QColor(41, 41, 41);
        m_visible_timeline_color = QColor(80, 80, 80);
        m_slider_color = QColor(100, 100, 100);
        m_selected_slider_color = QColor(130, 130, 130);
        m_text_color = QColor(150, 150, 150);
    }
    else
    {
        m_background_color = palette().window().color();
        m_visible_timeline_color = palette().base().color();
        m_slider_color = palette().light().color();
        m_selected_slider_color = m_background_color;
        m_text_color = QColor(59, 59, 59);
    }
}

double RangeSlider::get_start_time() const
{
    return m_start_time;
}

double RangeSlider::get_end_time() const
{
    return m_end_time;
}

double RangeSlider::get_current_start_time() const
{
    return m_current_start_time;
}

double RangeSlider::get_current_end_time() const
{
    return m_current_end_time;
}

bool RangeSlider::slider_moving() const
{
    return m_slider_moving;
}

void RangeSlider::set_start_time(double start_time)
{
    if (qFuzzyCompare(m_start_time, start_time))
    {
        return;
    }

    m_start_time = start_time;

    emit start_time_changed(m_start_time);

    if (m_start_time > m_current_start_time)
    {
        set_current_start_time(m_start_time);
    }

    if (m_start_time >= m_end_time)
    {
        set_end_time(m_start_time + 1);
    }

    update();
}

void RangeSlider::set_end_time(double end_time)
{
    if (qFuzzyCompare(m_end_time, end_time))
    {
        return;
    }

    m_end_time = end_time;

    emit end_time_changed(m_end_time);

    if (m_end_time < m_current_end_time)
    {
        set_current_end_time(m_end_time);
    }

    if (m_end_time <= m_start_time)
    {
        set_start_time(m_end_time - 1);
    }

    update();
}

void RangeSlider::set_current_start_time(double current_start_time)
{
    if (qFuzzyCompare(m_current_start_time, current_start_time))
    {
        return;
    }

    m_current_start_time = current_start_time;

    emit current_start_time_changed(m_current_start_time);

    if (m_current_start_time < m_start_time)
    {
        set_start_time(m_current_start_time);
    }

    if (m_current_start_time >= m_current_end_time)
    {
        set_current_end_time(m_current_start_time + 1);
    }

    update();
}

void RangeSlider::set_current_end_time(double current_end_time)
{
    if (qFuzzyCompare(m_current_end_time, current_end_time))
    {
        return;
    }

    m_current_end_time = current_end_time;

    emit current_end_time_changed(m_current_end_time);

    if (m_current_end_time <= m_current_start_time)
    {
        set_current_start_time(m_current_end_time - 1);
    }

    if (m_current_end_time > m_end_time)
    {
        set_end_time(m_current_end_time);
    }

    update();
}

void RangeSlider::set_time_display(TimeDisplay mode)
{
    m_time_display = mode;

    update();
}

void RangeSlider::set_fps(double fps)
{
    m_fps = fps;

    update();
}

void RangeSlider::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    paint_background();
    paint_visible_timeline();
    paint_sliders();
    paint_sliders_value();
}

void RangeSlider::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);

    m_slider_moving = true;

    m_previous_pos = m_current_pos;
    m_current_pos = event->pos();

    update_elements();
}

void RangeSlider::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);

    if (event->button() & Qt::LeftButton)
    {
        m_current_pos = event->pos();
        m_previous_pos = m_current_pos;

        m_selected_element = get_selected_element(m_current_pos);

        QRectF selected_element_rect = get_element_rect(m_selected_element);

        m_first_click_pos_in_rect = QPoint(m_current_pos.x() - selected_element_rect.left(), m_current_pos.y() - selected_element_rect.bottom());

        update_elements();
    }
}

void RangeSlider::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);

    if (event->button() & Qt::LeftButton)
    {
        m_slider_moving = false;

        if (m_selected_element == ElementType::VisibleTimeline)
        {
            emit range_changed(m_current_start_time, m_current_end_time);
            emit start_time_changed(m_start_time);
            emit end_time_changed(m_end_time);
        }
        else
        {
            emit current_start_time_changed(m_current_start_time);
            emit current_end_time_changed(m_current_end_time);
            emit start_time_changed(m_start_time);
            emit end_time_changed(m_end_time);
        }

        m_selected_element = ElementType::None;

        update_elements();
    }
}

void RangeSlider::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);

    if (event->button() & Qt::LeftButton)
    {
        ElementType click_on = get_selected_element(event->pos());

        switch (click_on)
        {
        case RangeSlider::ElementType::StartSlider:
        case RangeSlider::ElementType::EndSlider:
        case RangeSlider::ElementType::VisibleTimeline:
        {
            if (m_current_start_time != m_start_time || m_current_end_time != m_end_time)
            {
                m_previous_start_time = m_current_start_time;
                m_previous_end_time = m_current_end_time;

                m_current_start_time = m_start_time;
                m_current_end_time = m_end_time;

                emit range_changed(m_current_start_time, m_current_end_time);
            }
            else
            {
                m_current_start_time = m_previous_start_time;
                m_current_end_time = m_previous_end_time;

                emit range_changed(m_current_start_time, m_current_end_time);
            }
            break;
        }
        case RangeSlider::ElementType::None:
        default:
        {
            break;
        }
        }
    }
}

void RangeSlider::paint_background()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::NoBrush);

    painter.fillRect(rect(), m_background_color);
}

void RangeSlider::paint_visible_timeline()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (get_color_theme() == ColorTheme::LIGHT)
    {
        painter.setPen(QPen(QColor(205, 205, 205), 1));
    }
    else
    {
        painter.setPen(Qt::NoPen);
    }
    painter.setBrush(Qt::NoBrush);

    const QRectF visible_timeline = get_element_rect(ElementType::VisibleTimeline);

    QPainterPath visible_timeline_path;
    visible_timeline_path.addRoundedRect(visible_timeline, visible_timeline_x_radius, visible_timeline_y_radius);

    painter.fillPath(visible_timeline_path, m_visible_timeline_color);
    painter.drawPath(visible_timeline_path);
}

void RangeSlider::paint_sliders()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (get_color_theme() == ColorTheme::LIGHT)
    {
        painter.setPen(QPen(QColor(205, 205, 205), 1));
    }
    else
    {
        painter.setPen(Qt::NoPen);
    }
    painter.setBrush(Qt::NoBrush);

    // start_slider
    const QRectF start_slider = get_element_rect(ElementType::StartSlider);

    QPainterPath start_slider_path;
    start_slider_path.addRoundedRect(start_slider, slider_x_radius, slider_y_radius);

    painter.fillPath(start_slider_path, m_selected_element == ElementType::StartSlider ? m_selected_slider_color : m_slider_color);
    painter.drawPath(start_slider_path);

    // end_slider
    const QRectF end_slider = get_element_rect(ElementType::EndSlider);

    QPainterPath end_slider_path;
    end_slider_path.addRoundedRect(end_slider, slider_x_radius, slider_y_radius);

    painter.fillPath(end_slider_path, m_selected_element == ElementType::EndSlider ? m_selected_slider_color : m_slider_color);
    painter.drawPath(end_slider_path);
}

void RangeSlider::paint_sliders_value()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setPen(m_text_color);

    const double widget_y_center = height() / 2.0;

    const double slider_size_in_pixel = compute_slider_size();

    const double min_x_start_slider = x_by_time(m_current_start_time);
    const double max_x_start_slider = min_x_start_slider + slider_size_in_pixel;
    const double min_x_end_slider = x_by_time(m_current_end_time);

    QString start_time_text;
    QString end_time_text;

    if (m_time_display == OPENDCC_NAMESPACE::TimeDisplay::Timecode)
    {
        start_time_text = to_timecode(m_current_start_time, m_fps);
        end_time_text = to_timecode(m_current_end_time, m_fps);
    }
    else
    {
        start_time_text = QString::number(m_current_start_time);
        end_time_text = QString::number(m_current_end_time);
    }

    QFontMetrics metrics = painter.fontMetrics();

    const int text_padding = horizontal_margin_in_pixel + 2;

    double start_text_max_x = max_x_start_slider + text_padding + metrics.horizontalAdvance(start_time_text);
    double end_text_min_x = min_x_end_slider - text_padding - metrics.horizontalAdvance(end_time_text);

    const double baseline = widget_y_center + metrics.capHeight() / 2.0;

    QRectF end_rect = get_element_rect(ElementType::EndSlider);

    if (start_text_max_x > end_rect.left())
    {
        return;
    }

    painter.drawText(max_x_start_slider + text_padding, baseline, start_time_text);

    if (end_text_min_x < start_text_max_x)
    {
        return;
    }

    painter.drawText(min_x_end_slider - text_padding - metrics.horizontalAdvance(end_time_text), baseline, end_time_text);
}

void RangeSlider::update_elements()
{
    switch (m_selected_element)
    {
    case RangeSlider::ElementType::StartSlider:
    {
        const double current_x_pos = m_current_pos.x();

        const double new_time =
            std::floor(time_by_x(current_x_pos - m_first_click_pos_in_rect.x())) + (m_current_start_time - std::floor(m_current_start_time));

        if (new_time < m_start_time)
        {
            set_current_start_time(m_start_time);
        }
        else if (new_time >= m_current_end_time)
        {
            set_current_start_time(m_current_end_time - 1);
        }
        else
        {
            set_current_start_time(new_time);
        }

        break;
    }
    case RangeSlider::ElementType::EndSlider:
    {
        const double current_x_pos = m_current_pos.x();

        const double new_time =
            std::floor(time_by_x(current_x_pos - m_first_click_pos_in_rect.x())) + (m_current_end_time - std::floor(m_current_end_time));

        if (new_time > m_end_time)
        {
            set_current_end_time(m_end_time);
        }
        else if (new_time <= m_current_start_time)
        {
            set_current_end_time(m_current_start_time + 1);
        }
        else
        {
            set_current_end_time(new_time);
        }

        break;
    }
    case RangeSlider::ElementType::VisibleTimeline:
    {
        const double previous_time = qRound(time_by_x(m_previous_pos.x()));
        const double new_time = qRound(time_by_x(m_current_pos.x()));

        if (previous_time != new_time)
        {
            const double start_end_distance = m_current_end_time - m_current_start_time;
            const double start_prev_distance = previous_time - m_current_start_time;
            const double end_prev_distance = m_current_end_time - previous_time;

            const double new_start_time = new_time - start_prev_distance;
            const double new_end_time = new_time + end_prev_distance;

            if (new_start_time < m_start_time)
            {
                set_current_start_time(m_start_time);
                set_current_end_time(m_start_time + start_end_distance);
            }
            else if (new_end_time > m_end_time)
            {
                set_current_start_time(m_end_time - start_end_distance);
                set_current_end_time(m_end_time);
            }
            else
            {
                set_current_start_time(new_start_time);
                set_current_end_time(new_end_time);
            }
        }

        break;
    }
    case RangeSlider::ElementType::None:
    default:
    {
        break;
    }
    }

    update();
}

RangeSlider::ElementType RangeSlider::get_selected_element(const QPoint& pos)
{
    const QRectF start_slider = get_element_rect(ElementType::StartSlider);
    const QRectF end_slider = get_element_rect(ElementType::EndSlider);
    const QRectF visible_timeline = get_element_rect(ElementType::VisibleTimeline);

    const bool start_slider_contains = start_slider.contains(pos);
    const bool end_slider_contains = end_slider.contains(pos);
    const bool visible_timeline_contains = visible_timeline.contains(pos);

    if (start_slider_contains && end_slider_contains)
    {
        return (pos.x() - start_slider.left() < end_slider.right() - pos.x()) ? ElementType::StartSlider : ElementType::EndSlider;
    }
    else if (start_slider_contains)
    {
        return ElementType::StartSlider;
    }
    else if (end_slider_contains)
    {
        return ElementType::EndSlider;
    }
    else if (visible_timeline_contains)
    {
        return ElementType::VisibleTimeline;
    }
    else
    {
        return ElementType::None;
    }
}

QRectF RangeSlider::get_element_rect(ElementType type)
{
    switch (type)
    {
    case RangeSlider::ElementType::StartSlider:
    {
        return QRectF(x_by_time(m_current_start_time), compute_slider_y_pos(), compute_slider_width(), compute_slider_height());
    }
    case RangeSlider::ElementType::EndSlider:
    {
        return QRectF(x_by_time(m_current_end_time), compute_slider_y_pos(), compute_slider_width(), compute_slider_height());
    }
    case RangeSlider::ElementType::VisibleTimeline:
    {
        return QRectF(x_by_time(m_current_start_time) - horizontal_margin_in_pixel, compute_slider_y_pos() - vertical_margin_in_pixel,
                      compute_visible_timeline_width(), compute_visible_timeline_height());
    }
    case RangeSlider::ElementType::None:
    default:
    {
        return QRectF();
    }
    }
}

double RangeSlider::compute_slider_size()
{
    return height() - 4.0 * vertical_margin_in_pixel;
}

double RangeSlider::x_by_time(const double time)
{
    const double min_x_pos_start = 2.0 * horizontal_margin_in_pixel;

    return min_x_pos_start + (time - m_start_time) * compute_step();
}

double RangeSlider::time_by_x(const double x)
{
    const double min_x_pos_start = 2.0 * horizontal_margin_in_pixel;

    return (x - min_x_pos_start) / compute_step() + m_start_time;
}

double RangeSlider::compute_step()
{
    const double width = this->width();

    const double slider_size_in_pixel = compute_slider_size();

    const double min_x_pos_start = 2.0 * horizontal_margin_in_pixel;
    const double max_x_pos_end = width - 2.0 * horizontal_margin_in_pixel - slider_size_in_pixel;

    return (max_x_pos_end - min_x_pos_start) / (m_end_time - m_start_time);
}

double RangeSlider::compute_slider_width()
{
    return compute_slider_size();
}

double RangeSlider::compute_slider_height()
{
    return compute_slider_size();
}

double RangeSlider::compute_visible_timeline_width()
{
    const double slider_size_in_pixel = compute_slider_size();

    const double start_slider_min_x = x_by_time(m_current_start_time);
    const double end_slider_min_x = x_by_time(m_current_end_time);
    const double end_slider_max_x = end_slider_min_x + slider_size_in_pixel;

    return end_slider_max_x - start_slider_min_x + 2.0 * horizontal_margin_in_pixel;
}

double RangeSlider::compute_visible_timeline_height()
{
    const double slider_size_in_pixel = compute_slider_size();

    return slider_size_in_pixel + 2.0 * vertical_margin_in_pixel;
}

double RangeSlider::compute_slider_y_pos()
{
    const double slider_size_in_pixel = compute_slider_size();

    const double widget_y_center = height() / 2.0;

    return widget_y_center - slider_size_in_pixel / 2;
}

OPENDCC_NAMESPACE_CLOSE
