/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"

#include <memory>
#include <string>

#include <pxr/usd/usd/tokens.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/base/vt/value.h>

OPENDCC_NAMESPACE_OPEN

struct ExpressionContext
{
    double frame;
    PXR_NS::SdfPath attribute_path;
    std::unordered_map<std::string, PXR_NS::VtValue> variables;
};

class IExpression
{
public:
    virtual ~IExpression() {}
    virtual PXR_NS::VtValue evaluate(const ExpressionContext& context, bool& success) = 0;
};

using IExpressionPtr = std::shared_ptr<IExpression>;

OPENDCC_NAMESPACE_CLOSE
