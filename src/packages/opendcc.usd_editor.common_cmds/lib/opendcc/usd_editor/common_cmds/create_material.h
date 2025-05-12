/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/base/commands_api/core/command.h"

#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/stage.h>

OPENDCC_NAMESPACE_OPEN

namespace commands
{
    class CreateMaterialCommand : public UndoCommand
    {
    public:
        virtual CommandResult execute(const CommandArgs& args) override;
        virtual void undo() override;
        virtual void redo() override;

        static std::string cmd_name;
        static CommandSyntax cmd_syntax();
        static std::shared_ptr<Command> create_cmd();

    private:
        CommandResult do_cmd();

        bool m_need_create_scope = false;
        PXR_NS::UsdStageWeakPtr m_stage;

        CommandArgs m_create_scope_args;
        CommandArgs m_create_mat_args;
        CommandArgs m_remove_args;

        PXR_NS::SdfPath m_material_path;
        PXR_NS::SdfPath m_scope_path;
    };
}

OPENDCC_NAMESPACE_CLOSE
