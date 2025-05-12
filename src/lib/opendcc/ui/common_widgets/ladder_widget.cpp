// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/common_widgets/ladder_widget.h"

#include <QApplication>
#include <qevent.h>
#include <string>
#include <cmath>
#include <QValidator>
#include "QPainter"

OPENDCC_NAMESPACE_OPEN

LadderNumberWidget::LadderNumberWidget(QWidget* parent, bool as_int /*= false*/)
    : QLineEdit(parent)
    , m_as_int(as_int)
{
    QValidator* validator;
    if (as_int)
        validator = new QIntValidator(this);
    else
    {
        auto double_validator = new QDoubleValidator(this);
        double_validator->setLocale(QLocale("English"));
        double_validator->setNotation(QDoubleValidator::StandardNotation);
        validator = double_validator;
    }
    setValidator(validator);
    m_clamp = { std::numeric_limits<qreal>::min(), std::numeric_limits<qreal>::max() };
}

LadderNumberWidget::~LadderNumberWidget()
{
    if (m_activated)
    {
        QApplication::restoreOverrideCursor();
        delete m_ladder;
    }
}

void LadderNumberWidget::set_clamp(float min, float max)
{
    m_clamp = { min, max };
    m_enable_clamp = true;
}

void LadderNumberWidget::set_clamp_minimum(float min)
{
    m_clamp.setX(min);
    m_enable_clamp = true;
}

void LadderNumberWidget::set_clamp_maximum(float max)
{
    m_clamp.setY(max);
    m_enable_clamp = true;
}

QPointF LadderNumberWidget::clamp()
{
    return m_clamp;
}

void LadderNumberWidget::enable_clamp(bool val)
{
    m_enable_clamp = val;
}

bool LadderNumberWidget::is_clamped() const
{
    return m_enable_clamp;
}

void LadderNumberWidget::enable_marker(bool val)
{
    if (m_enable_marker == val)
        return;
    m_enable_marker = val;

    setTextMargins(m_enable_marker ? 5 : 0, 0, 0, 0);
}

void LadderNumberWidget::set_marker_color(float r, float g, float b)
{
    m_r = r;
    m_g = g;
    m_b = b;
}

void LadderNumberWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::MouseButton::MiddleButton)
    {
        QApplication::setOverrideCursor(Qt::SizeHorCursor);
        m_activated = true;
        m_pos = e->globalPos();
        e->accept();
        m_start_value = value();
        if (m_as_int)
            m_ladder = new LadderScale(this, 0.9, 1000, 10);
        else
            m_ladder = new LadderScale();
        m_ladder->updateGeometry();
        m_ladder->show();
        if (m_as_int)
            m_ladder->move(e->globalPos() - QPoint(0, m_ladder->height() - 15));
        else
            m_ladder->move(e->globalPos() - QPoint(0, m_ladder->height() / 2));
        m_ladder->pointer_changed(m_pos);
        start_changing();
    }
    else
        QLineEdit::mousePressEvent(e);
}

void LadderNumberWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (m_activated)
    {
        auto pos = e->globalPos();
        if (!m_ladder->pointer_changed(pos))
        {
            auto delta = (pos.x() - m_pos.x()) / LADDER_SENS * m_ladder->get_scale();
            qreal val = m_start_value + delta;
            if (m_enable_clamp)
                val = std::max(m_clamp.x(), std::min(val, m_clamp.y()));
            set_value(val);
            m_ladder->set_target_value(m_start_value + delta);
        }
        else
        {
            m_pos = pos;
            m_ladder->set_target_value(m_start_value);
        }
        changing();
    }
    else
        QLineEdit::mouseMoveEvent(e);
}

void LadderNumberWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_activated)
    {
        QApplication::restoreOverrideCursor();
        m_activated = false;
        delete m_ladder;
        m_ladder = nullptr;
        editingFinished();
        stop_changing();
    }
    else
        QLineEdit::mouseReleaseEvent(e);
}

void LadderNumberWidget::paintEvent(QPaintEvent* e)
{
    QLineEdit::paintEvent(e);
    if (!m_enable_marker)
        return;

    QPen pen;
    pen.setColor({ 58, 58, 58 });

    QBrush brush;
    float w = 4;
    float h = height() - 2;

    QPainter painter(this);
    painter.setPen(pen);
    brush.setColor({ 42, 42, 42 });
    brush.setStyle(Qt::BrushStyle::DiagCrossPattern);
    painter.setBrush(brush);
    painter.drawRect(1, 1, w, h);

    brush.setColor(QColor::fromRgbF(m_r, m_g, m_b));
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    painter.setBrush(brush);
    painter.drawRect(1, 1, w, h);
}

float LadderNumberWidget::value()
{
    return text().toFloat();
}

void LadderNumberWidget::set_value(float value)
{
    QString val;
    if (m_as_int)
        val.setNum(int(value));
    else
        val.setNum(value, 'g', int(std::log10(1 + std::abs(value)) + 4));

    auto deb = val.toLocal8Bit().toStdString();
    setText(val);
    setCursorPosition(0);
}

//////////////////////////////////////////////////////////////////////////

LadderScale::LadderScale(QWidget* parent /*= nullptr*/, float min /*= 0.001*/, float max /*= 1000*/, float step_size /*= 10*/)
    : QFrame(parent, Qt::ToolTip)
{
    setFrameStyle(QFrame::Box);
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setStyleSheet("background-color: rgb(42, 42, 42);");

    min = std::max(min, 0.00001f);
    step_size = std::max(0.001f, step_size);
    for (float i = max; i >= min; i /= step_size)
    {
        auto label = new LadderScaleItem(this);
        label->set_scale(i);
        if (std::abs(i - 1) < 0.0001)
        {
            label->set_active(true);
            m_active_scale = label;
        }
        label->setMinimumSize(50, 30);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label, Qt::AlignCenter);
        m_scale_items.push_back(label);

        auto line = new QFrame(this);
        line->setFixedHeight(1);
        line->setStyleSheet("background-color: rgb(255, 255, 255);");
        layout->addWidget(line);
    }
}

float LadderScale::get_scale()
{
    return m_scale;
}

bool LadderScale::pointer_changed(QPoint pos)
{
    auto local_pos = mapFromGlobal(pos);
    if (rect().contains(local_pos))
    {
        for (auto item : m_scale_items)
        {
            if (item->geometry().contains(local_pos))
            {
                for (auto label : m_scale_items)
                    label->set_active(false);
                item->set_active(true);
                m_active_scale = item;
                m_scale = item->scale();
                break;
            }
        }
        return true;
    }
    return false;
}

void LadderScale::set_target_value(float val)
{
    if (m_active_scale)
        m_active_scale->set_target_value(val);
}

//////////////////////////////////////////////////////////////////////////

LadderScaleItem::LadderScaleItem(QWidget* parent)
    : QLabel(parent)
{
}

void LadderScaleItem::set_active(bool is_active)
{
    if (m_is_active != is_active)
    {
        m_is_active = is_active;
        setWordWrap(m_is_active);
        set_scale(m_scale);
        if (m_is_active)
            setStyleSheet("background-color: rgb(76, 110, 93);");
        else
            setStyleSheet("background-color: rgb(42, 42, 42);");
    }
}

void LadderScaleItem::set_scale(float i)
{
    setText(QString::number(i));
    m_scale = i;
}

float LadderScaleItem::scale() const
{
    return m_scale;
}

void LadderScaleItem::set_target_value(float val)
{
    setText(QString::number(m_scale) + "\n" + QString::number(val, 'g', int(std::log10(1 + std::abs(val)) + 4)));
}

OPENDCC_NAMESPACE_CLOSE
