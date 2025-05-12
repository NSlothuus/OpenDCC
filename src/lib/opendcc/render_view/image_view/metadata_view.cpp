// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "metadata_view.h"
#include "app.h"

#include <QVBoxLayout>
#include <QTreeWidget>

OPENDCC_NAMESPACE_OPEN

RenderViewMetadataView::RenderViewMetadataView(RenderViewMainWindow* app)
{
    m_app = app;
    setLayout(new QVBoxLayout);

    m_tree_widget = new QTreeWidget;

    layout()->addWidget(m_tree_widget);
    QStringList headers;
    headers << i18n("render_view.metadata_view", "name");
    headers << i18n("render_view.metadata_view", "value");
    m_tree_widget->setAlternatingRowColors(true);
    m_tree_widget->setHeaderLabels(headers);
}

RenderViewMetadataView::~RenderViewMetadataView() {}

void RenderViewMetadataView::update_metadata()
{
    m_tree_widget->clear();

    auto image = m_app->get_current_image();
    if (image)
    {
        auto image_spec = image->spec();
        for (int i = 0; i < image_spec.extra_attribs.size(); i++)
        {
            QStringList item_data;
            item_data << image_spec.extra_attribs[i].name().c_str();

            auto string_data = image_spec.extra_attribs[i].get_string();

            const int big_string_data = 1024;
            if (string_data.size() < big_string_data)
            {
                item_data << string_data.c_str();
            }
            else
            {
                item_data << i18n("render_view.metadata_view", "<Big Data>");
            }
            new QTreeWidgetItem(m_tree_widget, item_data);
        }
    }
}

OPENDCC_NAMESPACE_CLOSE
