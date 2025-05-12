/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/base/commands_api/core/command.h"
#include "opendcc/app/core/undo/inverse.h"
#include "opendcc/app/core/selection_list.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/usd/prim.h>

OPENDCC_NAMESPACE_OPEN

namespace commands
{
    class CreateCameraFromViewCommand : public UndoCommand
    {
    public:
        virtual CommandResult execute(const CommandArgs& args) override;

        virtual void undo() override;
        virtual void redo() override;

        static std::string cmd_name;
        static CommandSyntax cmd_syntax();
        static std::shared_ptr<Command> create_cmd();

    private:
        void do_cmd();

        std::unique_ptr<UndoInverse> m_inverse;
    };
}

OPENDCC_NAMESPACE_CLOSE
