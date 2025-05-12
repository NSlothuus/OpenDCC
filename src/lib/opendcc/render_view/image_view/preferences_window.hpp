// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/render_view/image_view/api.h"
#include "opendcc/render_view/image_view/key_sequence_edit.h"

#include <QString>
#include <QLabel>
#include <QWidget>
#include <QAction>
#include <QPalette>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <memory>

class QSettings;
class QComboBox;
class QTranslator;
class QTableWidget;
class QItemSelection;

OPENDCC_NAMESPACE_OPEN

class RenderViewMainWindow;

struct OPENDCC_RENDER_VIEW_API RenderViewPreferences
{
    QString scratch_image_location;
    QString image_color_space;
    QString default_image_color_space;
    QString display;
    QString default_display_view;
    int background_mode;
    bool burn_in_mapping_on_save;
    bool show_resolution_guides;
    int image_cache_size = 100;
    QString language;
    std::unique_ptr<QSettings> settings;

    static RenderViewPreferences read(std::unique_ptr<QSettings> settings);
    void save() const;
};

struct RenderViewPreferencesWindowOptions
{
    std::vector<QString> color_space_values;
    std::vector<QString> display_values;
};

class RenderViewPreferencesWindow : public QWidget
{
    Q_OBJECT

public:
    RenderViewPreferencesWindow(RenderViewMainWindow* parent, const RenderViewPreferencesWindowOptions& options);
    ~RenderViewPreferencesWindow();

    void update_pref_windows();

Q_SIGNALS:
    void preferences_updated();

private Q_SLOTS:
    void apply_prefs();
    void set_scratch_image_location();
    void show_sequence_in_key_editor();
    void assign_new_shortcut();
    void shortcuts_to_default();
    void detect_collision();

private:
    void fill_shortcuts_table();
    void apply_shortcuts();

    RenderViewMainWindow* m_parent_window;
    QPushButton* m_assign_button;
    QLineEdit* m_scratch_image_location_ledit;
    QCheckBox* m_burn_in_mapping_on_save_chb;
    QComboBox* m_color_space_cmb;
    QComboBox* m_display_space_cmb;
    QComboBox* m_language_cmb;
    QTableWidget* m_shortcuts_table_widget;
    KeySequenceEdit* m_key_editor;
    QSpinBox* m_image_cache_size;
};

OPENDCC_NAMESPACE_CLOSE
