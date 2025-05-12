// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/base/logging/logger.h"

#include "expression_factory.h"
#include "engine.h"

PXR_NAMESPACE_USING_DIRECTIVE

OPENDCC_NAMESPACE_OPEN

IExpressionPtr ExpressionFactory::create_expression(TfToken expression_type, SdfValueTypeName result_type, const std::string& expression)
{
    auto type_it = m_creators.find(expression_type);
    if (type_it == m_creators.end())
    {
        OPENDCC_ERROR("Unsupported expression type: {}", expression_type.GetText());
        return nullptr;
    }
    auto result_type_it = type_it->second.find(result_type);
    if (result_type_it == type_it->second.end())
    {
        OPENDCC_ERROR("Unsupported result type: {} for  expression type {}", result_type.GetAsToken().GetText(), expression_type.GetText());
        return nullptr;
    }

    return result_type_it->second(expression);
}

void ExpressionFactory::register_creator(TfToken expression_type, SdfValueTypeName result_type, const ExtensionCreator& expression)
{
    m_creators[expression_type][result_type] = expression;
}

OPENDCC_NAMESPACE_CLOSE
