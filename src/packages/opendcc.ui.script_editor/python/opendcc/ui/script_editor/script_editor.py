# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtCore, QtGui, QtWidgets
from code import InteractiveInterpreter
import sys
import os
from functools import partial
import re
import codecs
import json

import opendcc.core as dcc_core
from opendcc.ui.code_editor.code_editor import CodeEditor

from ._conf import SETTINGS_PATH

from opendcc.color_theme import get_color_theme, ColorTheme
from opendcc.i18n import i18n


class TabContent:
    def __init__(self, tab_name, tab_code, file_path, dirty):
        self.tab_name = tab_name
        self.tab_code = tab_code
        self.file_path = file_path
        self.dirty = dirty


class ScriptOut(object):
    def __init__(self, widget, format=None, sysout=None):
        self.widget = widget
        self.format = format
        self.sysout = sysout

    def set_format(self, format):
        self.format = format

    def write(self, message):
        curs = self.widget.textCursor()
        curs.movePosition(QtGui.QTextCursor.End, QtGui.QTextCursor.MoveAnchor)
        if self.format:
            curs.insertText(message, self.format)
        else:
            curs.insertText(message)

        # duplicate message to another output if needed
        if self.sysout:
            self.sysout.write(message)

        # scroll to cursor
        self.widget.setTextCursor(curs)
        self.widget.ensureCursorVisible()

    def flush(self):
        pass


class SearchScriptEditor(QtWidgets.QDialog):
    """Search Script Editor dialog"""

    def __init__(self, parent=None):
        QtWidgets.QDialog.__init__(self, parent)
        self.setWindowFlags(self.windowFlags() & ~QtCore.Qt.WindowContextHelpButtonHint)
        self.setWindowTitle(i18n("script_editor.search", "Search"))
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)

        layout = QtWidgets.QVBoxLayout()
        layout.addWidget(QtWidgets.QLabel(i18n("script_editor.search", "Search String:")))
        self.input = QtWidgets.QLineEdit()
        self.input.setPlaceholderText(i18n("script_editor.search", "Find..."))
        layout.addWidget(self.input)

        buttons_layout = QtWidgets.QHBoxLayout()
        buttons_layout.addStretch()

        search_tabs_button = QtWidgets.QPushButton(i18n("script_editor.search", "Search Tabs"))
        search_tabs_button.clicked.connect(self.search_tabs)
        search_output_button = QtWidgets.QPushButton(i18n("script_editor.search", "Search Output"))
        search_output_button.clicked.connect(self.search_output)
        cancel_button = QtWidgets.QPushButton(i18n("script_editor.search", "Cancel"))
        cancel_button.clicked.connect(self.close)

        buttons_layout.addWidget(search_tabs_button)
        buttons_layout.addWidget(search_output_button)
        buttons_layout.addWidget(cancel_button)
        layout.addLayout(buttons_layout)

        self.setLayout(layout)
        self.result = None

    def search_tabs(self):
        self.result = ["tabs", self.input.text()]
        self.close()

    def search_output(self):
        self.result = ["output", self.input.text()]
        self.close()


class EditorWidget(QtWidgets.QWidget):
    enter_pressed = QtCore.Signal()

    def __init__(self, title, parent, script_editor, file_path):
        QtWidgets.QWidget.__init__(self, parent=parent)
        self.layout = QtWidgets.QVBoxLayout()
        self.layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(self.layout)
        if not title:
            title = "Tab"
        parent.addTab(self, title)
        self.script_editor = script_editor
        self.file_path = file_path
        self.dirty = False

        self.setupEditor()

    def setupEditor(self):
        self.code_edit = CodeEditor(lang="python", highlight_line=True, line_numbers=True)
        self.layout.addWidget(self.code_edit)

        self.code_edit.code_editor.setFocus()
        self.code_edit.code_editor.enter_pressed.connect(self.run_code)
        if self.file_path:
            self.code_edit.code_editor.modificationChanged.connect(self.set_dirty)
            self.file_watcher = QtCore.QFileSystemWatcher(self)
            self.file_watcher.addPath(self.file_path)
            self.file_watcher.fileChanged.connect(self.file_changed)
            self.itself = False

    def file_changed(self, file_path):
        if self.itself:
            self.itself = False
            return

        # clear signals because it seems to be bugged or something
        self.file_watcher.fileChanged.disconnect(self.file_changed)

        load = True
        if self.dirty:
            mb = QtWidgets.QMessageBox()
            dirty_msg = i18n("script_editor", "has been modified externally, load changes?")
            ret = mb.question(
                self,
                i18n("script_editor", "Load Changes?"),
                '"%s" %s' % (self.file_path, dirty_msg),
                mb.Yes | mb.No,
            )
            if ret != mb.Yes:
                load = False
        if load:
            with open(self.file_path, "r") as tab_file:
                code_text = tab_file.read()
                self.set_code(code_text)

        self.file_watcher.fileChanged.connect(self.file_changed)

    def set_code(self, code):
        self.code_edit.code_editor.setPlainText(code)
        self.code_edit.code_editor.document().setModified(False)

    def run_code(self):
        self.enter_pressed.emit()

    def setPythonLocals(self, locals):
        self.code_edit.code_editor.setPythonLocals(locals)

    def set_dirty(self, dirty):
        if self.file_path:
            self.dirty = dirty
            tab_bar = self.script_editor.tabwidget.tabBar()
            for index in range(tab_bar.count()):
                widget = self.script_editor.tabwidget.widget(index)
                if self.file_path == widget.file_path:
                    text = tab_bar.tabText(index)
                    if text.endswith("*"):
                        text = text[:-1]
                    if dirty:
                        tab_bar.setTabText(index, text + "*")
                    else:
                        tab_bar.setTabText(index, text)

    def save(self):
        if self.file_path:
            with codecs.open(self.file_path, "w", encoding="utf-8") as tab_file:
                tab_file.write(self.code_edit.code_editor.toPlainText())
            self.code_edit.code_editor.document().setModified(False)
            self.dirty = False
            self.itself = True


class ScriptEditor(QtWidgets.QWidget):
    __interpreter = None  # There must be only one, I think
    __save_tabs_before_execute = None
    __widget = None

    @staticmethod
    def getSaveTabs():
        return ScriptEditor.__save_tabs_before_execute

    @staticmethod
    def setSaveTabs(value):
        ScriptEditor.__save_tabs_before_execute = value

    @staticmethod
    def getInterpreter():
        """Static access method."""
        if ScriptEditor.__interpreter == None:
            ScriptEditor.__interpreter = InteractiveInterpreter(locals())
        return ScriptEditor.__interpreter

    @staticmethod
    def getWidget():
        return ScriptEditor.__widget

    def __init__(self, host="dcc", parent=None):
        QtWidgets.QWidget.__init__(self, parent=parent)

        ScriptEditor.__widget = self

        self.host = host
        if self.host == "dcc":
            self.settings_path = SETTINGS_PATH
            if ScriptEditor.__save_tabs_before_execute is None:
                ScriptEditor.__save_tabs_before_execute = (
                    dcc_core.Application.instance()
                    .get_settings()
                    .get_bool("scripteditor.save_before_execute", True)
                )
        else:
            if ScriptEditor.__save_tabs_before_execute is None:
                ScriptEditor.__save_tabs_before_execute = True
            self.settings_path = os.path.join(SETTINGS_PATH, self.host)

        self.deleted_tabs = []

        self.setWindowTitle("Python Script Editor")
        self.layout = QtWidgets.QVBoxLayout()
        self.setLayout(self.layout)

        self.layout.setContentsMargins(0, 0, 0, 0)
        self.layout.setSpacing(0)

        self.toolbar = self.create_toolbar()
        self.toolbar_layout = QtWidgets.QVBoxLayout()
        self.layout.addLayout(self.toolbar_layout)
        self.toolbar_layout.addWidget(self.toolbar)
        self.toolbar_layout.setContentsMargins(0, 0, 0, 0)

        self.splitter = QtWidgets.QSplitter(QtCore.Qt.Vertical)
        self.layout.addWidget(self.splitter)
        self.setupOutputEdit()

        self.tabwidget = QtWidgets.QTabWidget()
        self.tabwidget.setDocumentMode(True)
        self.splitter.addWidget(self.tabwidget)
        self.splitter.setStretchFactor(1, 10)
        self.tabwidget.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)

        self.new_tab_action = QtWidgets.QAction(i18n("script_editor", "New Tab"), self)
        self.new_tab_action.setShortcut("Ctrl+T")
        self.new_tab_action.setShortcutContext(QtCore.Qt.WidgetShortcut)
        self.new_tab_action.triggered.connect(self.add_new_tab)
        self.addAction(self.new_tab_action)

        self.open_action = QtWidgets.QAction(i18n("script_editor", "Open"), self)
        self.open_action.setShortcut("Ctrl+O")
        self.open_action.setShortcutContext(QtCore.Qt.WidgetShortcut)
        self.open_action.triggered.connect(self.open_file)
        self.addAction(self.open_action)

        self.save_action = QtWidgets.QAction(i18n("script_editor", "Save"), self)
        self.save_action.setShortcut("Ctrl+S")
        self.save_action.setShortcutContext(QtCore.Qt.WidgetShortcut)
        self.save_action.triggered.connect(self.save_tab)
        self.addAction(self.save_action)

        self.close_current_tab_action = QtWidgets.QAction(
            i18n("script_editor", "Close Current Tab"), self
        )
        self.close_current_tab_action.setShortcut("Ctrl+W")
        self.close_current_tab_action.setShortcutContext(QtCore.Qt.WidgetShortcut)
        self.close_current_tab_action.triggered.connect(self.close_tab)
        self.addAction(self.close_current_tab_action)

        self.open_closed_tab_action = QtWidgets.QAction(
            i18n("script_editor", "Open Closed Tab"), self
        )
        self.open_closed_tab_action.setShortcut("Ctrl+Shift+T")
        self.open_closed_tab_action.setShortcutContext(QtCore.Qt.WidgetShortcut)
        self.open_closed_tab_action.triggered.connect(self.restore_last_closed_tab)
        self.addAction(self.open_closed_tab_action)

        self.locals = locals()
        del self.locals["self"]  # because it's strange, I think

        self.open_dir = ""

        if not self.load_settings():
            self.apply_default_tab_settings()
        self.apply_tabwidget_settings()

        if not ScriptEditor.__interpreter:
            ScriptEditor.__interpreter = InteractiveInterpreter(self.locals)

        self.apply_output_edit_settings()

        QtWidgets.QApplication.instance().aboutToQuit.connect(self.on_app_close)

        self.setAcceptDrops(True)

    def closeEvent(self, event):
        self.save_settings()
        QtWidgets.QWidget.closeEvent(self, event)

    def dragEnterEvent(self, event):
        mime_data = event.mimeData()
        if mime_data.hasUrls():
            event.accept()
            return
        event.ignore()

    def dropEvent(self, event):
        for url in event.mimeData().urls():
            self.add_new_tab(None, url.toLocalFile())

    def setupOutputEdit(self):
        self.output_edit = QtWidgets.QTextEdit()
        self.splitter.addWidget(self.output_edit)
        self.output_edit.setReadOnly(True)

    def apply_output_edit_settings(self):
        """
        Create loggers, output format
        and then redirect output to our output widget
        """
        font = QtGui.QFont()
        font.setFamily("Courier")
        font.setFixedPitch(True)
        font.setPointSize(10)
        self.out_format = self.output_edit.currentCharFormat()
        self.out_format.setFont(font)
        self.code_out_format = QtGui.QTextCharFormat()
        if get_color_theme() == ColorTheme.LIGHT:
            self.code_out_format.setForeground(QtGui.QBrush(QtGui.QColor(0, 191, 8)))
        else:
            self.code_out_format.setForeground(QtGui.QBrush(QtGui.QColor(127, 202, 222)))
        self.code_out_format.setFont(font)
        self.err_out_format = QtGui.QTextCharFormat()
        self.err_out_format.setForeground(QtGui.QBrush(QtGui.QColor(225, 120, 120)))
        self.err_out_format.setFont(font)

        self.logger = ScriptOut(self.output_edit, self.out_format)
        self.logger_err = ScriptOut(self.output_edit, self.err_out_format, sys.stderr)

        self.redirect_log_to_output_widget()

    def redirect_log_to_output_widget(self):
        self.sysstdout = sys.stdout
        self.sysstderr = sys.stderr
        sys.stdout = self.logger
        sys.stderr = self.logger_err

    def redirect_log_to_std(self):
        sys.stdout = self.sysstdout
        sys.stderr = self.sysstderr

    def create_toolbar(self):
        toolbar = QtWidgets.QToolBar()
        toolbar.setIconSize(QtCore.QSize(16, 16))

        self.run_act = QtWidgets.QAction(
            QtGui.QIcon(":/icons/run"), i18n("script_editor", "Run Code"), self
        )
        self.run_act.triggered.connect(self.run_code)
        toolbar.addAction(self.run_act)

        self.clear_output_act = QtWidgets.QAction(
            QtGui.QIcon(":/icons/small_garbage"), i18n("script_editor", "Clear Output"), self
        )
        self.clear_output_act.triggered.connect(self.clean_output)
        toolbar.addAction(self.clear_output_act)

        if self.host == "dcc":
            self.add_to_shelf_act = QtWidgets.QAction(
                QtGui.QIcon(":/icons/small_shelf"), i18n("script_editor", "Add to Shelf"), self
            )
            self.add_to_shelf_act.triggered.connect(self.add_to_shelf)
            toolbar.addAction(self.add_to_shelf_act)

        self.search_tabs_act = QtWidgets.QAction(
            QtGui.QIcon(":/icons/small_search"), i18n("script_editor", "Search"), self
        )
        self.search_tabs_act.triggered.connect(self.search_tabs)
        toolbar.addAction(self.search_tabs_act)

        return toolbar

    def add_to_shelf(self):
        # get current tab
        wdgt_on_crnt_tab = self.tabwidget.currentWidget()

        # get selected text
        cursor = wdgt_on_crnt_tab.code_edit.code_editor.textCursor()
        if cursor.hasSelection():
            code_text = cursor.selection().toPlainText()
            from opendcc.actions.tab_toolbar import Shelf

            shelf = Shelf.instance()
            shelf.create_custom_action(code_text)

    def search_tabs(self):
        search_dialog = SearchScriptEditor(self)
        search_dialog.exec_()

        if search_dialog.result:
            search_type = search_dialog.result[0]
            text = search_dialog.result[1]
            if search_type == "tabs":
                for index in range(self.tabwidget.count()):
                    widget = self.tabwidget.widget(index).code_edit.code_editor
                    cursor = widget.textCursor()
                    cursor.movePosition(QtGui.QTextCursor.Start, QtGui.QTextCursor.MoveAnchor)
                    widget.setTextCursor(cursor)
                    if widget.find(text, QtGui.QTextDocument.FindCaseSensitively):
                        self.tabwidget.setCurrentIndex(index)
                        widget.ensureCursorVisible()
                        return
            elif search_type == "output":
                cursor = self.output_edit.textCursor()
                cursor.movePosition(QtGui.QTextCursor.Start, QtGui.QTextCursor.MoveAnchor)
                self.output_edit.setTextCursor(cursor)
                self.output_edit.find(text, QtGui.QTextDocument.FindCaseSensitively)
                self.output_edit.ensureCursorVisible()

    def apply_default_tab_settings(self):
        self.add_new_tab()

    def apply_tabwidget_settings(self):
        self.tabwidget.setMovable(True)
        self.tabwidget.setTabsClosable(True)

        self.new_tab_btn = QtWidgets.QPushButton(QtGui.QIcon(":/icons/new_tab"), "")
        self.new_tab_btn.setFlat(True)
        self.new_tab_btn.setMinimumHeight(16)
        self.tabwidget.setCornerWidget(self.new_tab_btn, QtCore.Qt.TopRightCorner)

        self.new_tab_btn.pressed.connect(self.add_new_tab)
        self.tabwidget.tabCloseRequested.connect(self.close_tab)
        self.tabwidget.tabBarDoubleClicked.connect(self.rename_tab_open_dialog)
        self.tabwidget.customContextMenuRequested.connect(self._on_right_click)

        resourceDir = os.path.dirname(os.path.realpath(__file__)) + "/"
        if get_color_theme() == ColorTheme.LIGHT:
            sheet = open(os.path.join(resourceDir, "script_editor_tabbar_light.qss"), "r")
        else:
            sheet = open(os.path.join(resourceDir, "script_editor_tabbar.qss"), "r")

        sheetString = sheet.read()
        self.tabwidget.tabBar().setStyleSheet(sheetString)

    def _on_right_click(self, pos):
        tab_index = self.tabwidget.tabBar().tabAt(pos)
        menu = QtWidgets.QMenu(self)
        menu.addAction(self.new_tab_action)
        menu.addSeparator()
        rename_tab = menu.addAction(
            i18n("script_editor", "Rename Tab"),
            partial(lambda: self.rename_tab_open_dialog(tab_index)),
        )

        widget = self.tabwidget.widget(tab_index)
        if tab_index < 0 or widget.file_path:
            rename_tab.setEnabled(False)

        menu.addSeparator()
        menu.addAction(self.open_action)
        menu.addAction(self.save_action)
        menu.addSeparator()
        menu.addAction(self.close_current_tab_action)
        menu.addAction(self.open_closed_tab_action)
        menu.exec_(self.tabwidget.mapToGlobal(pos))

    def open_file(self):
        file_name, _ = QtWidgets.QFileDialog.getOpenFileName(
            self, i18n("script_editor", "Select file"), self.open_dir, filter="*.py"
        )
        if not file_name or not os.path.isfile(file_name):
            return
        self.open_dir = os.path.dirname(file_name)
        self.add_new_tab(None, file_name)

    def add_new_tab(self, title=None, file_path=None):
        if not file_path:
            title = self.gen_new_tab_name(title)
        else:
            for index in range(self.tabwidget.count()):
                widget = self.tabwidget.widget(index)
                if file_path == widget.file_path:
                    self.tabwidget.setCurrentIndex(index)
                    return
            title = os.path.basename(file_path)

        widget = EditorWidget(title, self.tabwidget, self, file_path)
        widget.setPythonLocals(self.locals)
        widget.enter_pressed.connect(self.run_code)
        index = self.tabwidget.count() - 1

        if file_path:
            self.tabwidget.setTabToolTip(index, file_path)
            with codecs.open(file_path, "r", encoding="utf-8") as tab_file:
                code_text = tab_file.read()
                widget.set_code(code_text)

        self.tabwidget.setCurrentIndex(index)

        return widget

    def get_tab_titles(self):
        tab_titles = []
        for index in range(self.tabwidget.count()):
            tab_titles.append(self.tabwidget.tabText(index))
        return tab_titles

    def gen_new_tab_name(self, title=None):
        tab_titles = self.get_tab_titles()
        if not title:
            num = len(tab_titles)
            title = i18n("script_editor", "tab")
        else:
            num = 1
        potential_name = title
        while True:
            if potential_name not in tab_titles:
                return potential_name
            else:
                potential_name = title + " {0}".format(num)
                num += 1

    def rename_tab(self, title, index=None):
        title = self.gen_new_tab_name(title)
        if not index:
            index = self.tabwidget.currentIndex()
        widget = self.tabwidget.widget(index)
        if widget.file_path:
            return
        if title not in self.get_tab_titles():
            self.tabwidget.setTabText(index, title)
        else:
            print(i18n("script_editor", "Something went wrong while renaming tabs!"))

    def save_tab(self, index=None):
        if not index:
            index = self.tabwidget.currentIndex()
        widget = self.tabwidget.widget(index)
        widget.save()

    def close_tab(self, index=None):
        if not index:
            index = self.tabwidget.currentIndex()

        widget = self.tabwidget.widget(index)
        title = self.tabwidget.tabText(index)
        if widget.dirty:
            mb = QtWidgets.QMessageBox()
            dirty_msg = i18n("script_editor", "has been modified, save changes?")
            ret = mb.question(
                self,
                i18n("script_editor", "Save Changes?"),
                '"%s" %s' % (title, dirty_msg),
                mb.Yes | mb.No,
            )
            if ret == mb.Yes:
                widget.save()

        # save content before close to restore it if needed
        self.deleted_tabs.append(
            TabContent(
                title, widget.code_edit.code_editor.toPlainText(), widget.file_path, widget.dirty
            )
        )

        self.tabwidget.removeTab(index)
        widget.deleteLater()

    def restore_last_closed_tab(self):
        if self.deleted_tabs:
            tab = self.deleted_tabs[-1]
            widget = self.add_new_tab(tab.tab_name, tab.file_path)
            widget.set_code(tab.tab_code)
            widget.set_dirty(tab.dirty)
            self.deleted_tabs.pop(-1)

    def rename_tab_open_dialog(self, index=None):
        if index == None:
            return
        if index < 0:
            return

        widget = self.tabwidget.widget(index)
        if widget.file_path:
            return

        title, ok = QtWidgets.QInputDialog.getText(
            self,
            i18n("script_editor", "Rename tab"),
            i18n("script_editor", "New title:"),
            QtWidgets.QLineEdit.Normal,
            "",
        )
        if ok:
            title = title.strip()
            if re.match("\\w+( *\\w+)*$", title):
                self.rename_tab(title, index)

    def output_code_text(self, code_text):
        """
        Before running the code we output it to the output widget
        with different color
        """
        separator = "#" * 30 + "\n"

        self.logger.set_format(self.code_out_format)
        self.logger.write("\n" + separator)
        self.logger.write(code_text + "\n")
        self.logger.write(separator)
        self.logger.set_format(self.out_format)

    def run_code(self):
        # save tabs because it can crash if it's enabled
        if ScriptEditor.getSaveTabs():
            self.save_settings()

        # get current tab
        wdgt_on_crnt_tab = self.tabwidget.currentWidget()

        # get selected text
        cursor = wdgt_on_crnt_tab.code_edit.code_editor.textCursor()
        command_string = wdgt_on_crnt_tab.code_edit.code_editor.toPlainText()
        if cursor.hasSelection():
            code_text = cursor.selection().toPlainText()
        else:
            code_text = command_string

        # get the number of lines, if 1 line - process it like idle
        # means print variable value without print statement
        code_text_list = code_text.splitlines()
        self.output_code_text(code_text)
        if len(code_text_list) <= 1:
            # symbol='single' - to execute single statement
            ScriptEditor.getInterpreter().runsource(code_text, symbol="single")
        else:
            # symbol='exec' - to execute the whole code
            ScriptEditor.getInterpreter().runsource(code_text, symbol="exec")

    def clean_output(self):
        self.output_edit.clear()

    def save_settings(self):
        """
        Save sequence of tab names and their content
        """
        # write tab's content
        tab_titles = []
        for index in range(self.tabwidget.count()):
            widget = self.tabwidget.widget(index)
            code_text = widget.code_edit.code_editor.toPlainText()
            tab_title = self.tabwidget.tabText(index)
            if widget.file_path:
                tab_titles.append("$" + widget.file_path)
                title = self.tabwidget.tabText(index)
                if widget.dirty:
                    mb = QtWidgets.QMessageBox()
                    dirty_msg = i18n("script_editor", "has been modified, save changes?")
                    ret = mb.question(
                        self,
                        i18n("script_editor", "Save Changes?"),
                        '"%s" %s' % (title, dirty_msg),
                        mb.Yes | mb.No,
                    )
                    if ret == mb.Yes:
                        widget.save()
            else:
                tab_titles.append(tab_title)

                # check if self.settings_path exists and create if it doesn't
                if not os.path.exists(self.settings_path):
                    try:
                        os.makedirs(self.settings_path)
                    except OSError as e:
                        print(e)
                        print(i18n("script_editor", "Can't save settings"))
                        return
                path = os.path.join(self.settings_path, "{}.py".format(tab_title))

                with codecs.open(path, "w", encoding="utf-8") as tab_file:
                    tab_file.write(code_text)

        # write metadata to config file
        config = {}
        config["Tabs"] = tab_titles
        config["Current_Tab"] = self.tabwidget.currentIndex()
        config["Open_Dir"] = self.open_dir

        path = os.path.join(self.settings_path, "config.json")
        with codecs.open(path, "w", encoding="utf-8") as configfile:
            json.dump(config, configfile, ensure_ascii=False, sort_keys=True, indent=4)

    def load_settings(self):
        # load metadata from config file if possible
        path = os.path.join(self.settings_path, "config.json")

        # TODO: remove this code after a while
        ini_path = os.path.join(self.settings_path, "config.ini")
        if not os.path.exists(path) and os.path.exists(ini_path):
            from six.moves import configparser

            config = configparser.SafeConfigParser()
            config.read(ini_path)

            try:
                tab_titles = config.get("Settings", "Tabs").split(",")
            except Exception as e:
                print(e)
                tab_titles = []

            try:
                current_tab_index = int(config.get("Settings", "Current_tab"))
            except Exception as e:
                print(e)
                current_tab_index = 0

            try:
                self.open_dir = config.get("Settings", "Open_Dir")
            except Exception as e:
                print(e)
                self.open_dir = ""

            if not tab_titles:
                return False

            for title in tab_titles:
                if not title.startswith("$"):
                    path = os.path.join(self.settings_path, "{}.py".format(title))
                    if not os.path.exists(path):
                        current_tab_index = 0
                        continue

                    with open(path, "r") as tab_file:
                        code_text = tab_file.read()
                        widget = self.add_new_tab(title)
                        widget.set_code(code_text)
                else:
                    if os.path.exists(title[1:]):
                        self.add_new_tab(None, title[1:])
            return True

        if not os.path.exists(path):
            return False

        with codecs.open(path, "r", encoding="utf-8") as configfile:
            config = json.load(configfile)

        if "Tabs" in config:
            tab_titles = config["Tabs"]
        else:
            tab_titles = []

        if "Current_Tab" in config:
            current_tab_index = config["Current_Tab"]
        else:
            current_tab_index = 0

        if "Open_Dir" in config:
            self.open_dir = config["Open_Dir"]
        else:
            self.open_dir = ""

        if not tab_titles:
            return False

        # apply settings, create tabs and fill them with content
        for title in tab_titles:
            if not title.startswith("$"):
                path = os.path.join(self.settings_path, "{}.py".format(title))
                if not os.path.exists(path):
                    current_tab_index = 0
                    continue

                with codecs.open(path, "r", encoding="utf-8") as tab_file:
                    code_text = tab_file.read()
                    widget = self.add_new_tab(title)
                    widget.set_code(code_text)
            else:
                if os.path.exists(title[1:]):
                    self.add_new_tab(None, title[1:])
                else:
                    current_tab_index = 0

        self.tabwidget.setCurrentIndex(current_tab_index)
        return True

    def on_app_close(self):
        """Overloaded method"""
        self.save_settings()
        self.redirect_log_to_std()

    def event(self, event):
        """Stop Ctrl+S Ctrl+O Shortcut propagation"""
        if event.type() == QtCore.QEvent.ShortcutOverride and (
            event.key() == QtCore.Qt.Key_S or event.key() == QtCore.Qt.Key_O
        ):
            event.accept()

        return QtWidgets.QWidget.event(self, event)

    def keyPressEvent(self, event):
        """Overloaded function for hotkeys"""
        if (
            event.key() == QtCore.Qt.Key_T
            and event.modifiers() & QtCore.Qt.ControlModifier
            and not (event.modifiers() & QtCore.Qt.ShiftModifier)
        ):
            self.add_new_tab()
        elif (
            event.key() == QtCore.Qt.Key_W
            and event.modifiers() & QtCore.Qt.ControlModifier
            and not (event.modifiers() & QtCore.Qt.ShiftModifier)
        ):
            self.close_tab()
        elif (
            event.key() == QtCore.Qt.Key_O
            and event.modifiers() & QtCore.Qt.ControlModifier
            and not (event.modifiers() & QtCore.Qt.ShiftModifier)
        ):
            self.open_file()
        elif (
            event.key() == QtCore.Qt.Key_S
            and event.modifiers() & QtCore.Qt.ControlModifier
            and not (event.modifiers() & QtCore.Qt.ShiftModifier)
        ):
            self.save_tab()
        elif (
            event.key() == QtCore.Qt.Key_T
            and event.modifiers() & QtCore.Qt.ControlModifier
            and event.modifiers() & QtCore.Qt.ShiftModifier
        ):
            self.restore_last_closed_tab()
        else:
            QtWidgets.QWidget.keyPressEvent(self, event)


def test():
    import sys
    import resources.resources

    app = QtWidgets.QApplication(sys.argv)
    window = ScriptEditor()
    window.setFixedWidth(600)
    window.setFixedHeight(800)
    window.show()

    sys.exit(app.exec_())


if __name__ == "__main__":
    test()
