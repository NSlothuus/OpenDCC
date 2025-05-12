// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/common_widgets/rollout_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVariant>
#include <QMouseEvent>

OPENDCC_NAMESPACE_OPEN

RolloutWidget::RolloutWidget(QString title, bool expandable /*= true*/, QWidget* parent /*= nullptr*/)
    : QWidget(parent)
{
    m_expandable = expandable;
    setAutoFillBackground(true);
    auto layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    QString style = R""""(
.rollout-widget-header-label {
    font-weight: bold;
}

.rollout-widget-header-label:disabled {
    color: #a6a6a6;
}

.rollout-widget-header {
    background: rgba(0, 0, 0, 40);
}

.rollout-widget-header:disabled {
    
}

.rollout-widget-content {
    /*border: 1px solid #373737;*/
    background: rgba(0, 0, 0, 15);
}
)""""; // #373737

    if (expandable)
    {
        style += R""""(
.rollout-widget-header:hover
{
    border: 1px solid #5b5b5b;
}
)"""";
    }

    setStyleSheet(style);

    m_header = new QWidget;
    m_header->setAutoFillBackground(true);
    m_header->setProperty("class", "rollout-widget-header");
    auto header_layout = new QHBoxLayout;
    header_layout->setContentsMargins(1, 1, 1, 1);
    header_layout->setSpacing(2);
    m_header->setLayout(header_layout);
    m_header->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);

    m_arrow = new QLabel;

    header_layout->addWidget(m_arrow);
    auto label = new QLabel(title);
    label->setProperty("class", "rollout-widget-header-label");
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    header_layout->addWidget(label);
    header_layout->addStretch();

    m_content = new QWidget;
    m_content->setAutoFillBackground(true);
    m_content->setProperty("class", "rollout-widget-content");

    layout->addWidget(m_header);
    layout->addWidget(m_content);
    layout->setAlignment(Qt::AlignTop);

    set_expanded(true);

    QSizePolicy size_policy = m_arrow->sizePolicy();
    size_policy.setRetainSizeWhenHidden(true);
    m_arrow->setSizePolicy(size_policy);

    if (!expandable)
    {
        m_arrow->hide();
    }
    else
    {
        connect(this, &RolloutWidget::clicked, [this](bool expanded) { set_expanded(!expanded); });
    }
}

RolloutWidget::~RolloutWidget() {}

void RolloutWidget::set_expanded(bool expanded)
{
    if (expanded)
    {
        auto arrow = QPixmap(":icons/dd_open.png");
        m_arrow->setPixmap(arrow.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_expanded = expanded;
        m_content->show();
    }
    else
    {
        auto arrow = QPixmap(":icons/dd_close.png");
        m_arrow->setPixmap(arrow.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_expanded = expanded;
        m_content->hide();
    }
}

void RolloutWidget::set_layout(QLayout* layout)
{
    m_content->setLayout(layout);
}

void RolloutWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_header->rect().contains(event->pos()))
    {
        m_header_pressed = true;
    }
    return QWidget::mousePressEvent(event);
}

void RolloutWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_header_pressed)
    {
        if (m_header->rect().contains(event->pos()))
        {
            emit clicked(m_expanded);
        }
    }
    m_header_pressed = false;

    return QWidget::mouseReleaseEvent(event);
}
OPENDCC_NAMESPACE_CLOSE
