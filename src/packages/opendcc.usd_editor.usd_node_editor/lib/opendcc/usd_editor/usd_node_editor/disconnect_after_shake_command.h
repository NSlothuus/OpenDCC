/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/base/commands_api/core/command.h"

OPENDCC_NAMESPACE_OPEN

namespace commands
{
    class UndoInverse;
    class UsdEditsBlock;
}

class DisconnectAfterShakeCommand
    : public UndoCommand
    , public ToolCommand
{
public:
    DisconnectAfterShakeCommand() = default;
    virtual ~DisconnectAfterShakeCommand() override = default;

    void undo() override;
    void redo() override;

    void start_block();
    void end_block();

    CommandArgs make_args() const override;
    CommandResult execute(const CommandArgs& args) override;

private:
    std::unique_ptr<commands::UndoInverse> m_inverse;
    std::unique_ptr<commands::UsdEditsBlock> m_change_block;
};

OPENDCC_NAMESPACE_CLOSE
