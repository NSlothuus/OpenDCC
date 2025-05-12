# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtCore, QtGui, QtWidgets
from collections import OrderedDict
from opendcc.i18n import i18n

COLOR_NAME_RED = i18n("palette_widget", "Red")
COLOR_NAME_BROWN = i18n("palette_widget", "Brown")
COLOR_NAME_GREEN = i18n("palette_widget", "Green")
COLOR_NAME_BLUE = i18n("palette_widget", "Blue")
COLOR_NAME_PURPLE = i18n("palette_widget", "Purple")
COLOR_NAME_BLACK = i18n("palette_widget", "Black")
COLOR_NAME_GRAY = i18n("palette_widget", "Gray")

CLASSIC_COLORS = OrderedDict(
    [
        (COLOR_NAME_RED, [120, 0, 0]),
        (COLOR_NAME_BROWN, [70, 50, 20]),
        (COLOR_NAME_GREEN, [20, 60, 10]),
        (COLOR_NAME_BLUE, [20, 40, 95]),
        (COLOR_NAME_PURPLE, [50, 5, 50]),
        (COLOR_NAME_BLACK, [0, 0, 0]),
    ]
)

PASTEL_COLORS = OrderedDict(
    [
        (COLOR_NAME_RED, [210, 150, 160]),
        (COLOR_NAME_BROWN, [140, 120, 0]),
        (COLOR_NAME_GREEN, [90, 120, 80]),
        (COLOR_NAME_BLUE, [50, 110, 170]),
        (COLOR_NAME_PURPLE, [145, 70, 145]),
        (COLOR_NAME_BLACK, [145, 145, 145]),
    ]
)

PASTEL_COLUMN_NUM = 4
RIGHT_COLORS_PARTITION = 5  # empirically obtained number for nice colours in the 2 right columns
COLUMNS_NUM = 7


class PaletteWidget(QtWidgets.QWidget):
    color_selected = QtCore.Signal(QtGui.QColor)

    def __init__(self, parent):
        QtWidgets.QWidget.__init__(self, parent)
        self.main_layout = QtWidgets.QGridLayout()
        self.setLayout(self.main_layout)
        self.setFixedSize(207, 180)
        self.main_layout.setSpacing(2)
        self.fill_palette()

        if parent:
            self.fit(parent)

    def fill_palette(self):
        color_names = tuple(CLASSIC_COLORS.keys())

        for color_name in color_names:
            row_index = color_names.index(color_name)
            if color_name == COLOR_NAME_BLACK:
                next_color = [
                    300,
                    300,
                    300,
                ]  # as black is the last colour, here are just values for good-looking gradient
            elif color_name == COLOR_NAME_PURPLE:
                next_color = [
                    50,
                    5,
                    50,
                ]  # as next colour for purple is black (0, 0, 0), for correct work of the algorithm below here is special colour
            else:
                next_color = CLASSIC_COLORS[color_names[row_index + 1]]

            for i in range(COLUMNS_NUM):
                color = QtWidgets.QWidget()
                color.setFixedSize(25, 25)
                if i <= PASTEL_COLUMN_NUM:
                    smart_color = [
                        int(x + i * (y - x) / PASTEL_COLUMN_NUM)
                        for x, y in zip(CLASSIC_COLORS[color_name], PASTEL_COLORS[color_name])
                    ]
                else:
                    smart_color = [
                        int(abs(x - i * (x - y)) / RIGHT_COLORS_PARTITION)
                        for x, y in zip(PASTEL_COLORS[color_name], next_color[::-1])
                    ]  # reverse next_colour list to get a color change that corresponds to the row base colour

                current_color = CLASSIC_COLORS[color_name] if i == 0 else smart_color
                color.setAutoFillBackground(True)
                widget_palette = color.palette()
                widget_palette.setColor(QtGui.QPalette.Window, QtGui.QColor(*current_color))
                color.setPalette(widget_palette)
                self.main_layout.addWidget(color, row_index, i)

    def mousePressEvent(self, event):
        widget = self.childAt(event.pos())
        if widget:
            self.color_selected.emit(widget.palette().color(QtGui.QPalette.Window))

    def fit(self, widget):
        self.move(widget.width() - self.width(), widget.height() - self.height())
