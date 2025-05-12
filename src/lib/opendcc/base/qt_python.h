/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

// issue with python3 and Qt5 moc slots keyword
// workaround from https://stackoverflow.com/a/49359288
#pragma push_macro("slots")
#undef slots
#include "Python.h"
#pragma pop_macro("slots")
