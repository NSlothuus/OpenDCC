// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

// @snippet create_graphics_item_in_python
struct AutoRefCallable
{
    PyObject* m_obj = nullptr;
    AutoRefCallable(PyObject* obj)
        : m_obj(obj)
    {
        Shiboken::GilState state;
        Py_INCREF(m_obj);
    }
    AutoRefCallable(const AutoRefCallable& callable)
    {
        m_obj = callable.m_obj;
        Py_INCREF(m_obj);
    }
    ~AutoRefCallable()
    {
        Shiboken::GilState state;
        Py_DECREF(m_obj);
    }
    QGraphicsItem* operator()()
    {
        Shiboken::GilState state;
        PyObject* ret = PyObject_CallObject(m_obj, nullptr);
        if (PyErr_Occurred())
        {
            PyErr_Print();
            return nullptr;
        }
        auto cpp_result = % CONVERTTOCPP[QGraphicsItem * ](ret);
        return cpp_result;
    }
};

auto callable = % PYARG_3;
AutoRefCallable cppCallback(callable);
% RETURN_TYPE % 0 = % CPPSELF.% FUNCTION_NAME(% 1, % 2, cppCallback);
% PYARG_0 = % CONVERTTOPYTHON[% RETURN_TYPE](cppResult);
// @snippet create_graphics_item_in_python

// @snippet set_ctx_menu_handler
struct AutoRefCallable
{
    PyObject* m_obj = nullptr;
    AutoRefCallable(PyObject* obj)
        : m_obj(obj)
    {
        Shiboken::GilState state;
        Py_INCREF(m_obj);
    }
    AutoRefCallable(const AutoRefCallable& callable)
    {
        m_obj = callable.m_obj;
        Py_INCREF(m_obj);
    }
    ~AutoRefCallable()
    {
        Shiboken::GilState state;
        Py_DECREF(m_obj);
    }
    void operator()(QContextMenuEvent* arg)
    {
        Shiboken::GilState state;
        Shiboken::AutoDecRef arglist(PyTuple_New(1));
        PyTuple_SET_ITEM(arglist, 0, % CONVERTTOPYTHON[QContextMenuEvent * ](arg));

        PyObject_CallObject(m_obj, arglist);
        if (PyErr_Occurred())
            PyErr_Print();
    }
};

auto callable = % PYARG_1;
AutoRefCallable cppCallback(callable);
% CPPSELF.% FUNCTION_NAME(cppCallback);
// @snippet set_ctx_menu_handler
