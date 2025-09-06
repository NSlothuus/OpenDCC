# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import unittest
import sys
import os
from PySide6 import QtCore, QtGui, QtWidgets
from PySide6.QtTest import QTest
import script_editor
from _conf import SETTINGS_PATH

path = os.path.join(SETTINGS_PATH, "config.ini")
if os.path.exists(path):
    os.remove(os.path.join(SETTINGS_PATH, "config.ini"))
app = QtWidgets.QApplication(sys.argv)


class TestScriptEditor(unittest.TestCase):
    code_examples = [
        ("print 1", "1\n"),
        ("10", "10\n"),
        ("a=10", ""),
        ("a", "10\n"),
        ("import sys\nprint sys.__name__", "sys\n"),
        (
            "a=b",
            "Traceback (most recent call last):\n  File \"<input>\", line 1, in <module>\nNameError: name 'b' is not defined\n",
        ),
    ]

    def setUp(self):
        self.widget = script_editor.ScriptEditor()
        self.code_edit = self.widget.tabwidget.currentWidget().code_edit
        self.output_edit = self.widget.output_edit

    def run_code(self, code, res):
        self.output_edit.clear()
        self.code_edit.setPlainText(code)
        self.widget.run_code()
        output = self.output_edit.toPlainText()
        self.assertEqual(output, res)

    def test_run(self):
        for code, res in self.code_examples:
            self.run_code(code, res)

    def run_with_hotkey(self, code, res):
        self.output_edit.clear()
        self.code_edit.setPlainText(code)
        QTest.keyPress(self.code_edit, QtCore.Qt.Key_Enter)
        output = self.output_edit.toPlainText()
        self.assertEqual(output, res)

    def test_run_with_hotkey(self):
        for code, res in self.code_examples:
            self.run_with_hotkey(code, res)

    def run_with_buttons(self, code, res):
        self.output_edit.clear()
        self.code_edit.setPlainText(code)
        QTest.mouseClick(
            self.widget.toolbar.widgetForAction(self.widget.run_act),
            QtCore.Qt.LeftButton,
        )
        output = self.output_edit.toPlainText()
        self.assertEqual(output, res)

    def test_run_with_buttons(self):
        for code, res in self.code_examples:
            self.run_with_buttons(code, res)

    def test_run_selected_code_with_hotkey(self):
        self.output_edit.clear()
        self.code_edit.setPlainText("print 1\nprint 2\nprint 3")
        cursor = self.widget.textCursor()
        cursor.setPosition(8)
        cursor.setPosition(16, QtGui.QTextCursor.KeepAnchor)
        self.code_edit.setTextCursor(cursor)
        QTest.keyPress(self.code_edit, QtCore.Qt.Key_Enter)
        output = self.output_edit.toPlainText()
        self.assertEqual(output, "2\n")

    def test_run_selected_code_with_buttons(self):
        self.output_edit.clear()
        self.code_edit.setPlainText("print 1\nprint 2\nprint 3")
        cursor = self.code_edit.textCursor()
        cursor.setPosition(8)
        cursor.setPosition(16, QtGui.QTextCursor.KeepAnchor)
        self.code_edit.setTextCursor(cursor)
        QTest.mouseClick(
            self.widget.toolbar.widgetForAction(self.widget.run_act),
            QtCore.Qt.LeftButton,
        )
        output = self.output_edit.toPlainText()
        self.assertEqual(output, "2\n")

    def test_empty_output(self):
        code = "print 1"
        self.code_edit.setPlainText(code)
        self.widget.run_code()
        QTest.mouseClick(
            self.widget.toolbar.widgetForAction(self.widget.clean_output_act),
            QtCore.Qt.LeftButton,
        )
        output = self.output_edit.toPlainText()
        self.assertEqual(output, "")

    def test_tabs_behavior(self):
        self.assertEqual(self.widget.tabwidget.count(), 1)
        QTest.mouseClick(self.widget.tabwidget.cornerWidget(), QtCore.Qt.LeftButton)
        self.assertEqual(self.widget.tabwidget.count(), 2)
        btn = self.widget.tabwidget.tabBar().tabButton(0, QtWidgets.QTabBar.RightSide)
        QTest.mouseClick(btn, QtCore.Qt.LeftButton)
        self.assertEqual(self.widget.tabwidget.count(), 1)


if __name__ == "__main__":
    unittest.main()
