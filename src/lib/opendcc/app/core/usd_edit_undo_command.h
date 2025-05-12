/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/commands_api/core/command.h"

#include <memory>

OPENDCC_NAMESPACE_OPEN

namespace commands
{
    class UndoInverse;
}

class UsdEditUndoCommand : public UndoCommand
{
public:
    virtual ~UsdEditUndoCommand() = default;

    virtual void redo() override;
    virtual void undo() override;
    virtual CommandResult execute(const CommandArgs& args) override;
    void set_state(std::shared_ptr<commands::UndoInverse> inverse);

private:
    std::shared_ptr<commands::UndoInverse> m_inverse;
};

OPENDCC_NAMESPACE_CLOSE
