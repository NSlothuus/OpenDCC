# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets, QtGui, QtCore
import opendcc.core as dcc_core
import copy

from opendcc.i18n import i18n


class FloatWidget(QtWidgets.QDoubleSpinBox):
    def __init__(self, parent=None):
        super(FloatWidget, self).__init__(parent=parent)
        self.setButtonSymbols(QtWidgets.QAbstractSpinBox.NoButtons)
        self.setFixedWidth(70)
        self.setMaximum(1e10)
        self.setMinimum(-1e10)
        self.setLocale(QtCore.QLocale(QtCore.QLocale.Hawaiian, QtCore.QLocale.UnitedStates))


class Option(object):
    def __init__(self, name, setting, meta, default):
        self.name = name
        self.setting = setting
        self.meta = meta
        self.default = default
        self.widgets = []
        self.widget_layout = None

    def set_enable(self):
        for widget in self.widgets:
            widget.setEnabled(self.enable_widget.isChecked())

    def create_widget(self, layout, parent):
        if "enable" in self.meta:
            self.widget_layout.addStretch()
            self.enable_widget = QtWidgets.QCheckBox()
            self.enable_widget.setToolTip("Enable")
            self.enable_widget.setTristate(False)
            self.enable_widget.stateChanged.connect(self.set_enable)
            self.widget_layout.addWidget(self.enable_widget)
            self.set_enable()

    def load_options(self):
        if "enable" in self.meta:
            settings = dcc_core.Application.instance().get_settings()
            value = settings.get_bool(self.setting + "_enable", self.meta["enable"])
            self.enable_widget.setChecked(value)

    def save_options(self):
        if "enable" in self.meta:
            settings = dcc_core.Application.instance().get_settings()
            value = self.enable_widget.isChecked()
            settings.set_bool(self.setting + "_enable", value)


class SpacerOption(Option):
    def create_widget(self, layout, parent):
        frame = QtWidgets.QFrame()
        frame.setFixedHeight(4)
        layout.addRow(frame)


class FloatOption(Option):
    def create_widget(self, layout, parent):
        self.widgets.append(FloatWidget(parent))
        self.widget_layout = QtWidgets.QHBoxLayout()
        self.widget_layout.addWidget(self.widgets[0])
        layout.addRow(self.name, self.widget_layout)
        super(FloatOption, self).create_widget(layout, parent)

    def load_options(self):
        settings = dcc_core.Application.instance().get_settings()
        value = settings.get_double(self.setting, self.default)
        self.widgets[0].setValue(value)
        super(FloatOption, self).load_options()

    def save_options(self):
        settings = dcc_core.Application.instance().get_settings()
        value = self.widgets[0].value()
        settings.set_double(self.setting, value)
        super(FloatOption, self).save_options()

    def get_value(self):
        return self.widgets[0].value()


class IntOption(Option):
    def create_widget(self, layout, parent):
        self.widgets.append(QtWidgets.QSpinBox(parent))
        self.widgets[0].setButtonSymbols(QtWidgets.QAbstractSpinBox.NoButtons)
        self.widgets[0].setMaximumWidth(70)
        if "min" in self.meta:
            self.widgets[0].setMinimum(self.meta["min"])
        if "max" in self.meta:
            self.widgets[0].setMaximum(self.meta["max"])
        self.widget_layout = QtWidgets.QHBoxLayout()
        self.widget_layout.addWidget(self.widgets[0])
        layout.addRow(self.name, self.widget_layout)
        super(IntOption, self).create_widget(layout, parent)

    def load_options(self):
        settings = dcc_core.Application.instance().get_settings()
        value = settings.get_int(self.setting, self.default)
        self.widgets[0].setValue(value)
        super(IntOption, self).load_options()

    def save_options(self):
        settings = dcc_core.Application.instance().get_settings()
        value = self.widgets[0].value()
        settings.set_int(self.setting, value)
        super(IntOption, self).save_options()

    def get_value(self):
        return self.widgets[0].value()


class BoolOption(Option):
    def create_widget(self, layout, parent):
        self.widgets.append(QtWidgets.QCheckBox())
        self.widgets[0].setTristate(False)
        self.widget_layout = QtWidgets.QHBoxLayout()
        self.widget_layout.addWidget(self.widgets[0])
        layout.addRow(self.name, self.widget_layout)
        super(BoolOption, self).create_widget(layout, parent)

    def load_options(self):
        settings = dcc_core.Application.instance().get_settings()
        value = settings.get_bool(self.setting, self.default)
        self.widgets[0].setChecked(value)
        super(BoolOption, self).load_options()

    def save_options(self):
        settings = dcc_core.Application.instance().get_settings()
        value = self.widgets[0].isChecked()
        settings.set_bool(self.setting, value)
        super(BoolOption, self).save_options()

    def get_value(self):
        return self.widgets[0].isChecked()


class OptionOption(Option):  # lol
    def __init__(self, name, setting, meta, default):
        super(OptionOption, self).__init__(name, setting, meta, default)
        self.data_options = False

    def create_widget(self, layout, parent):
        self.widgets.append(QtWidgets.QComboBox())
        for option in self.meta["options"]:
            if isinstance(option, list):
                self.widgets[0].addItem(*option)
                self.data_options = True
            else:
                self.widgets[0].addItem(option)
        self.widget_layout = QtWidgets.QHBoxLayout()
        self.widget_layout.addWidget(self.widgets[0])
        self.widget_layout.addStretch()
        layout.addRow(self.name, self.widget_layout)
        super(OptionOption, self).create_widget(layout, parent)

    def load_options(self):
        settings = dcc_core.Application.instance().get_settings()
        value = settings.get_string(self.setting, self.default)
        if self.data_options:
            index = self.widgets[0].findData(value)
            self.widgets[0].setCurrentIndex(index)
        else:
            self.widgets[0].setCurrentText(value)
        super(OptionOption, self).load_options()

    def save_options(self):
        settings = dcc_core.Application.instance().get_settings()
        if self.data_options:
            value = self.widgets[0].currentData()
            settings.set_string(self.setting, value)
        else:
            value = self.widgets[0].currentText()
            settings.set_string(self.setting, value)
        super(OptionOption, self).save_options()

    def get_value(self):
        value = None
        if self.data_options:
            value = self.widgets[0].currentData()
        else:
            value = self.widgets[0].currentText()
        return value


class Float2Option(Option):
    def create_widget(self, layout, parent):
        self.widgets.append(FloatWidget(parent))
        self.widgets.append(FloatWidget(parent))
        self.widget_layout = QtWidgets.QHBoxLayout()
        self.widget_layout.addWidget(self.widgets[0])
        self.widget_layout.addWidget(self.widgets[1])
        layout.addRow(self.name, self.widget_layout)
        super(Float2Option, self).create_widget(layout, parent)

    def load_options(self):
        settings = dcc_core.Application.instance().get_settings()
        value0 = settings.get_double(self.setting + "0", self.default[0])
        value1 = settings.get_double(self.setting + "1", self.default[1])
        self.widgets[0].setValue(value0)
        self.widgets[1].setValue(value1)
        super(Float2Option, self).load_options()

    def save_options(self):
        settings = dcc_core.Application.instance().get_settings()
        value0 = self.widgets[0].value()
        value1 = self.widgets[1].value()
        settings.set_double(self.setting + "0", value0)
        settings.set_double(self.setting + "1", value1)
        super(Float2Option, self).save_options()

    def get_value(self):
        return self.widgets[0].value(), self.widgets[1].value()


class StringOption(Option):
    def create_widget(self, layout, parent):
        self.widgets.append(QtWidgets.QLineEdit())
        self.widget_layout = QtWidgets.QHBoxLayout()
        self.widget_layout.addWidget(self.widgets[0])
        layout.addRow(self.name, self.widget_layout)
        super(StringOption, self).create_widget(layout, parent)

    def load_options(self):
        settings = dcc_core.Application.instance().get_settings()
        value = settings.get_string(self.setting, self.default)
        self.widgets[0].setText(value)
        super(StringOption, self).load_options()

    def save_options(self):
        settings = dcc_core.Application.instance().get_settings()
        value = self.widgets[0].text()
        settings.set_string(self.setting, value)
        super(StringOption, self).save_options()

    def get_value(self):
        return self.widgets[0].text()


def make_option(name, setting, meta, default):
    if meta["type"] == "spacer":
        return SpacerOption(name, setting, meta, default)
    elif meta["type"] == "float":
        return FloatOption(name, setting, meta, default)
    elif meta["type"] == "int":
        return IntOption(name, setting, meta, default)
    elif meta["type"] == "bool":
        return BoolOption(name, setting, meta, default)
    elif meta["type"] == "option":
        return OptionOption(name, setting, meta, default)
    elif meta["type"] == "float2":
        return Float2Option(name, setting, meta, default)
    elif meta["type"] == "string":
        return StringOption(name, setting, meta, default)
    else:
        return Option(name, setting, meta, default)


class OptionsWidget(QtWidgets.QWidget):
    def __init__(self, title, button, callback, options, parent=None):
        QtWidgets.QWidget.__init__(self, parent=parent)
        self.setWindowFlags(QtCore.Qt.Window)
        title_ending = i18n("edit_menu.options_widget", "Options")
        self.setWindowTitle("{} {}".format(title, title_ending))
        self.setObjectName("{}_options".format(title.lower().replace(" ", "_")))
        self.resize(400, 200)
        self.title = title
        self.callback = callback
        self.options = copy.deepcopy(options)

        self._layout = QtWidgets.QVBoxLayout(self)

        options_layout = QtWidgets.QFormLayout()
        self._layout.addLayout(options_layout)

        for option in self.options:
            option.create_widget(options_layout, self)

        self._layout.addSpacerItem(QtWidgets.QSpacerItem(0, 0, QtWidgets.QSizePolicy.Expanding))
        self._buttons_layout = QtWidgets.QHBoxLayout()
        self._layout.addLayout(self._buttons_layout)
        if callback:
            self.do_button = QtWidgets.QPushButton(button)
            self._buttons_layout.addWidget(self.do_button)
            self.do_button.clicked.connect(self.do)

        self.save_button = QtWidgets.QPushButton(i18n("edit_menu.options_widget", "Save and Close"))
        self.cancel_button = QtWidgets.QPushButton(i18n("edit_menu.options_widget", "Cancel"))
        self._layout.setAlignment(self._buttons_layout, QtCore.Qt.AlignBottom)

        self._buttons_layout.addWidget(self.save_button)
        self._buttons_layout.addWidget(self.cancel_button)

        self.save_button.clicked.connect(self.save_options)
        self.cancel_button.clicked.connect(self.close)

    def load_options(self):
        for option in self.options:
            option.load_options()

    def save_options(self):
        for option in self.options:
            option.save_options()
        self.close()

    def do(self):
        self.save_options()
        self.callback()

    def showEvent(self, event):
        app = dcc_core.Application.instance()
        main_window = app.get_main_window()
        self.move(main_window.geometry().center() - self.rect().center())
        self.load_options()  # because actions can share options
        QtWidgets.QWidget.showEvent(self, event)
