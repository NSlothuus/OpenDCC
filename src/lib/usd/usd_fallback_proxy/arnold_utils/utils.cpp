// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <usd_fallback_proxy/arnold_utils/utils.h>

#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/attributeSpec.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

static const std::unordered_map<TfToken, unsigned, TfToken::HashFunctor> USD_ARNOLD_TYPE_TO_ARNOLD_NODE_TYPE = {
    { TfToken("SceneLibArnoldDriver"), AI_NODE_DRIVER },
    { TfToken("SceneLibArnoldFilter"), AI_NODE_FILTER }
};

static const std::unordered_map<std::string, const SdfValueTypeName&> ARNOLD_TYPE_TO_USD_TYPE = { { "STRING", SdfValueTypeNames->String },
                                                                                                  { "INT", SdfValueTypeNames->Int },
                                                                                                  { "FLOAT", SdfValueTypeNames->Float },
                                                                                                  { "BOOL", SdfValueTypeNames->Bool },
                                                                                                  { "ENUM", SdfValueTypeNames->Token } };

class SdfValueTypeNameWrapper
{
public:
    SdfValueTypeNameWrapper(const char* type_name)
    {
        auto type_iter = ARNOLD_TYPE_TO_USD_TYPE.find(type_name);
        if (type_iter != ARNOLD_TYPE_TO_USD_TYPE.end())
        {
            m_type_name = &type_iter->second;
        }
        else
        {
            m_type_name = &SdfValueTypeNames->String;
        }
    }

    operator SdfValueTypeName() const { return *m_type_name; }

    const SdfValueTypeName& array() const
    {
        static const std::unordered_map<SdfValueTypeName, SdfValueTypeName, SdfValueTypeNameHash> type_names = {
            { SdfValueTypeNames->String, SdfValueTypeNames->StringArray },
            { SdfValueTypeNames->Bool, SdfValueTypeNames->BoolArray },
            { SdfValueTypeNames->Float, SdfValueTypeNames->FloatArray },
            { SdfValueTypeNames->Int, SdfValueTypeNames->IntArray }
        };

        return type_names.at(*m_type_name);
    }

private:
    const SdfValueTypeName* m_type_name;
};

template <typename TRet>
TRet get_from_arnold_array(const AtParamValue* value, int index)
{
    assert(false && "Unknown type.");
}

template <>
int get_from_arnold_array(const AtParamValue* value, int index)
{
    return AiArrayGetInt(value->ARRAY(), index);
}

template <>
float get_from_arnold_array(const AtParamValue* value, int index)
{
    return AiArrayGetFlt(value->ARRAY(), index);
}

template <>
std::string get_from_arnold_array(const AtParamValue* value, int index)
{
    return AiArrayGetStr(value->ARRAY(), index).c_str();
}

template <>
bool get_from_arnold_array(const AtParamValue* value, int index)
{
    return AiArrayGetBool(value->ARRAY(), index);
}

template <class TCollection, class TRet>
VtValue fill_array(const AtParamValue* value)
{
    TCollection result;
    result.resize(AiArrayGetNumElements(value->ARRAY()));
    for (int i = 0; i < result.size(); i++)
    {
        result[i] = get_from_arnold_array<TRet>(value, i);
    }
    return VtValue(result);
}

unsigned get_arnold_node_type(const TfToken& prim_type)
{
    auto iter = USD_ARNOLD_TYPE_TO_ARNOLD_NODE_TYPE.find(prim_type);
    if (iter == USD_ARNOLD_TYPE_TO_ARNOLD_NODE_TYPE.end())
        return -1;

    return iter->second;
}

VtTokenArray get_allowed_tokens(const AtParamEntry* param)
{
    VtTokenArray tokens;
    auto arnold_enum = AiParamGetEnum(param);
    for (int i = 0; true; i++)
    {
        auto token = arnold_enum[i];
        if (token)
            tokens.push_back(TfToken(token));
        else
            break;
    }

    return tokens;
}

VtValue get_value_for_arnold_param(const AtParamEntry* param)
{
    auto value = AiParamGetDefault(param);
    switch (AiParamGetType(param))
    {
    case AI_TYPE_INT:
        return VtValue(value->INT());
    case AI_TYPE_BOOLEAN:
        return VtValue(value->BOOL());
    case AI_TYPE_FLOAT:
        return VtValue(value->FLT());
    case AI_TYPE_STRING:
        return VtValue(std::string(value->STR().c_str()));
    case AI_TYPE_ENUM:
        return VtValue(std::string(AiEnumGetString(AiParamGetEnum(param), value->INT())));
    case AI_TYPE_ARRAY:
        switch (AiArrayGetType(value->ARRAY()))
        {
        case AI_TYPE_INT:
            return fill_array<VtIntArray, int>(value);
        case AI_TYPE_FLOAT:
            return fill_array<VtFloatArray, float>(value);
        case AI_TYPE_BOOLEAN:
            return fill_array<VtBoolArray, bool>(value);
        case AI_TYPE_STRING:
            return fill_array<VtStringArray, std::string>(value);
        }
    }

    return VtValue();
}

VtTokenArray get_nodes_by_type(unsigned node_type)
{
    if (node_type == -1)
        return {};

    VtTokenArray result;
    static std::unordered_map<unsigned, std::unordered_set<std::string>> node_entry_maps;
    auto& node_entry_map = node_entry_maps[node_type];
    if (!node_entry_map.empty())
    {
        for (auto item : node_entry_map)
            result.push_back(TfToken(item));
        return result;
    }

#if AI_VERSION_ARCH_NUM >= 7 && AI_VERSION_MAJOR_NUM >= 2
    auto status = AiRenderGetStatus(AiRenderSession(nullptr));
#else
    auto status = AiRenderGetStatus();
#endif
    if (status == AI_RENDER_STATUS_NOT_STARTED)
        AiBegin();

    auto iter = AiUniverseGetNodeEntryIterator(node_type);
    while (!AiNodeEntryIteratorFinished(iter))
    {
        auto node_entry = AiNodeEntryIteratorGetNext(iter);
        auto node_entry_name = AiNodeEntryGetName(node_entry);
        node_entry_map.emplace(node_entry_name);
        result.push_back(TfToken(node_entry_name));
    }

    if (status == AI_RENDER_STATUS_NOT_STARTED)
        AiEnd();
    return result;
}

SdfLayerRefPtr get_arnold_entry_map(unsigned node_type, const std::string& node_entry_type, const std::string& attr_namespace)
{
    if (node_type == -1)
        return nullptr;

    static std::unordered_map<unsigned, std::unordered_map<std::string, SdfLayerRefPtr>> node_entry_maps;
    auto& node_entry_map = node_entry_maps[node_type];

    auto node_entry_iter = node_entry_map.find(node_entry_type);
    if (node_entry_iter != node_entry_map.end())
    {
        return node_entry_iter->second;
    }

#if AI_VERSION_ARCH_NUM >= 7 && AI_VERSION_MAJOR_NUM >= 2
    auto status = AiRenderGetStatus(AiRenderSession(nullptr));
#else
    auto status = AiRenderGetStatus();
#endif
    if (status == AI_RENDER_STATUS_NOT_STARTED)
        AiBegin();

    auto iter = AiUniverseGetNodeEntryIterator(node_type);
    while (!AiNodeEntryIteratorFinished(iter))
    {
        auto node_entry = AiNodeEntryIteratorGetNext(iter);
        auto node_entry_name = AiNodeEntryGetName(node_entry);
        node_entry_iter = node_entry_map.find(node_entry_name);
        if (node_entry_iter != node_entry_map.end())
        {
            continue;
        }

        auto node_params = AiNodeEntryGetParamIterator(node_entry);

        auto layer = SdfLayer::CreateAnonymous("temp_layer");
        auto spec = SdfCreatePrimInLayer(layer, SdfPath("/temp_prim"));
        spec->SetSpecifier(SdfSpecifier::SdfSpecifierDef);
        while (!AiParamIteratorFinished(node_params))
        {
            auto param = AiParamIteratorGetNext(node_params);
            auto arnold_type = AiParamGetType(param);
            auto arnold_type_name = AiParamGetTypeName(arnold_type);
            auto arnold_default_value = AiParamGetDefault(param);

            SdfValueTypeName usd_type;
            if (!strcmp(arnold_type_name, "ARRAY"))
            {
                auto array_value_type_name = AiParamGetTypeName(AiArrayGetType(arnold_default_value->ARRAY()));
                usd_type = SdfValueTypeNameWrapper(array_value_type_name).array();
            }
            else
            {
                usd_type = SdfValueTypeNameWrapper(arnold_type_name);
            }

            auto name = AiParamGetName(param).c_str();
            if (!attr_namespace.empty())
            {
                name = (attr_namespace + ":" + name).c_str();
            }

            auto attr_spec = SdfAttributeSpec::New(spec, name, usd_type);
            auto default_value = get_value_for_arnold_param(param);
            attr_spec->SetDefaultValue(default_value);

            if (!strcmp(arnold_type_name, "ENUM"))
            {
                auto tokens = get_allowed_tokens(param);
                if (!tokens.empty())
                    attr_spec->SetAllowedTokens(tokens);
            }
        }

        node_entry_map[node_entry_name] = layer;
    }

    if (status == AI_RENDER_STATUS_NOT_STARTED)
        AiEnd();

    return node_entry_map[node_entry_type];
}

OPENDCC_NAMESPACE_CLOSE
