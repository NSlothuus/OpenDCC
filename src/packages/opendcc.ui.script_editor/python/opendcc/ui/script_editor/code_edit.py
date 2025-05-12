# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtCore, QtGui, QtWidgets
import keyword
import ast
import re


class CodeInputWidget(QtWidgets.QTextEdit):
    """
    Text Editor for inputting code
    With Syntax Highlighting, autocomplete, tab-handling
    """

    enter_pressed = QtCore.Signal()

    TAB_SEQ = " " * 4

    def __init__(self, parent=None):
        QtWidgets.QTextEdit.__init__(self, parent=parent)
        self.prefix = ""
        self.keywords = set([word for word in keyword.kwlist if len(word) > 3])
        self.all_words = set()
        self.imported_names = {}
        self.define_completer()
        self.setLineWrapMode(QtWidgets.QTextEdit.NoWrap)
        self.locals = None

    def setPythonLocals(self, locals):
        self.locals = locals

    def define_completer(self):
        self.completer = QtWidgets.QCompleter()
        self.completer.setWidget(self)
        self.keyword_completer_model = QtCore.QStringListModel(self.keywords)
        self.completer.setModel(self.keyword_completer_model)
        self.completer.setCompletionMode(QtWidgets.QCompleter.PopupCompletion)
        self.completer.setCaseSensitivity(QtCore.Qt.CaseInsensitive)
        self.completer.setWrapAround(False)
        self.completer.activated.connect(self.insert_completion)

    def keyPressEvent(self, event):
        """Override method"""
        # if completer is visible ignore some key press in TextEdit
        if self.completer and self.completer.popup().isVisible():
            if event.key() in (
                QtCore.Qt.Key_Enter,
                QtCore.Qt.Key_Return,
                QtCore.Qt.Key_Tab,
                QtCore.Qt.Key_Backtab,
            ):
                event.ignore()
                return
            QtWidgets.QTextEdit.keyPressEvent(self, event)
        else:
            if event.key() == QtCore.Qt.Key_Enter:
                self.enter_pressed.emit()
            elif event.key() == QtCore.Qt.Key_Tab:
                self.add_spaces_instead_tab()
            elif (
                event.key() == QtCore.Qt.Key_Backtab and event.modifiers() & QtCore.Qt.ShiftModifier
            ):
                self.reduce_spaces()
            else:
                QtWidgets.QTextEdit.keyPressEvent(self, event)

        self.run_completer(event)

    def add_spaces_instead_tab(self):
        # TODO: 2 problems
        # After just one press tab selection is lost and you need to select again(
        # When you do undo after tab - you need to undo twice
        cursor = self.textCursor()
        if not cursor.hasSelection():
            cursor.insertText(self.TAB_SEQ)
        else:
            text = self.toPlainText()
            start, end = cursor.selectionStart(), cursor.selectionEnd()
            # add tab_seq in selected text
            code_text = text[start:end].replace("\n", "\n" + self.TAB_SEQ)
            cursor.insertText(code_text)

            # add tab_seq before first selected line
            # even if it's partially selected
            expression = QtCore.QRegExp("\n")
            start_of_line = expression.lastIndexIn(text[0:start]) + 1
            cursor.setPosition(start_of_line)
            cursor.insertText(self.TAB_SEQ)

    def reduce_spaces(self):
        # TODO: same as with tab
        cursor = self.textCursor()
        if not cursor.hasSelection():
            cursor.select(QtGui.QTextCursor.LineUnderCursor)
            spaces_expression = QtCore.QRegExp("^([ ]+)")
            text = cursor.selectedText()
            index = spaces_expression.indexIn(text)
            # only if there a spaces in the beginning of the line
            if index != -1:
                cursor.insertText(text.replace(" ", "", 4))
        else:
            text = self.toPlainText()
            start, end = cursor.selectionStart(), cursor.selectionEnd()
            # reduce spaces in selected text
            cursor.insertText(re.sub("\n([ ]{1,4})", "\n", text[start:end]))

            # reduce spaces before first selected line
            # even if it's partially selected
            cursor.setPosition(start)
            cursor.select(QtGui.QTextCursor.LineUnderCursor)
            text = cursor.selectedText()
            cursor.insertText(re.sub("^([ ]{1,4})", "", text))

    def run_completer(self, event):
        """Check if completer needed and run it or hide"""
        ctrl_or_shift = event.modifiers() & (QtCore.Qt.ControlModifier | QtCore.Qt.ShiftModifier)
        if not self.completer or (ctrl_or_shift and len(event.text()) == 0):
            return

        eow = "~!@#$%^&*()_+{}|:\"<>?,/;'[]\\-="
        has_modifier = (event.modifiers() != QtCore.Qt.NoModifier) and not ctrl_or_shift
        prefix = self.text_under_cursor()

        # hide - if conditions not in the proper state
        if (
            has_modifier
            or len(event.text()) == 0
            or eow.rfind(event.text()[-1]) != -1
            or not prefix
        ):
            self.completer.popup().hide()

        # dot case - show completer with object attributes
        elif prefix != self.completer.completionPrefix() and len(prefix) > 0 and "." in prefix:
            self.completer.setCompletionPrefix(prefix)

            found_unit_descr = re.search("(.*)\.", prefix)

            if found_unit_descr:
                unit_descr = found_unit_descr.groups()[0]
            else:
                return

            # try to get object from locals, if can't - hide completer
            if unit_descr in self.locals:
                module = self.locals[unit_descr]

                module_attr_list = [
                    unit_descr + "." + attr for attr in dir(module) if not attr.startswith("_")
                ]
                self.keyword_completer_model.setStringList(module_attr_list)
                self.show_completer(prefix)
            else:
                pass

        # keyword case - show completer if the word longer then 3 symbols
        elif prefix != self.completer.completionPrefix() and len(prefix) >= 3:
            self.make_words_set(prefix)
            self.keyword_completer_model.setStringList(self.all_words)
            self.show_completer(prefix)

    def show_completer(self, prefix):
        self.completer.setCompletionPrefix(prefix)
        self.completer.popup().setCurrentIndex(self.completer.completionModel().index(0, 0))

        cr = self.cursorRect()
        cr.setWidth(
            self.completer.popup().sizeHintForColumn(0)
            + self.completer.popup().verticalScrollBar().sizeHint().width()
        )
        self.completer.complete(cr)

    def insert_completion(self, completion):
        if self.completer.widget() != self:
            return

        cursor = self.textCursor()
        len_pref = len(self.completer.completionPrefix())
        cursor.movePosition(QtGui.QTextCursor.Left)
        cursor.movePosition(QtGui.QTextCursor.EndOfWord)
        cursor.insertText(completion[len_pref:])
        self.completer.popup().hide()

    def focusInEvent(self, event):
        """Override method"""
        if self.completer:
            self.completer.setWidget(self)
        QtWidgets.QTextEdit.focusInEvent(self, event)

    def text_under_cursor(self):
        """Return words with numbers and symbols :._"""
        cursor = self.textCursor()
        pos = cursor.position()
        text = self.toPlainText()

        ex = QtCore.QRegExp("[^[\.\w+]")
        non_word_index = ex.lastIndexIn(text[:pos])

        return text[non_word_index + 1 : pos]

    def insertFromMimeData(self, source):
        text = source.text().replace("\t", self.TAB_SEQ)
        self.insertPlainText(text)

    def make_words_set(self, prefix):
        """Make set of all words longer than 3 symbols"""
        # TODO maybe rewrite this first solution to use ast
        text = self.toPlainText()
        words = [word for word in re.findall(r"\b\w+\b", text) if len(word) > 3 and word != prefix]

        self.all_words = self.keywords.union(words)

    def get_imports(self):
        """We search for all import statements before cursor,
        make a map of them and import them if it's possible"""
        cursor = self.textCursor()

        # we will search for import statements before cursor
        text = self.toPlainText()[: cursor.position() - 1]
        self.imported_names = {}

        try:
            syntax_tree = ast.parse(text)
        except SyntaxError:
            # if it's syntax error than probably user did something wrong
            # or just didn't finished writing his code
            # and we don't have to autocomplete such code
            return
        except Exception as e:
            print(e)

        for node in ast.walk(syntax_tree):
            if isinstance(node, ast.Import) or isinstance(node, ast.ImportFrom):
                for alias in node.names:
                    if alias.asname is None:
                        asname = alias.name
                    else:
                        asname = alias.asname
                    self.imported_names[asname] = alias.name

                    # if module, or it's attributes was not imported,
                    # import them for further getting their attributes
                    if asname not in globals():
                        new_tree = ast.Module(body=[node])
                        ast.fix_missing_locations(new_tree)
                        try:
                            exec(compile(new_tree, "<ast>", "exec"), globals())
                        except ImportError:
                            # I don't want to see such errors, but others may be
                            pass
                        except Exception as e:
                            print(e)
