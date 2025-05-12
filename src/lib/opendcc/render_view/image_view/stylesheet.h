/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"

OPENDCC_NAMESPACE_OPEN
static const char* render_view_stylesheet = R"#(
QMainWindow > QToolBar
{
    border: 0px;
}

QMainWindow > QToolBar::handle
{
    image: url(":icons/render_view/handle-horizontal");
}

QMainWindow > QToolBar::handle:vertical
{
    image: url(":icons/render_view/handle-vertical");
}

QDockWidget {
    border: none;
    titlebar-close-icon: url(:icons/render_view/dock_close);
    titlebar-normal-icon: url(:icons/render_view/dock_normal);
}

QDockWidget::float-button, QDockWidget::close-button {
    border: none;
    padding: 0;
    margin: 0;
    icon-size: 12px;
}

QDockWidget::close-button:pressed, QDockWidget::float-button:pressed {
    padding: 1px -1px -1px 1px;
}

QDockWidget::title {
    text-align: center;
    padding-top: 2px;
    padding-bottom: 3px;
}

QComboBox, QLineEdit, QDoubleSpinBox, QIntSpinBox
{
    border: 1px solid #333333;
    border-radius: 2px;
    background: palette(base);
    padding-left: 4px;
    padding-right: 1px;
    padding-top: 1px;
    padding-bottom: 1px;
    selection-background-color: palette(highlight);
}

QComboBox
{
    background: #373737;
}

QComboBox:focus, QLineEdit:focus, QDoubleSpinBox:focus, QIntSpinBox:focus
{
    border: 1px solid palette(highlight);
}

QComboBox:hover:!disabled:!focus
{
    border: 1px solid #5b5b5b;
}

QComboBox:disabled
{
    color: #555555;
}

QComboBox::drop-down
{
    subcontrol-origin: padding;
    subcontrol-position: top right;
    width: 20px;

    border-top-right-radius: 3px;
    border-bottom-right-radius: 3px;
}

QComboBox::down-arrow
{
    image: url(:icons/render_view/tabsMenu);
}

QComboBox::down-arrow:disabled
{
    image: url(:icons/render_view/tabsMenu_disabled);
}

QComboBox::down-arrow:hover:!disabled
{
    image: url(:icons/render_view/tabsMenu_hover);
}

QComboBox QAbstractItemView
{
    background-color: #444444;
    color: palette(window-text);
}

QComboBox QLineEdit
{
    border: 0px;
    background: #373737;
}

QComboBox QLineEdit:focus
{
    border: 0px;
}

QComboBox QLineEdit:hover
{
    border: 0px;
}

)#";

static const char* render_view_stylesheet_light = R"#(
QMainWindow > QToolBar
{
    border: 0px;
}

QMainWindow > QToolBar::handle
{
    image: url(":icons/render_view/handle-horizontal");
}

QMainWindow > QToolBar::handle:vertical
{
    image: url(":icons/render_view/handle-vertical");
}

QDockWidget {
    border: none;
    titlebar-close-icon: url(:icons/render_view/dock_close);
    titlebar-normal-icon: url(:icons/render_view/dock_normal);
}

QDockWidget::float-button, QDockWidget::close-button {
    border: none;
    padding: 0;
    margin: 0;
    icon-size: 12px;
}

QDockWidget::close-button:pressed, QDockWidget::float-button:pressed {
    padding: 1px -1px -1px 1px;
}

QDockWidget::title {
    text-align: center;
    padding-top: 2px;
    padding-bottom: 3px;
}

QComboBox, QLineEdit, QDoubleSpinBox, QIntSpinBox
{
    border: 1px solid #cdcdcd;
    border-radius: 2px;
    background: palette(base);
    padding-left: 4px;
    padding-right: 1px;
    padding-top: 1px;
    padding-bottom: 1px;
    selection-background-color: palette(highlight);
}

QComboBox
{
    background: #e8e8e8;
}

QComboBox:focus, QLineEdit:focus, QDoubleSpinBox:focus, QIntSpinBox:focus
{
    border: 1px solid palette(highlight);
}

QComboBox:hover:!disabled:!focus
{
    border: 1px solid #5b5b5b;
}

QComboBox:disabled
{
    color: #555555;
}

QComboBox::drop-down
{
    subcontrol-origin: padding;
    subcontrol-position: top right;
    width: 20px;

    border-top-right-radius: 3px;
    border-bottom-right-radius: 3px;
}

QComboBox::down-arrow
{
    image: url(:icons/render_view/tabsMenu);
}

QComboBox::down-arrow:disabled
{
    image: url(:icons/render_view/tabsMenu_disabled);
}

QComboBox::down-arrow:hover:!disabled
{
    image: url(:icons/render_view/tabsMenu_hover);
}

QComboBox QAbstractItemView
{
    background-color: #444444;
    color: palette(window-text);
}

QComboBox QLineEdit
{
    border: 0px;
    background: #373737;
}

QComboBox QLineEdit:focus
{
    border: 0px;
}

QComboBox QLineEdit:hover
{
    border: 0px;
}

)#";

OPENDCC_NAMESPACE_CLOSE
