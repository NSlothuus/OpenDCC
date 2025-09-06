# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import opendcc.core as dcc_core
import opendcc.attribute_editor as ae

from PySide6 import QtCore, QtGui, QtWidgets


class BakeWindow(QtWidgets.QWidget):
    def bake_all(self, layer, start_frame, end_frame, samples, remove_origin):
        raise NotImplementedError("bake_all")

    def bake_selected(self, layer, start_frame, end_frame, samples, remove_origin):
        raise NotImplementedError("bake_selected")

    def type_string(self):  # type_string() - "animtion curves", "expressions"
        return ""

    def __init__(self, layer, *args):

        QtWidgets.QWidget.__init__(self, *args)

        self.layer = layer
        app = dcc_core.Application.instance()
        session = app.get_session()
        self.stage = session.get_current_stage()
        bakeButton = QtWidgets.QPushButton("Bake " + self.type_string())
        mainLayout = QtWidgets.QVBoxLayout()
        self.setLayout(mainLayout)
        self.layout().setSizeConstraint(QtWidgets.QLayout.SetNoConstraint)
        self.setWindowTitle("Bake " + self.type_string())
        self.setWindowFlags(QtCore.Qt.Window)

        pathLayout = QtWidgets.QHBoxLayout()
        pathLayout.addWidget(QtWidgets.QLabel("Layer : " + layer.GetDisplayName()))
        pathLayout.addWidget(bakeButton)
        mainLayout.addLayout(pathLayout)

        optionsLayout = QtWidgets.QVBoxLayout()
        framesLayoyut = QtWidgets.QHBoxLayout()
        framesLayoyut.addWidget(QtWidgets.QLabel("Frame Range"))
        self.startFrameWidget = QtWidgets.QSpinBox()
        self.startFrameWidget.setMaximum(10000000)
        self.startFrameWidget.setMinimum(-10000000)
        self.startFrameWidget.setValue(self.stage.GetStartTimeCode())

        self.startFrameWidget.setToolTip("Start Frame")
        self.endFrameWidget = QtWidgets.QSpinBox()
        self.endFrameWidget.setMaximum(10000000)
        self.endFrameWidget.setMinimum(-10000000)
        self.endFrameWidget.setValue(self.stage.GetEndTimeCode())
        self.endFrameWidget.setToolTip("End Frame")
        framesLayoyut.addWidget(self.startFrameWidget)
        framesLayoyut.addWidget(self.endFrameWidget)
        optionsLayout.addLayout(framesLayoyut)

        shutterLayoyt = QtWidgets.QHBoxLayout()
        shutterLayoyt.addWidget(QtWidgets.QLabel("Shutter"))
        self.shutter = QtWidgets.QDoubleSpinBox()
        self.shutter.setValue(0.0)
        shutterLayoyt.addWidget(self.shutter)
        optionsLayout.addLayout(shutterLayoyt)

        all_or_selection_layoyut = QtWidgets.QHBoxLayout()
        self.bake_all_radio_batton = QtWidgets.QRadioButton("All")
        self.bake_selection = QtWidgets.QRadioButton("Selection")
        self.bake_selection.setChecked(True)
        all_or_selection_layoyut.addWidget(self.bake_selection)
        all_or_selection_layoyut.addWidget(self.bake_all_radio_batton)
        optionsLayout.addLayout(all_or_selection_layoyut)

        self.remove_origin = QtWidgets.QCheckBox("Remove origin " + self.type_string())
        self.remove_origin.setChecked(True)
        optionsLayout.addWidget(self.remove_origin)

        optionsLayout.addStretch()
        mainLayout.addLayout(optionsLayout)

        buttonsLayout = QtWidgets.QHBoxLayout()
        buttonsLayout.addStretch()
        buttonsLayout.addWidget(bakeButton)
        mainLayout.addLayout(buttonsLayout)

        mainLayout.setSizeConstraint(QtWidgets.QLayout.SetFixedSize)  # forbid shrinking

        bakeButton.clicked.connect(self.bake)

    def bake(self):
        start_frame = self.startFrameWidget.value()
        end_frame = self.endFrameWidget.value()
        shutter = self.shutter.value()
        remove_origin = self.remove_origin.isChecked()
        if shutter < 0.001:
            samples = [0]
        else:
            samples = [-shutter, 0, shutter]

        if self.bake_all_radio_batton.isChecked():
            ok = self.bake_all(self.layer, start_frame, end_frame, samples, remove_origin)
        else:
            ok = self.bake_selected(self.layer, start_frame, end_frame, samples, remove_origin)

        ae.update_all_attribute_editors()

        if ok:
            print("Bake OK")
        else:
            print("Bake Failed")
        self.close()
