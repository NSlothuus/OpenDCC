/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <unordered_map>
#include <string>
#include <functional>
#include "pxr/usd/usd/tokens.h"
#include "pxr/usd/sdf/valueTypeName.h"
#include "iexpression.h"

OPENDCC_NAMESPACE_OPEN

class ExpressionFactory
{
public:
    using ExtensionCreator = std::function<IExpressionPtr(std::string)>;
    static ExpressionFactory& instance()
    {
        static ExpressionFactory factory;
        return factory;
    }

    IExpressionPtr create_expression(PXR_NS::TfToken expression_type, PXR_NS::SdfValueTypeName result_type, const std::string& expression);
    void register_creator(PXR_NS::TfToken expression_type, PXR_NS::SdfValueTypeName result_type, const ExtensionCreator& expression);

private:
    struct SdfValueTypeNameHashFunctor
    {
        size_t operator()(PXR_NS::SdfValueTypeName const& token) const { return token.GetHash(); }
    };

    using ResultTypeToExpression = std::unordered_map<PXR_NS::SdfValueTypeName, ExtensionCreator, SdfValueTypeNameHashFunctor>;
    std::unordered_map<PXR_NS::TfToken, ResultTypeToExpression, PXR_NS::TfToken::HashFunctor> m_creators;
};

OPENDCC_NAMESPACE_CLOSE
