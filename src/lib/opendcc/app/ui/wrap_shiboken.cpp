// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/ui/wrap_shiboken.h"
#include "opendcc/base/logging/logger.h"

#include <shiboken.h>
#include <pyside2_qtcore_python.h>
#include <pyside2_qtgui_python.h>
#include <pyside2_qtwidgets_python.h>

PyTypeObject **SbkPySide2_QtCoreTypes = NULL;
PyTypeObject **SbkPySide2_QtGuiTypes = NULL;
PyTypeObject **SbkPySide2_QtWidgetsTypes = NULL;

OPENDCC_NAMESPACE_OPEN

// inspired by https://github.com/cryos/avogadro/blob/master/libavogadro/src/python/sip.cpp
namespace py_interp
{

    namespace bind
    {
        bool wrap_shiboken()
        {
            Shiboken::AutoDecRef core_module(Shiboken::Module::import("PySide2.QtCore"));
            if (core_module.isNull())
                return false;

            SbkPySide2_QtCoreTypes = Shiboken::Module::getTypes(core_module);

            Shiboken::AutoDecRef gui_module(Shiboken::Module::import("PySide2.QtGui"));
            if (gui_module.isNull())
                return false;
            SbkPySide2_QtGuiTypes = Shiboken::Module::getTypes(gui_module);

            Shiboken::AutoDecRef widgets_module(Shiboken::Module::import("PySide2.QtWidgets"));
            if (widgets_module.isNull())
                return false;
            SbkPySide2_QtWidgetsTypes = Shiboken::Module::getTypes(widgets_module);

            return true;
        }

    }
}
OPENDCC_NAMESPACE_CLOSE
