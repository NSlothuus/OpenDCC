// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/base/ipc_commands_api/command.h"

#include "opendcc/base/utils/string_utils.h"

OPENDCC_NAMESPACE_OPEN

IPC_NAMESPACE_OPEN

static const auto arg_splitter = ' ';
static const auto key_value_splitter = '=';

std::string Command::to_string() const
{
    std::string result;

    result += "name=" + name;

    for (const auto& arg : args)
    {
        result += arg_splitter + arg.first + key_value_splitter + arg.second;
    }

    return result;
}

/* static */
Command Command::from_string(const std::string& string)
{
    Command result;

    const auto splits = OPENDCC_NAMESPACE::split(string, arg_splitter);

    for (const auto& split : splits)
    {
        const auto args_split = OPENDCC_NAMESPACE::split(split, key_value_splitter);

        if (args_split.size() != 2)
        {
            continue;
        }

        const auto& key = args_split[0];
        const auto& value = args_split[1];

        if (key == "name")
        {
            result.name = value;
        }
        else
        {
            result.args[key] = value;
        }
    }

    return result;
}

IPC_NAMESPACE_CLOSE

OPENDCC_NAMESPACE_CLOSE
