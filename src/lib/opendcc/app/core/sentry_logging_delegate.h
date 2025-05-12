/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/base/logging/logging_delegate.h"

OPENDCC_NAMESPACE_OPEN

class SentryLoggingDelegate : public LoggingDelegate
{
public:
    SentryLoggingDelegate();
    ~SentryLoggingDelegate() override;
    void log(const MessageContext& context, const std::string& message) override;
};

OPENDCC_NAMESPACE_CLOSE
