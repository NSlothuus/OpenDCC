// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "selection_event_dispatcher.h"

OPENDCC_NAMESPACE_OPEN

SelectionEventDispatcher& global_selection_dispatcher()
{
    static SelectionEventDispatcher dispatcher;
    return dispatcher;
}

OPENDCC_NAMESPACE_CLOSE
