// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/bezier_tool/bezier_tool_settings_widget.h"
#include "opendcc/ui/common_widgets/rollout_widget.h"
#include "opendcc/app/viewport/tool_settings_view.h"
#include "opendcc/app/ui/application_ui.h"

#include <pxr/base/tf/diagnostic.h>

#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

REGISTER_TOOL_SETTINGS_VIEW(BezierToolTokens->name, TfToken("USD"), BezierToolContext, BezierToolSettingsWidget);

BezierToolSettingsWidget::BezierToolSettingsWidget(BezierToolContext* context)
    : m_context(context)
{
    if (!TF_VERIFY(m_context, "Invalid tool context."))
    {
        return;
    }

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    createManipSettingsRollout();
    createTangentSettingsRollout();
    createHotkeysRollout();
}

BezierToolSettingsWidget::~BezierToolSettingsWidget() {}

void BezierToolSettingsWidget::createHotkeysRollout()
{
    using Hotkeys = std::vector<std::pair<QString, QString>>;
    using Doc = std::pair<QString, Hotkeys>;
    using DocsVector = std::vector<Doc>;

    static const DocsVector docs = {
        { i18n("bezier_tool.settings.hotkeys", "Bezier Anchors"),
          {
              { i18n("bezier_tool.settings.hotkeys.anchors", "CTRL + LMB:"),
                i18n("bezier_tool.settings.hotkeys.anchors", "Reset anchor tangent handles") },
              { i18n("bezier_tool.settings.hotkeys.anchors", "CTRL + SHIFT + LMB:"),
                i18n("bezier_tool.settings.hotkeys.anchors", "Close curve (only for first anchor)") },
              { i18n("bezier_tool.settings.hotkeys.anchors", "DELETE:"), i18n("bezier_tool.settings.hotkeys.anchors", "Delete selected anchor") },
          } },
        { i18n("bezier_tool.settings.hotkeys", "Bezier Handles"),
          {
              { i18n("bezier_tool.settings.hotkeys.handles", "CTRL + LMB:"), i18n("bezier_tool.settings.hotkeys.handles", "Break tangency") },
              { i18n("bezier_tool.settings.hotkeys.handles", "SHIFT + LMB:"),
                i18n("bezier_tool.settings.hotkeys.handles", "Constrain tangent angle") },
          } },
        { i18n("bezier_tool.settings.hotkeys", "Other"),
          { { i18n("bezier_tool.settings.hotkeys.other", "MMB:"), i18n("bezier_tool.settings.hotkeys.other", "Enable manip mode") } } }
    };

    auto hotkeys = new RolloutWidget(i18n("bezier_tool.settings", "Hotkeys"));
    auto settings = Application::instance().get_settings();
    bool expanded = settings->get<bool>(std::string("viewport.bezier_tool.ui.hotkeys"), true);
    hotkeys->set_expanded(expanded);
    connect(hotkeys, &RolloutWidget::clicked, [](bool expanded) {
        auto settings = Application::instance().get_settings();
        settings->set("viewport.bezier_tool.ui.hotkeys", !expanded);
    });

    auto content_layout = new QGridLayout;
    content_layout->setColumnStretch(0, 2);
    content_layout->setColumnStretch(1, 5);

    const auto docs_size = docs.size();

    int offset = 0;

    for (int d = 0; d < docs_size; ++d)
    {
        QLabel* docs_name = new QLabel(QString("<b>%1</b>").arg(docs[d].first));

        content_layout->addWidget(docs_name, d + offset, 0, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);

        const auto& hotkeys = docs[d].second;
        const auto hotkeys_size = hotkeys.size();

        for (int h = 0; h < hotkeys_size; ++h)
        {
            QLabel* l = new QLabel(hotkeys[h].first);
            QLabel* r = new QLabel(hotkeys[h].second);

            const auto row = 1 + d + h + offset;

            content_layout->addWidget(l, row, 0, Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
            content_layout->addWidget(r, row, 1, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);
        }

        offset += hotkeys_size;
    }

    hotkeys->set_layout(content_layout);

    m_layout->addWidget(hotkeys);
}

void BezierToolSettingsWidget::createManipSettingsRollout()
{
    auto manip_settings = new RolloutWidget(i18n("bezier_tool.settings", "Manip Settings"));
    auto settings = Application::instance().get_settings();
    bool expanded = settings->get("viewport.bezier_tool.ui.manip_settings", true);
    manip_settings->set_expanded(expanded);
    connect(manip_settings, &RolloutWidget::clicked, [](bool expanded) {
        auto settings = Application::instance().get_settings();
        settings->set("viewport.bezier_tool.ui.manip_settings", !expanded);
    });

    auto label = new QLabel(i18n("bezier_tool.settings.manip", "Manip Mode:"));
    auto combobox = new QComboBox;

    combobox->addItem(i18n("bezier_tool.settings.manip", "Translate Mode"), QVariant(static_cast<int>(BezierToolContext::ManipMode::Translate)));
    combobox->addItem(i18n("bezier_tool.settings.manip", "Scale Mode"), QVariant(static_cast<int>(BezierToolContext::ManipMode::Scale)));

    const auto index = settings->get("viewport.bezier_tool.ui.manip_mode", static_cast<int>(BezierToolContext::ManipMode::Translate));
    combobox->setCurrentIndex(index);
    m_context->set_manip_mode(static_cast<BezierToolContext::ManipMode>(index));

    QObject::connect(combobox, qOverload<int>(&QComboBox::activated), this, [this, combobox](int index) {
        m_context->set_manip_mode(static_cast<BezierToolContext::ManipMode>(combobox->itemData(index).toInt()));

        auto settings = Application::instance().get_settings();
        settings->set("viewport.bezier_tool.ui.manip_mode", index);
    });

    auto content_layout = new QGridLayout;
    content_layout->setColumnStretch(0, 2);
    content_layout->setColumnStretch(1, 5);
    content_layout->addWidget(label, 0, 0, Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addWidget(combobox, 0, 1, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);

    manip_settings->set_layout(content_layout);

    m_layout->addWidget(manip_settings);
}

void BezierToolSettingsWidget::createTangentSettingsRollout()
{
    auto tangent_settings = new RolloutWidget(i18n("bezier_tool.settings", "Tangent Settings"));
    auto settings = Application::instance().get_settings();
    bool expanded = settings->get("viewport.bezier_tool.ui.tangent_settings", true);
    tangent_settings->set_expanded(expanded);
    connect(tangent_settings, &RolloutWidget::clicked, [](bool expanded) {
        auto settings = Application::instance().get_settings();
        settings->set("viewport.bezier_tool.ui.tangent_settings", !expanded);
    });

    auto label = new QLabel(i18n("bezier_tool.settings.tangent", "Select Mode:"));
    auto combobox = new QComboBox;

    combobox->addItem(i18n("bezier_tool.settings.tangent", "Normal Select"), QVariant(static_cast<int>(BezierCurve::TangentMode::Normal)));
    combobox->addItem(i18n("bezier_tool.settings.tangent", "Weighted Select"), QVariant(static_cast<int>(BezierCurve::TangentMode::Weighted)));
    combobox->addItem(i18n("bezier_tool.settings.tangent", "Tangent Select"), QVariant(static_cast<int>(BezierCurve::TangentMode::Tangent)));

    const auto index = settings->get("viewport.bezier_tool.ui.select_mode", static_cast<int>(BezierCurve::TangentMode::Normal));
    combobox->setCurrentIndex(index);
    m_context->set_tangent_mode(static_cast<BezierCurve::TangentMode>(index));

    QObject::connect(combobox, qOverload<int>(&QComboBox::activated), this, [this, combobox](int index) {
        m_context->set_tangent_mode(static_cast<BezierCurve::TangentMode>(combobox->itemData(index).toInt()));

        auto settings = Application::instance().get_settings();
        settings->set("viewport.bezier_tool.ui.select_mode", index);
    });

    auto content_layout = new QGridLayout;
    content_layout->setColumnStretch(0, 2);
    content_layout->setColumnStretch(1, 5);
    content_layout->addWidget(label, 0, 0, Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addWidget(combobox, 0, 1, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);

    tangent_settings->set_layout(content_layout);

    m_layout->addWidget(tangent_settings);
}

OPENDCC_NAMESPACE_CLOSE
