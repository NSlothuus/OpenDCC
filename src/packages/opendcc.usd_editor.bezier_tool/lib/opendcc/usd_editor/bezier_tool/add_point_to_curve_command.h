/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/commands_api/core/command.h"
#include "opendcc/usd_editor/bezier_tool/bezier_curve.h"
#include "opendcc/usd_editor/bezier_tool/bezier_tool_context.h"

#include <pxr/base/gf/vec3f.h>

#include <memory>

OPENDCC_NAMESPACE_OPEN

class AddPointToCurveCommand
    : public UndoCommand
    , public ToolCommand
{
public:
    AddPointToCurveCommand(BezierCurvePtr curve, BezierToolContext* context, const BezierCurve::Point& point);

    CommandResult execute(const CommandArgs& args) override;

    void undo() override;
    void redo() override;

    bool merge_with(UndoCommand* command) override;

    CommandArgs make_args() const override;

private:
    BezierCurvePtr m_curve;
    BezierCurve::Point m_point;
    BezierToolContext* m_context = nullptr;
};

OPENDCC_NAMESPACE_CLOSE
