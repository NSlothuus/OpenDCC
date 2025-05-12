# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets


def i18n(context, string, disambiguation=None, n=-1):
    return QtWidgets.QApplication.translate(context, string, disambiguation, n)
