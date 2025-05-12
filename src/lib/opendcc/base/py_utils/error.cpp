// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/base/py_utils/error.h"
#include <boost/python.hpp>
#include "opendcc/base/logging/logger.h"

OPENDCC_NAMESPACE_OPEN

std::string py_get_error_str()
{
    using namespace boost::python;
    try
    {
        PyObject* type = nullptr;
        PyObject* value = nullptr;
        PyObject* trace = nullptr;
        PyErr_Fetch(&type, &value, &trace);
        PyErr_NormalizeException(&type, &value, &trace);

        auto tb_mod = object(handle<>(PyImport_ImportModule("traceback")));
        auto exc = tb_mod.attr("format_exception")(handle<>(allow_null(type)), handle<>(allow_null(value)), handle<>(allow_null(trace)));

        std::string exception_string;
        const auto size = len(exc);
        for (ssize_t i = 0; i < size; ++i)
        {
            exception_string += extract<std::string>(exc[i]);
        }

        return exception_string;
    }
    catch (const error_already_set&)
    {
        PyErr_Print();
        return std::string();
    }
}

OPENDCC_NAMESPACE_CLOSE
