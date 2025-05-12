// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/base/ipc_commands_api/command_registry.h"

OPENDCC_NAMESPACE_OPEN

IPC_NAMESPACE_OPEN

/* static */
CommandRegistry& CommandRegistry::instance()
{
    static CommandRegistry s_instance;
    return s_instance;
}

void CommandRegistry::add_handler(const std::string& name, const CommandHandler& handler)
{
    m_handlers[name] = handler;
}

void CommandRegistry::handle_command(const Command& command)
{
    const auto find = m_handlers.find(command.name);
    if (find == m_handlers.end())
    {
        return;
    }
    find->second(command);
}

CommandRegistry::CommandRegistry() {}

CommandRegistry::~CommandRegistry() {}

IPC_NAMESPACE_CLOSE

OPENDCC_NAMESPACE_CLOSE
