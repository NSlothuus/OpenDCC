# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets, QtCore, QtGui
import opendcc.core as dcc_core

from opendcc.i18n import i18n

# Moved out of ToolContextActions due to a translate-parser bug
tool_select = i18n("toolbars.tools", "Select Tool")
tool_move = i18n("toolbars.tools", "Move Tool")
tool_rotate = i18n("toolbars.tools", "Rotate Tool")
tool_scale = i18n("toolbars.tools", "Scale Tool")


class Wrapper:
    def __init__(self, cls):
        self.wrapped = cls
        self.instance = None

    def __call__(self, *args, **kwargs):
        if self.instance is None:
            self.instance = self.wrapped(*args, **kwargs)
        return self.instance


def singleton(cls):
    return Wrapper(cls)


def set_tool_context(tool_name):
    current_context = dcc_core.Application.instance().get_current_viewport_tool()
    if not current_context or current_context.get_name() != tool_name:
        tool_context = dcc_core.ViewportToolContextRegistry.create_tool_context("USD", tool_name)
        dcc_core.Application.instance().set_current_viewport_tool(tool_context)


@singleton
class ToolContextActions:
    def __init__(self):
        self._action_groups = {}
        self._last_actions = {}
        self.register_action_group("USD")
        self._cur_ctx = "USD"
        self._last_action = None

        # register basic tools: select, move, rotate, scale
        self.register_action("USD", "select_tool", tool_select, ":icons/select_tool.png", "Q")
        self.register_action("USD", "move_tool", tool_move, ":icons/move_tool.png", "W")
        self.register_action("USD", "rotate_tool", tool_rotate, ":icons/rotate_tool.png", "E")
        self.register_action("USD", "scale_tool", tool_scale, ":icons/scale_tool.png", "R")

    def register_action_group(self, ctx_name):
        app = dcc_core.Application.instance()
        mw = app.get_main_window()
        action_group = QtWidgets.QActionGroup(mw)
        action_group.setExclusive(True)
        self._action_groups[ctx_name] = action_group

    def get_action_group(self, ctx_name):
        return self._action_groups.get(ctx_name, None)

    def get_actions(self, ctx_name=None):
        if not ctx_name:
            result = []
            for action_group in self._action_groups.values():
                result += action_group.actions()
            return result

        action_group = self._action_groups.get(ctx_name, None)
        if not action_group:
            return None
        return action_group.actions()

    def get_action(self, ctx_name, tool_ctx_name):
        action_group = self._action_groups.get(ctx_name, None)
        if not action_group:
            return None

        for action in action_group.actions():
            if action.data() == tool_ctx_name:
                return action

        return None

    def save_last_action(self, ctx_name):
        action_group = self._action_groups.get(ctx_name, None)
        if not action_group:
            return

        self._last_actions[ctx_name] = action_group.checkedAction()

    def switch_context(self, ctx_name):
        if ctx_name == self._cur_ctx:
            return

        old_ctx = self._cur_ctx
        self._cur_ctx = ctx_name

        if self._last_action:
            self._last_action.trigger()
        else:
            self._action_groups[ctx_name].actions()[0].trigger()
        self._last_action = self._action_groups[old_ctx].checkedAction()

    def register_action(self, ctx_name, tool_ctx_name, label, icon, shortcut):
        action = QtWidgets.QAction(label)
        action.setShortcutContext(QtCore.Qt.ApplicationShortcut)
        action.setIcon(QtGui.QIcon(icon))
        action.setShortcut(QtGui.QKeySequence(shortcut))
        action.setCheckable(True)
        action.setData(tool_ctx_name)
        action.triggered.connect(lambda checked: self._change_tool_context(ctx_name, tool_ctx_name))
        self._action_groups[ctx_name].addAction(action)

    def _change_tool_context(self, ctx_name, tool_ctx_name):
        current_context = dcc_core.Application.instance().get_current_viewport_tool()
        if current_context and current_context.get_name() == tool_ctx_name:
            return

        tool_context = dcc_core.ViewportToolContextRegistry.create_tool_context(
            ctx_name, tool_ctx_name
        )
        dcc_core.Application.instance().set_current_viewport_tool(tool_context)
