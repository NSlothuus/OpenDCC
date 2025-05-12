# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets, QtCore
from opendcc import packaging
import opendcc.core as dcc_core

from opendcc.i18n import i18n


def load_plugin_settings(packages):
    settings = dcc_core.Application.instance().get_settings()

    result = {}
    for package in packages:
        result[package.get_name()] = settings.get_bool(
            "opendcc.plugin_manager.packages.{}.autoload".format(package.get_name()), False
        )
    return result


def load_plugin(pkg_name):
    res = False
    try:
        pkg_registry = dcc_core.Application.instance().get_package_registry()
        res = pkg_registry.load(pkg_name)
    except Exception as exc:
        try:
            print("Unable to load package {0}: {1}".format(pkg_name, exc))
        except:
            print("Unable to load package {0}".format(pkg_name))

    return res


from pxr import Tf
from pxr import Plug


class PyHooksContainer(object):
    pass


PyHooksTfType = Tf.Type.Define(PyHooksContainer)


class PluginManager(QtWidgets.QWidget):
    def __init__(self, parent=None):
        QtWidgets.QWidget.__init__(self, parent=parent)
        self.setWindowFlags(QtCore.Qt.Window)
        self.setWindowTitle(i18n("plugin_manager", "Plug-in Manager"))
        self.setObjectName("plugin_manager")
        self.resize(300, 400)

        self._main_layout = QtWidgets.QVBoxLayout(self)
        self._main_layout.setContentsMargins(0, 0, 0, 0)
        self._scroll_area = QtWidgets.QScrollArea(parent=self)
        self._main_widget = QtWidgets.QWidget(self)
        self._scroll_area.setWidget(self._main_widget)
        self._scroll_area.setWidgetResizable(True)
        self._main_layout.addWidget(self._scroll_area)
        self._layout = QtWidgets.QGridLayout(self._main_widget)

        pkg_registry = dcc_core.Application.instance().get_package_registry()
        packages = pkg_registry.get_all_packages()
        self.auto_load_settings = load_plugin_settings(packages)

        packages_dict = {package.get_name(): package for package in packages}
        if not packages:
            return

        row = 0
        for pkg_name in sorted(packages_dict.keys()):
            pkg = packages_dict[pkg_name]
            self._layout.addWidget(QtWidgets.QLabel(pkg_name), row, 0)

            chbx_loaded = QtWidgets.QCheckBox(i18n("plugin_manager", "Loaded"))
            chbx_loaded.pkg_name = pkg_name
            chbx_loaded.setChecked(pkg.is_loaded())
            if pkg.is_loaded():
                chbx_loaded.setChecked(True)
                chbx_loaded.setEnabled(False)

            chbx_loaded.stateChanged.connect(
                lambda state, chbx=chbx_loaded: self.loaded_changed(state, chbx)
            )
            chbx_loaded.setTristate(False)
            self._layout.addWidget(chbx_loaded, row, 1)

            chbx_autoload = QtWidgets.QCheckBox(i18n("plugin_manager", "Auto load"))
            chbx_autoload.pkg_name = pkg_name
            if self.auto_load_settings.get(pkg_name):
                chbx_autoload.setChecked(True)

            chbx_autoload.stateChanged.connect(
                lambda state, chbx=chbx_autoload: self.change_autoload_setting(state, chbx)
            )
            self._layout.addWidget(chbx_autoload, row, 2)
            row += 1
        self._layout.setRowStretch(self._layout.rowCount(), 1)

    def loaded_changed(self, state, check_box):
        if state:
            res = load_plugin(check_box.pkg_name)
            if res:
                check_box.setEnabled(False)
            else:
                check_box.setChecked(False)

    def change_autoload_setting(self, state, check_box):
        settings = dcc_core.Application.instance().get_settings()
        if state:
            settings.set_bool(
                "opendcc.plugin_manager.packages.{}.autoload".format(check_box.pkg_name), True
            )
        else:
            settings.remove(
                "opendcc.plugin_manager.packages.{}.autoload".format(check_box.pkg_name)
            )

    @staticmethod
    def show_window():
        window = PluginManager(dcc_core.Application.instance().get_main_window())
        window.show()
