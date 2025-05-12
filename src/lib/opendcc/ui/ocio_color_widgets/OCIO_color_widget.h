/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/ocio_color_widgets/api.h"
#include "opendcc/ui/common_widgets/color_widget.h"

#include <OpenColorIO/OpenColorIO.h>

#include <QWidget>
#include <QCheckBox>
#include <QColor>

OPENDCC_NAMESPACE_OPEN

/**
 * @brief A class that manages color operations using the OpenColorIO library.
 * This class provides functionality for managing color operations, such as
 * getting the current OpenColorIO configuration, obtaining color processors,
 * and performing color transformations.
 *
 */
class OCIOColorManager
{
    OCIO_NAMESPACE::ConstConfigRcPtr m_config; // Gets current configuration.
    OCIO_NAMESPACE::ConstProcessorRcPtr m_processor; // ROLE_SCENE_LINEAR -> ROLE_COLOR_PICKING
    OCIO_NAMESPACE::ConstProcessorRcPtr m_reverse_processor; // ROLE_COLOR_PICKING -> ROLE_SCENE_LINEAR
public:
    OCIOColorManager();
    /**
     *   @brief Converts the given color using the OpenColorIO library.
     *   This function converts the input color from ROLE_SCENE_LINEAR to ROLE_COLOR_PICKING,
     *   or from ROLE_COLOR_PICKING to ROLE_SCENE_LINEAR if the 'reverse' parameter is set to true.
     *   @param color The QColor object representing the color to be converted.
     *   @param reverse If true, the color is converted from ROLE_COLOR_PICKING to ROLE_SCENE_LINEAR.
     *   @return The converted QColor.
     */
    QColor convert(const QColor& color, bool reverse = false);
};

class OPENDCC_UI_OCIO_COLOR_WIDGETS_API OCIOColorWidget : public ColorWidget
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a ColorWidget.
     *
     * @param parent The QWidget parent.
     * @param enable_alpha If true, enables alpha channel support for the color widget.
     */
    OCIOColorWidget(QWidget* parent = nullptr, bool enable_alpha = false);
    ~OCIOColorWidget();

    /**
     * @brief Checks if the color widget is enabled.
     *
     * @return True if the color widget is enabled, false otherwise.
     */
    bool is_enabled();
    /**
     * @brief Converts the given QColor.
     *
     * This function converts the input color from ROLE_SCENE_LINEAR to ROLE_COLOR_PICKING,
     * or optionally, from ROLE_COLOR_PICKING to ROLE_SCENE_LINEAR if the 'reverse' parameter is set to true.
     * It also normalizes the color values if the 'normalize' parameter is set to true.
     *
     * @param color The color to be converted.
     * @param reverse If true, the color is converted from ROLE_COLOR_PICKING to ROLE_SCENE_LINEAR.
     * @param normalize If true, normalizes the color values.
     */
    QColor convert(const QColor& color, bool reverse = false, bool normalize = true);

Q_SIGNALS:
    /**
     * @brief Signal emitted when the color widget is enabled.
     *
     * @param enabled True if the color widget is enabled, false otherwise.
     */
    void enabled(bool);

public Q_SLOTS:
    /**
     * @brief Slot for handling color change events.
     *
     * This slot is triggered when the color of the widget is changed.
     *
     * @param new_color The color it is changed to.
     */
    void color_changed(const QColor& new_color);

private:
    void setup_hue();
    void setup_sat();
    void setup_val();

    void setup_R();
    void setup_G();
    void setup_B();
    void setup_alpha();

    void init_box();
    void init_box_sat_val();
    void init_box_alpha();

    void setup_preview();
    void setup_palette();

private:
    OCIOColorManager* m_color_manager;
    QCheckBox* m_check_box;
    QColor m_converted;
};

class OPENDCC_UI_OCIO_COLOR_WIDGETS_API OCIOColorPickDialog : public ColorPickDialog
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a ColorPickDialog.
     *
     * @param parent The parent QWidget.
     * @param enable_alpha If true, enables alpha channel support in the color pick dialog.
     */
    OCIOColorPickDialog(QWidget* parent = nullptr, bool enable_alpha = false);
};

class OPENDCC_UI_OCIO_COLOR_WIDGETS_API OCIOColorButton : public ColorButton
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a ColorButton.
     *
     * @param parent The parent QWidget.
     * @param enable_alpha If true, enables alpha channel support for the color button.
     */
    OCIOColorButton(QWidget* parent, bool enable_alpha = false);
};

OPENDCC_NAMESPACE_CLOSE
