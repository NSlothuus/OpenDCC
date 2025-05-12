// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "expression_factory.h"
#include "opendcc/app/core/application.h"

#include "pxr/usd/sdf/valueTypeName.h"
#include "pxr/usd/sdf/types.h"

#include <sstream>
#include <string>
#include <regex>

#include "session.h"

PXR_NAMESPACE_USING_DIRECTIVE

OPENDCC_NAMESPACE_OPEN

std::string template_replace(const std::string& source, std::function<bool(const std::string& variable, std::string& resolved)> resolve_callback)
{
    using namespace std;

    const regex re("\\$\\{?([_\\w]+)\\}?", regex_constants::ECMAScript | regex_constants::icase);

    std::string result;

    sregex_iterator it(source.begin(), source.end(), re);

    if (it == sregex_iterator())
    {
        return source;
    }
    else
    {
        for (sregex_iterator end, prev_it(it);;)
        {
            if (it->prefix().length())
            {
                result += it->prefix().str();
            }

            std::string resolved;

            if (resolve_callback(it->str(1), resolved))
            {
                result += resolved;
            }
            else
            {
                result += it->str(0);
            }

            prev_it = it;
            ++it;

            if (it == end)
            {
                result += prev_it->suffix().str();

                break;
            }
        }
    }

    return result;
}

class ExpandVarsExpressionBase : public IExpression
{
public:
    ExpandVarsExpressionBase(const std::string& expression_str)
        : m_expression_str(expression_str)
    {
    }
    virtual std::string evaluate_string(const ExpressionContext& context, bool& success)
    {
        success = true;
        return template_replace(m_expression_str, [context](const std::string& key, std::string& value) {
            return ExpressionSession::instance().evaluate_string(context, key, value);
        });
    }

private:
    std::string m_expression_str;
};

class ExpandVarsExpressionAsset : public ExpandVarsExpressionBase
{
public:
    ExpandVarsExpressionAsset(const std::string& expression_str)
        : ExpandVarsExpressionBase(expression_str)
    {
    }
    virtual VtValue evaluate(const ExpressionContext& context, bool& success) override
    {
        return VtValue(SdfAssetPath(evaluate_string(context, success)));
    }
};

class ExpandVarsExpressionString : public ExpandVarsExpressionBase
{
public:
    ExpandVarsExpressionString(const std::string& expression_str)
        : ExpandVarsExpressionBase(expression_str)
    {
    }
    virtual VtValue evaluate(const ExpressionContext& context, bool& success) override { return VtValue(evaluate_string(context, success)); }
};

class ExpandVarsExpressionToken : public ExpandVarsExpressionBase
{
public:
    ExpandVarsExpressionToken(const std::string& expression_str)
        : ExpandVarsExpressionBase(expression_str)
    {
    }
    virtual VtValue evaluate(const ExpressionContext& context, bool& success) override { return VtValue(TfToken(evaluate_string(context, success))); }
};

bool register_default_expressions()
{
    ExpressionFactory::instance().register_creator(
        TfToken("expand_vars"), SdfValueTypeNames->String,
        [](std::string expression_str) -> IExpressionPtr { return std::make_shared<ExpandVarsExpressionString>(expression_str); });

    ExpressionFactory::instance().register_creator(
        TfToken("expand_vars"), SdfValueTypeNames->Asset,
        [](std::string expression_str) -> IExpressionPtr { return std::make_shared<ExpandVarsExpressionAsset>(expression_str); });

    ExpressionFactory::instance().register_creator(
        TfToken("expand_vars"), SdfValueTypeNames->Token,
        [](std::string expression_str) -> IExpressionPtr { return std::make_shared<ExpandVarsExpressionToken>(expression_str); });

    return true;
}

static bool tmp = register_default_expressions();

OPENDCC_NAMESPACE_CLOSE
