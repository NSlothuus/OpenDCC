/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ipc - inter-process communication
#define IPC_NAMESPACE ipc
#define IPC_NAMESPACE_OPEN  \
    namespace IPC_NAMESPACE \
    {
#define IPC_NAMESPACE_CLOSE }
#define IPC_NAMESPACE_USING using namespace IPC_NAMESPACE;
