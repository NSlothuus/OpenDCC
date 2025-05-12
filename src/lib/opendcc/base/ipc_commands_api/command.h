/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/ipc_commands_api/api.h"
#include "opendcc/base/ipc_commands_api/ipc.h"

#include <string>
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN

IPC_NAMESPACE_OPEN

/**
 * @struct Command
 * @brief Structure which represents an ipc command.
 */
struct IPC_COMMANDS_API Command
{
    /**
     * @brief The name of the command.
     */
    std::string name;

    /**
     * @brief Arguments of the command.
     */
    std::unordered_map<std::string /* key */, std::string /* value */> args;

    /**
     * @brief Converts the Command to its string representation.
     * @return The string representation of the Command.
     */
    std::string to_string() const;

    /**
     * @brief Creates a Command from its string representation.
     * @param string The string representation of the Command.
     * @return The Command.
     */
    static Command from_string(const std::string& string);
};

IPC_NAMESPACE_CLOSE

OPENDCC_NAMESPACE_CLOSE
