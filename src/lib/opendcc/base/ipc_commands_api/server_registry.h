/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/ipc_commands_api/api.h"
#include "opendcc/base/ipc_commands_api/ipc.h"
#include "opendcc/base/ipc_commands_api/server_info.h"

#include <mutex>
#include <thread>
#include <string>
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN

IPC_NAMESPACE_OPEN

/**
 * @brief A class that manages the ipc server registry.
 */
class IPC_COMMANDS_API ServerRegistry
{
public:
    ServerRegistry(const ServerRegistry&) = delete;
    ServerRegistry(ServerRegistry&&) = delete;
    ServerRegistry& operator=(const ServerRegistry&) = delete;
    ServerRegistry& operator=(ServerRegistry&&) = delete;

    /**
     * @brief Returns the singleton instance of the ServerRegistry.
     * @return The instance of the ServerRegistry.
     */
    static ServerRegistry& instance();

    /**
     * @brief Adds a server to the registry.
     * @param pid The process ID of the server.
     * @param info The ServerInfo containing information about the server.
     */
    void add_server(const std::string& pid, const ServerInfo& info);

    /**
     * @brief Finds a server in the registry.
     * @param pid The process ID of the server to find.
     * @return The ServerInfo containing information about the server,
     * or an empty ServerInfo if the server is not found.
     */
    ServerInfo find_server(const std::string& pid);

private:
    ServerRegistry();
    ~ServerRegistry();

    using ServersMap = std::unordered_map<std::string /* pid */, ServerInfo>;
    ServersMap m_servers;
};

IPC_NAMESPACE_CLOSE

OPENDCC_NAMESPACE_CLOSE
