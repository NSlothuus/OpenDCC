/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <memory>

OPENDCC_NAMESPACE_OPEN
namespace commands
{
    class UndoInverse;

    class OPENDCC_API UsdEditsUndoBlock
    {
    public:
        UsdEditsUndoBlock();
        ~UsdEditsUndoBlock();
    };

    class OPENDCC_API UsdEditsBlock
    {
    public:
        UsdEditsBlock();
        std::unique_ptr<UndoInverse> take_edits();
        ~UsdEditsBlock();
    };
} // namespace commands
OPENDCC_NAMESPACE_CLOSE
