// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/render_view/image_view/preferences_window.hpp"
#include "opendcc/render_view/image_view/translator.h"
#include "opendcc/render_view/image_view/app.h"

#include <QtCore/QDir>
#include <QtWidgets/QLabel>
#include <QtCore/QSettings>
#include <QtWidgets/QComboBox>
#include <QtCore/QMetaType>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QApplication>

#include <iostream>

Q_DECLARE_METATYPE(QAction*);

OPENDCC_NAMESPACE_OPEN

static const char* render_view_pref_stylesheet = R"#(
QTabWidget::pane {
    background: palette(light);
    border-top-color: palette(light);
}

QTabBar::tab:selected, QTabBar::tab:hover {
    background: palette(light);
    color: palette(foreground);
}

QTabBar::tab:!selected {
    background: rgb(55, 55, 55);
}

QToolBar[att~="main"] {
    background: palette(light);
    margin-top: 1px;
    margin-bottom: 1px;
}

QTabBar::tab {
    color: palette(dark);
    background: rgb(55, 55, 55);

    padding-left: 12px;
    padding-right: 12px;
    padding-top: 4px;
    padding-bottom: 5px;

    border-radius: 0px;
    border-left: 0px;

    border: 1px solid;
    border-width: 0px 1px 0px 0px;
    border-color: palette(base) palette(light) palette(light) palette(base);
}

QTabBar::tab:left {
    padding-left: 4px;
    padding-right: 5px;
    padding-top: 12px;
    padding-bottom: 12px;
    border-width: 0px 0px 1px 0px;
    border-color: palette(light) palette(base) palette(light) palette(light);
}

QTabBar::tab:last {
    border: 0px;
}

)#";

static const char* render_view_pref_stylesheet_light = R"#(
QTabWidget::pane {
    background: palette(light);
    border-top-color: palette(light);
}

QTabBar::tab:selected, QTabBar::tab:hover {
    background: palette(light);
    color: palette(foreground);
}

QTabBar::tab:!selected {
    color: palette(dark);
    background: palette(window);
}

QToolBar[att~="main"] {
    background: palette(light);
    margin-top: 1px;
    margin-bottom: 1px;
}

QTabBar::tab {
    color: palette(foreground);
    background: palette(light);

    padding-left: 12px;
    padding-right: 12px;
    padding-top: 4px;
    padding-bottom: 5px;

    border-radius: 0px;
    border-left: 0px;

    border: 1px solid;
    border-width: 0px 1px 0px 0px;
    border-color: palette(base) palette(light) palette(light) palette(base);
}

QTabBar::tab:left {
    padding-left: 4px;
    padding-right: 5px;
    padding-top: 12px;
    padding-bottom: 12px;
    border-width: 0px 0px 1px 0px;
    border-color: palette(light) palette(base) palette(light) palette(light);
}

QTabBar::tab:last {
    border: 0px;
}

)#";

//////////////////////////////////////////////////////////////////////////
// RenderViewPreferencesWindow
//////////////////////////////////////////////////////////////////////////

RenderViewPreferencesWindow::RenderViewPreferencesWindow(RenderViewMainWindow* parent, const RenderViewPreferencesWindowOptions& options)
{
    setStyleSheet(render_view_pref_stylesheet);

    m_parent_window = parent;
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    // Top layout. The only layout on whole WINDOW widget
    QVBoxLayout* main_layout = new QVBoxLayout();

    // All preferences falls into several distinct tabs. This is the widget handling this tabs
    QTabWidget* preferences_tabs_main_widget = new QTabWidget();

    // Two tabs: generals and shortcut editor
    QWidget* generals_tab_widget = new QWidget(preferences_tabs_main_widget);
    QWidget* shortcuts_tab_widget = new QWidget(preferences_tabs_main_widget);

    // Two widgets (above) are containers for their personal single layouts (below)
    QVBoxLayout* generals_tab_layout = new QVBoxLayout();
    QVBoxLayout* shortcut_editor_layout = new QVBoxLayout();

    // Layout for img location line
    QHBoxLayout* img_location_line_layout = new QHBoxLayout();

    // Layout for main buttons of whole window
    QHBoxLayout* apply_discard_layout = new QHBoxLayout();
    // Layout for controls in shortcut editor
    QHBoxLayout* keyeditor_control_line = new QHBoxLayout();

    // Layout for image cache size
    QHBoxLayout* image_cache_size_layout = new QHBoxLayout();

    auto& prefs = m_parent_window->get_prefs();

    // INIT WIDGETS START
    m_burn_in_mapping_on_save_chb = new QCheckBox(i18n("render_view.preferences.general", "Burn In Mapping On Save"), generals_tab_widget);
    m_color_space_cmb = new QComboBox(generals_tab_widget);
    for (const QString& it : options.color_space_values)
        m_color_space_cmb->addItem(it);
    m_color_space_cmb->setCurrentText(prefs.default_image_color_space);

    m_display_space_cmb = new QComboBox(generals_tab_widget);
    for (const QString& it : options.display_values)
        m_display_space_cmb->addItem(it);
    m_display_space_cmb->setCurrentText(prefs.default_display_view);

    QPushButton* m_apply_button = new QPushButton(i18n("render_view.preferences", "Apply"), this);
    QPushButton* m_cancel_button = new QPushButton(i18n("render_view.preferences", "Cancel"), this);

    m_scratch_image_location_ledit = new QLineEdit();
    QPushButton* m_scratch_image_location_button = new QPushButton("...");
    m_scratch_image_location_button->setMaximumWidth(22);

    m_key_editor = new KeySequenceEdit(shortcuts_tab_widget);
    m_key_editor->setObjectName("key_sequence_editor");
    m_key_editor->setContentsMargins(0, 0, 0, 0);
    m_assign_button = new QPushButton(i18n("render_view.preferences.hotkeys", "Assign"), shortcuts_tab_widget);
    m_assign_button->setContentsMargins(0, 0, 0, 0);
    QPushButton* m_to_defaults_button = new QPushButton(i18n("render_view.preferences.hotkeys", "Reset"), shortcuts_tab_widget);

    m_image_cache_size = new QSpinBox;
    m_image_cache_size->setRange(0, std::numeric_limits<int>::max());
    m_image_cache_size->setSingleStep(1);
    // INIT WIDGETS END

    // FILL GENERALS TAB BEGIN
    img_location_line_layout->addWidget(new QLabel(i18n("render_view.preferences.general", "Scratch Image Location:")));
    img_location_line_layout->addWidget(m_scratch_image_location_ledit);
    img_location_line_layout->addWidget(m_scratch_image_location_button);

    image_cache_size_layout->addWidget(new QLabel(i18n("render_view.preferences.general", "Image Cache Size:")));
    image_cache_size_layout->addWidget(m_image_cache_size);

    auto warning_layout = new QHBoxLayout;
    auto warning_label = new QLabel;
    warning_label->setPixmap(QPixmap(":/icons/warning"));
    warning_layout->addWidget(warning_label);
    warning_layout->addWidget(new QLabel(i18n("preferences.common", "You must restart the program for the changes to take effect.")));
    warning_layout->addStretch();
    auto warning = new QFrame;
    warning->setObjectName("language_change_warning");
    warning->setStyleSheet(R"(
        QFrame #language_change_warning {
            background-color: rgba(255, 50, 50, 50);
            border-radius: 2px;
            border: 1px solid;
            border-color: rgba(255, 0, 0, 50);
        }
    )");
    warning->setLayout(warning_layout);
    warning->hide();

    auto language_layout = new QHBoxLayout();
    language_layout->addWidget(new QLabel(i18n("render_view.preferences.general", "Language:")));
    m_language_cmb = new QComboBox;
    language_layout->addWidget(m_language_cmb);

    const auto& translator = Translator::instance();
    const auto supported_languages = translator.get_supported_beauty_languages();
    for (const auto& languages : supported_languages)
    {
        m_language_cmb->addItem(languages);
    }
    m_language_cmb->setCurrentText(translator.to_beauty(prefs.language));
    QObject::connect(m_language_cmb, &QComboBox::currentTextChanged, this, [this, warning](const QString& text) {
        auto& prefs = m_parent_window->get_prefs();
        auto& translator = Translator::instance();
        const auto language = translator.from_beauty(text);
        if (translator.set_language(language))
        {
            prefs.language = language;
            warning->show();
        }
    });

    generals_tab_layout->setAlignment(Qt::AlignTop);

    generals_tab_layout->addLayout(language_layout);
    generals_tab_layout->addWidget(warning);
    generals_tab_layout->addLayout(img_location_line_layout);
    generals_tab_layout->addLayout(image_cache_size_layout);
    generals_tab_layout->addWidget(m_burn_in_mapping_on_save_chb);
    auto glayout = new QGridLayout();
    glayout->addWidget(new QLabel(i18n("render_view.preferences.general", "Default Color Space:")), 0, 0);
    glayout->addWidget(m_color_space_cmb, 0, 1);
    glayout->addWidget(new QLabel(i18n("render_view.preferences.general", "Default Display View:")), 1, 0);
    glayout->addWidget(m_display_space_cmb, 1, 1);
    generals_tab_layout->addLayout(glayout);

    generals_tab_widget->setLayout(generals_tab_layout);
    preferences_tabs_main_widget->addTab(generals_tab_widget, i18n("render_view.preferences.general", "General"));
    // FILL GENERALS TAB END

    // TABLE SETUP BEGIN
    m_shortcuts_table_widget = new QTableWidget(0, 2);
    m_shortcuts_table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_shortcuts_table_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_shortcuts_table_widget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_shortcuts_table_widget->verticalHeader()->setVisible(false);
    m_shortcuts_table_widget->setMinimumHeight(250);
    m_shortcuts_table_widget->setHorizontalHeaderItem(0, new QTableWidgetItem(i18n("render_view.preferences.hotkeys", "Action")));
    m_shortcuts_table_widget->setHorizontalHeaderItem(1, new QTableWidgetItem(i18n("render_view.preferences.hotkeys", "Shortcut")));
    QHeaderView* header = m_shortcuts_table_widget->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    fill_shortcuts_table();
    header->sortIndicatorOrder();
    m_shortcuts_table_widget->sortItems(0, Qt::AscendingOrder);
    m_shortcuts_table_widget->setColumnWidth(1, 150);
    // TABLE SETUP END

    // FILL KEY EDITOR CONTROL LINE BEGIN
    keyeditor_control_line->setContentsMargins(0, 10, 0, 0);
    keyeditor_control_line->addWidget(m_key_editor);
    keyeditor_control_line->addSpacing(10);
    keyeditor_control_line->addWidget(m_assign_button);
    keyeditor_control_line->addSpacing(10);
    keyeditor_control_line->addWidget(m_to_defaults_button);
    // FILL KEY EDITOR CONTROL LINE END

    // FILL SHORTCUT EDITOR TAB BEGIN
    shortcut_editor_layout->addWidget(m_shortcuts_table_widget);
    shortcut_editor_layout->addLayout(keyeditor_control_line);
    shortcuts_tab_widget->setLayout(shortcut_editor_layout);
    preferences_tabs_main_widget->addTab(shortcuts_tab_widget, i18n("render_view.preferences.hotkeys", "Hotkeys"));
    // FILL SHORTCUT EDITOR TAB END

    main_layout->addWidget(preferences_tabs_main_widget);
    main_layout->addLayout(apply_discard_layout);

    // ADD APPLY/DISCARD BUTTONS ON THE MAIN LAYOUT BEGIN
    apply_discard_layout->addWidget(m_apply_button);
    apply_discard_layout->addWidget(m_cancel_button);
    // ADD APPLY/DISCARD BUTTONS ON THE MAIN LAYOUT END

    setLayout(main_layout);

    connect(m_assign_button, SIGNAL(clicked()), this, SLOT(assign_new_shortcut()));
    connect(m_shortcuts_table_widget->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this,
            SLOT(show_sequence_in_key_editor()));
    connect(m_apply_button, SIGNAL(clicked()), this, SLOT(apply_prefs()));
    connect(m_apply_button, SIGNAL(clicked()), this, SLOT(hide()));
    connect(m_cancel_button, SIGNAL(clicked()), this, SLOT(hide()));
    connect(m_to_defaults_button, SIGNAL(clicked()), this, SLOT(shortcuts_to_default()));
    connect(m_key_editor, SIGNAL(keySequenceChanged(const QKeySequence&)), this, SLOT(detect_collision()));

    connect(m_scratch_image_location_button, SIGNAL(clicked()), this, SLOT(set_scratch_image_location()));
    resize(600, 400);
    setWindowTitle(i18n("render_view.preferences", "Preferences"));
}

RenderViewPreferencesWindow::~RenderViewPreferencesWindow() {}

void RenderViewPreferencesWindow::apply_prefs()
{
    if (!QDir(m_scratch_image_location_ledit->text()).exists())
    {
        QMessageBox::warning(nullptr, i18n("render_view.preferences.apply.error", "Error"),
                             i18n("render_view.preferences.apply.error", "Directory ") + m_scratch_image_location_ledit->text() +
                                 i18n("render_view.preferences.apply.error", " doesn't exist"));
        return;
    }

    auto& prefs = m_parent_window->get_prefs();

    prefs.scratch_image_location = m_scratch_image_location_ledit->text();
    prefs.burn_in_mapping_on_save = m_burn_in_mapping_on_save_chb->isChecked();
    prefs.default_image_color_space = m_color_space_cmb->itemText(m_color_space_cmb->currentIndex());
    prefs.default_display_view = m_display_space_cmb->itemText(m_display_space_cmb->currentIndex());
    prefs.image_cache_size = m_image_cache_size->value();

    auto& translator = Translator::instance();
    const auto language = translator.from_beauty(m_language_cmb->currentText());
    prefs.language = language;

    apply_shortcuts();
    emit preferences_updated();
}

void RenderViewPreferencesWindow::update_pref_windows()
{
    auto& prefs = m_parent_window->get_prefs();
    m_scratch_image_location_ledit->setText(prefs.scratch_image_location);
    m_shortcuts_table_widget->setRowCount(0);
    fill_shortcuts_table();
    m_key_editor->set_key_sequence(QKeySequence());
    m_burn_in_mapping_on_save_chb->setChecked(prefs.burn_in_mapping_on_save);
    m_image_cache_size->setValue(prefs.image_cache_size);
}

void RenderViewPreferencesWindow::set_scratch_image_location()
{
    auto& prefs = m_parent_window->get_prefs();
    QString scratch_image_location = QFileDialog::getExistingDirectory(
        this, i18n("render_view.preferences.general.scratch_image_location.file_dialog", "Scratch Image Location"), prefs.scratch_image_location);
    if (scratch_image_location != "")
    {
        m_scratch_image_location_ledit->setText(scratch_image_location);
    }
}

// Shortcut editor specific functions BEGIN
void RenderViewPreferencesWindow::fill_shortcuts_table()
{
    m_shortcuts_table_widget->setSortingEnabled(false);
    int row_cur_number = 0;
    for (auto map_item : m_parent_window->get_defaults_map())
    {
        m_shortcuts_table_widget->insertRow(m_shortcuts_table_widget->rowCount());

        QTableWidgetItem* action_name = new QTableWidgetItem(map_item.first->text());
        QVariant var;
        var.setValue((QAction*)map_item.first);
        action_name->setData(Qt::UserRole, var);

        QTableWidgetItem* action_shortcut = new QTableWidgetItem(map_item.first->shortcut().toString());
        m_shortcuts_table_widget->setItem(row_cur_number, 0, action_name);
        m_shortcuts_table_widget->setItem(row_cur_number, 1, action_shortcut);
        ++row_cur_number;
    }
    m_shortcuts_table_widget->setSortingEnabled(true);
}

void RenderViewPreferencesWindow::show_sequence_in_key_editor()
{
    if (!m_shortcuts_table_widget->selectedItems().isEmpty())
    {
        m_key_editor->set_key_sequence(m_shortcuts_table_widget->selectedItems()[1]->text());
    }
}

void RenderViewPreferencesWindow::assign_new_shortcut()
{
    if (!m_shortcuts_table_widget->selectedItems().isEmpty())
    {
        m_shortcuts_table_widget->selectedItems()[1]->setText(m_key_editor->key_sequence().toString());
    }
}

void RenderViewPreferencesWindow::apply_shortcuts()
{
    for (int i = 0; i < m_shortcuts_table_widget->rowCount(); i++)
    {
        QTableWidgetItem* item = m_shortcuts_table_widget->item(i, 0);
        QAction* found_action = item->data(Qt::UserRole).value<QAction*>();
        found_action->setShortcut(m_shortcuts_table_widget->item(i, 1)->text());
    }
}

void RenderViewPreferencesWindow::shortcuts_to_default()
{
    for (int i = 0; i < m_shortcuts_table_widget->rowCount(); i++)
    {
        QAction* found_action = m_shortcuts_table_widget->item(i, 0)->data(Qt::UserRole).value<QAction*>();
        QString default_sequence = m_parent_window->get_defaults_map()[found_action].toString();
        m_shortcuts_table_widget->item(i, 1)->setText(default_sequence);
    }
}

void RenderViewPreferencesWindow::detect_collision()
{
    m_key_editor->set_is_error(false);
    m_assign_button->setEnabled(true);

    for (int i = 0; i < m_shortcuts_table_widget->rowCount(); i++)
    {
        QKeySequence current_comparable(m_shortcuts_table_widget->item(i, 1)->text());
        if (current_comparable == m_key_editor->key_sequence())
        {
            m_key_editor->set_is_error(true);
            m_assign_button->setEnabled(false);
            break;
        }
    }
}

// Shortcut editor specific functions END

//////////////////////////////////////////////////////////////////////////
// RenderViewPreferences
//////////////////////////////////////////////////////////////////////////

RenderViewPreferences RenderViewPreferences::read(std::unique_ptr<QSettings> settings)
{
    RenderViewPreferences preferences;
    preferences.settings = std::move(settings);
    preferences.scratch_image_location = preferences.settings->value("main/scratch_image_location", QDir::tempPath()).toString();
    preferences.burn_in_mapping_on_save = preferences.settings->value("main/burn_in_mapping_on_save", false).toBool();
    preferences.default_image_color_space = preferences.settings->value("main/default_image_color_space", "").toString();
    preferences.default_display_view = preferences.settings->value("main/default_display_view", "").toString();
    preferences.background_mode = preferences.settings->value("main/background_mode", 0).toInt();
    preferences.show_resolution_guides = preferences.settings->value("main/show_resolution_guides", false).toBool();
    preferences.image_cache_size = preferences.settings->value("main/image_cache_size", 100).toInt();
    preferences.language = preferences.settings->value("main/language", "English").toString();
    return preferences;
}

void RenderViewPreferences::save() const
{
    settings->setValue("main/burn_in_mapping_on_save", burn_in_mapping_on_save);
    settings->setValue("main/scratch_image_location", scratch_image_location);
    settings->setValue("main/default_image_color_space", default_image_color_space);
    settings->setValue("main/default_display_view", default_display_view);
    settings->setValue("main/background_mode", background_mode);
    settings->setValue("main/show_resolution_guides", show_resolution_guides);
    settings->setValue("main/image_cache_size", image_cache_size);
    const auto test = language.toStdString();
    settings->setValue("main/language", language);
}

OPENDCC_NAMESPACE_CLOSE
