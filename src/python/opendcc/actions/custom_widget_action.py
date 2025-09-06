# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from PySide6 import QtCore, QtGui, QtWidgets
import opendcc.core as dcc_core
from opendcc.color_theme import get_color_theme, ColorTheme


class OptionButton(QtWidgets.QWidget):
    def paintEvent(self, event):
        painter = QtGui.QPainter(self)
        painter.drawRect(QtCore.QRect(self.rect().x(), self.rect().y(), 10, 10))


class CustomActionWidget(QtWidgets.QWidget):
    def __init__(self, title, parent=None):
        QtWidgets.QWidget.__init__(self, parent)

        layout = QtWidgets.QHBoxLayout()
        self.setLayout(layout)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        self.left_widget = QtWidgets.QFrame()
        left_layout = QtWidgets.QHBoxLayout()
        self.left_widget.setLayout(left_layout)
        left_layout.setContentsMargins(7, 3, 5, 4)
        left_layout.setSpacing(0)

        self.right_widget = QtWidgets.QFrame()
        right_layout = QtWidgets.QHBoxLayout()
        self.right_widget.setLayout(right_layout)
        right_layout.setContentsMargins(4, 4, 4, 4)
        right_layout.setSpacing(0)

        self.icon = QtWidgets.QLabel()
        self.icon.setMinimumSize(16, 16)
        left_layout.addWidget(self.icon)

        left_layout.addSpacerItem(QtWidgets.QSpacerItem(6, 6))

        label_title = QtWidgets.QLabel(title)
        left_layout.addWidget(label_title)

        left_layout.addSpacerItem(QtWidgets.QSpacerItem(0, 0, QtWidgets.QSizePolicy.Expanding))

        self.label_shortcut = QtWidgets.QLabel()
        left_layout.addWidget(self.label_shortcut)
        self.label_shortcut.setStyleSheet("QLabel {padding-right: 6px; }")

        self.options_checkbox = OptionButton()
        self.options_checkbox.setMinimumSize(11, 11)
        self.right_widget.setVisible(False)

        right_layout.addWidget(self.options_checkbox)

        layout.addWidget(self.left_widget)
        layout.addWidget(self.right_widget)

        hl_background = "#%02x%02x%02x" % self.palette().highlight().color().getRgb()[:3]
        hl_text = "#%02x%02x%02x" % self.palette().highlightedText().color().getRgb()[:3]

        # background = (
        #     "#%02x%02x%02x" % self.palette().base().color().getRgb()[:3]
        # )
        if get_color_theme() == ColorTheme.LIGHT:
            background = "#ffffff"
        else:
            background = "#%02x%02x%02x" % (  # menu's color isn't palette's base color
                52,
                52,
                52,
            )
        text = "#%02x%02x%02x" % self.palette().text().color().getRgb()[:3]

        self.highlight_on = "QWidget { background:%s; color: %s; }" % (
            hl_background,
            hl_text,
        )
        self.highlight_off = "QWidget { background:%s; color: %s; }" % (
            background,
            text,
        )

        # self.setAttribute(QtCore.Qt.WA_Hover, True)
        self.setMouseTracking(True)
        for widget in self.findChildren(QtWidgets.QWidget):
            widget.setMouseTracking(True)
        self.mouse_pos = None

    def check_mouse(self):
        if not self.right_widget.isHidden():
            if self.mouse_pos:
                if self.mouse_pos.x() >= (self.width() - self.right_widget.width()):
                    return False
        return True

    # def event(self, event): # probably overkill
    #     if event.type() == QtCore.QEvent.HoverMove:
    #         self.mouse_pos = event.pos()
    #         # self.set_highlight(True)
    #     elif event.type() == QtCore.QEvent.HoverEnter:
    #         self.set_highlight(True)
    #     elif event.type() == QtCore.QEvent.HoverLeave:
    #         self.set_highlight(False)
    #     return super(CustomActionWidget, self).event(event)

    def mousePressEvent(self, event):
        self.mouse_pos = event.pos()
        super(CustomActionWidget, self).mousePressEvent(event)

    def mouseMoveEvent(self, event):
        self.mouse_pos = event.pos()
        self.set_highlight(True)
        super(CustomActionWidget, self).mouseMoveEvent(event)

    def set_highlight(self, value):
        if value:
            self.setStyleSheet(self.highlight_on)
            if self.check_mouse():
                self.right_widget.setStyleSheet(self.highlight_off)
            else:
                self.right_widget.setStyleSheet(self.highlight_on)
        else:
            self.setStyleSheet(self.highlight_off)
            self.right_widget.setStyleSheet(self.highlight_off)

    ## after "fix_menu" there's no need for these, but maybe
    ## "fix_menu" was a terrible idea
    # def enterEvent(self, event):
    #     self.set_highlight(True)
    #     super(CustomActionWidget, self).enterEvent(event)

    def leaveEvent(self, event):
        self.set_highlight(False)
        super(CustomActionWidget, self).leaveEvent(event)

    def hideEvent(self, event):  # because leave event doesn't happen after hiding
        self.set_highlight(False)
        super(CustomActionWidget, self).hideEvent(event)


class CustomAction(QtWidgets.QWidgetAction):
    custom_triggered = QtCore.Signal()

    def __init__(self, title, parent=None):
        QtWidgets.QWidgetAction.__init__(self, parent)
        self.widget = CustomActionWidget(title)
        self.callback = None
        self.triggered.connect(self.handle_triggered)
        self.setDefaultWidget(self.widget)

    def handle_triggered(self, value):
        if self.widget.check_mouse():
            self.custom_triggered.emit()
        else:
            if self.callback:
                self.callback()

    def setIcon(self, icon):
        self.widget.icon.setPixmap(icon.pixmap(16, 16))

    def setShortcut(self, keys, callback):
        key_sequence = QtGui.QKeySequence(keys)
        app = dcc_core.Application.instance()
        main_window = app.get_main_window()
        shortcut = QtWidgets.QShortcut(
            main_window
        )  # because it can't be parented to self in this case
        shortcut.setKey(key_sequence)
        shortcut.setContext(QtCore.Qt.ApplicationShortcut)  # hello
        shortcut.activated.connect(callback)
        # QtWidgets.QWidgetAction.setShortcut(self, key_sequence) # "triggered" signal is being weirdly handled
        self.widget.label_shortcut.setText(key_sequence.toString())

    def on_checkbox_clicked(self, callback):
        self.widget.label_shortcut.setStyleSheet("QLabel {padding-right: 0px; }")
        self.widget.right_widget.setVisible(True)
        self.callback = callback

    def set_highlight(self, value):
        self.widget.setFocus()
        self.widget.set_highlight(value)


# https://stackoverflow.com/questions/55086498/highlighting-custom-qwidgetaction-on-hover
def fix_menu(menu):
    def highlight_action(action):
        for custom_action in menu.findChildren(CustomAction):
            custom_action.set_highlight(custom_action == action)

    menu.hovered.connect(highlight_action)
