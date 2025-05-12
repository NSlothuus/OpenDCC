// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <QVariant>
#include <QSizePolicy>

#include "opendcc/ui/common_widgets/group_widget.h"
#include <QPainter>
#include <QEvent>

OPENDCC_NAMESPACE_OPEN

void GroupWidget::add_widget(QWidget* widget)
{
    widget->setParent(this);
    m_content_layout->addWidget(widget);
    m_widgets.push_back(widget);
}

bool GroupWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease)
        switch_group();
    return false;
}

void GroupWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}

void GroupWidget::name(const QString& val)
{
    m_name->setVisible(!val.isEmpty());
    m_name->setText(val);
}

void GroupWidget::desc(const QString& val)
{
    if (m_desc)
        m_desc->setText(val);
}

void GroupWidget::clear()
{
    if (m_content_widget)
    {
        m_content_widget->hide();
        m_content_widget->setParent(nullptr);
        m_content_widget->deleteLater();
    }

    m_content_widget = new QWidget(m_data_widget);
    m_content_widget->setProperty("class", "group-widget-content");
    // m_content_widget->setStyleSheet("QWidget{background-color:rgb(42, 42, 42);}");
    m_content_widget->setContentsMargins(0, 0, 0, 0);
    m_content_layout = new QVBoxLayout(m_content_widget);
    m_content_layout->setContentsMargins(0, 0, 0, 0);
    m_content_layout->setSpacing(0);
    m_content_widget->setLayout(m_content_layout);
    m_content_widget->setContentsMargins(5, 3, 0, 0);
    m_data_layout->addWidget(m_content_widget);
}

GroupWidget::GroupWidget(const QString& name, const QString& desc, QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/)
    : QWidget(parent, flags)
{
    m_desc = new QLabel(desc);
    m_desc->setStyleSheet("QLabel{font-size: 13px; font-style: bold;}");
    setup(name, m_desc, 0);
}

GroupWidget::GroupWidget(const QString& name, QWidget* desc, QWidget* icon, QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/)
    : QWidget(parent, flags)
{
    setup(name, desc, icon);
}

GroupWidget::GroupWidget(QWidget* icon, const QString& name, const QString& desc, QWidget* parent /*= 0*/, Qt::WindowFlags flags /*= 0*/)
    : QWidget(parent, flags)
{
    m_desc = new QLabel(desc);
    m_desc->setStyleSheet("QLabel{font-size: 13px; font-style: bold;}");
    setup(name, m_desc, icon);
}

void GroupWidget::setup(const QString& name, QWidget* desc, QWidget* icon)
{
    // setStyleSheet("QWidget{background-color:rgb(50, 62, 64);}");
    setAutoFillBackground(true);
    setContentsMargins(0, 0, 0, 0);
    auto back_layout = new QVBoxLayout();
    back_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(back_layout);

    auto main_wid = new QWidget(this);
    main_wid->setProperty("class", "group-widget");
    main_wid->setContentsMargins(1, 1, 0, 0);
    main_wid->setLayout(m_main_layout = new QVBoxLayout());
    back_layout->addWidget(main_wid);
    main_wid->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);

    m_main_layout->setSpacing(0);
    auto head = new QWidget(this);
    head->setProperty("class", "group-widget-head");
    head->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
    auto header = new QHBoxLayout(head);
    header->setSizeConstraint(QLayout::SetMinimumSize);
    m_main_layout->addWidget(head);
    m_main_layout->setContentsMargins(0, 0, 0, 0);
    header->setContentsMargins(2, 0, 20, 0);
    header->setSpacing(1);
    head->installEventFilter(this);
    if (icon)
        header->addWidget(icon);

    m_open_btn = new QPushButton(this);
    m_open_btn->setProperty("class", "group-widget-open-btn");
    m_open_btn->setFixedSize(20, 20);
    m_open_btn->setStyleSheet("QPushButton{font-size: 16px; font-style: bold; outline: 0;}");
    // m_open_btn->setText("-");
    m_open_btn->setIcon(QIcon(":icons/dd_open.png"));
    m_open_btn->setFlat(true);
    header->addWidget(m_open_btn);
    QObject::connect(m_open_btn, &QPushButton::clicked, m_open_btn, [this](bool) { switch_group(); });

    header->addWidget(m_name = new QLabel(name));
    m_name->setStyleSheet("QLabel{font-size: 13px; font-style: bold; border: none; background-color: rgba(0, 0, 0, 0); }");
    // m_name->setReadOnly(true);
    m_name->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    if (name.isEmpty())
        m_name->setVisible(false);
    if (desc)
        header->addWidget(desc);

    m_data_widget = new QWidget(this);
    m_data_widget->setProperty("class", "data-widget");
    m_data_widget->setContentsMargins(0, 0, 0, 0);
    m_data_widget->setAutoFillBackground(true);
    m_data_widget->setContentsMargins(1, 0, 0, 0);

    m_data_layout = new QVBoxLayout(m_data_widget);
    m_data_layout->setContentsMargins(1, 0, 0, 0);

    m_main_layout->addWidget(m_data_widget);

    clear();
}

void GroupWidget::switch_group()
{
    if (m_content_widget->isHidden())
    {
        m_content_visible = true;
        m_open_btn->setIcon(QIcon(":icons/dd_open.png"));
        m_content_widget->setVisible(m_content_visible);
        open_state_changed(m_content_visible);
    }
    else
    {
        m_content_visible = false;
        m_open_btn->setIcon(QIcon(":icons/dd_close.png"));
        m_content_widget->setVisible(m_content_visible);
        open_state_changed(m_content_visible);
    }
}

void GroupWidget::set_close(bool is_closed)
{
    if (is_closed)
    {
        if (!m_content_widget->isHidden())
        {
            m_content_visible = false;
            m_open_btn->setIcon(QIcon(":icons/dd_close.png"));
            m_content_widget->setVisible(m_content_visible);
            open_state_changed(m_content_visible);
        }
    }
    else if (m_content_widget->isHidden())
    {
        m_content_visible = true;
        m_open_btn->setIcon(QIcon(":icons/dd_open.png"));
        m_content_widget->setVisible(m_content_visible);
        open_state_changed(m_content_visible);
    }
}

OPENDCC_NAMESPACE_CLOSE
