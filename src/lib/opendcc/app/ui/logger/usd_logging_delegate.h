/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/logging/logger.h"

#include <pxr/pxr.h>
#include <pxr/usd/usd/stage.h>

OPENDCC_NAMESPACE_OPEN

class UsdLoggingDelegate : public PXR_NS::TfDiagnosticMgr::Delegate
{
public:
    virtual void IssueError(const PXR_NS::TfError& err) override;
    virtual void IssueFatalError(const PXR_NS::TfCallContext& context, const std::string& msg) override;
    virtual void IssueStatus(const PXR_NS::TfStatus& status) override;
    virtual void IssueWarning(const PXR_NS::TfWarning& warning) override;
};

OPENDCC_NAMESPACE_CLOSE
