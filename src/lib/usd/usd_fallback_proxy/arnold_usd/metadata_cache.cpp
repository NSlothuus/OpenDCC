// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "usd/usd_fallback_proxy/arnold_usd/metadata_cache.h"
#include <ai.h>
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/attribute.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/matrix4f.h"
#include "usdUIExt/tokens.h"
#include "arnold_usd_property_factory.h"
#include "pxr/base/plug/registry.h"
#include "pxr/base/plug/plugin.h"
#include <pxr/usd/ndr/registry.h>
#include <set>
PXR_NAMESPACE_USING_DIRECTIVE;
OPENDCC_NAMESPACE_OPEN

namespace
{
    using AtValueConverter = std::function<VtValue(const AtParamValue&, const AtParamEntry*)>;

    GfMatrix4d convert_matrix(const AtMatrix& mat)
    {
        return GfMatrix4d(GfMatrix4f(mat.data));
    }
    const char* GetEnum(AtEnum en, int32_t id)
    {
        if (en == nullptr)
            return "";
        if (id < 0)
            return "";
        for (auto i = 0; i <= id; ++i)
        {
            if (en[i] == nullptr)
                return "";
        }
        return en[id];
    }
    const std::unordered_map<uint8_t, AtValueConverter> default_value_conversion_map = {
        { AI_TYPE_BYTE,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              return VtValue(pv.BYTE());
          } },
        { AI_TYPE_INT,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              return VtValue(pv.INT());
          } },
        { AI_TYPE_UINT,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              return VtValue(pv.UINT());
          } },
        { AI_TYPE_BOOLEAN,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              return VtValue(pv.BOOL());
          } },
        { AI_TYPE_FLOAT,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              return VtValue(pv.FLT());
          } },
        { AI_TYPE_RGB,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              const auto& v = pv.RGB();
              return VtValue(GfVec3f(v.r, v.g, v.b));
          } },
        { AI_TYPE_RGBA,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              const auto& v = pv.RGBA();
              return VtValue(GfVec4f(v.r, v.g, v.b, v.a));
          } },
        { AI_TYPE_VECTOR,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              const auto& v = pv.VEC();
              return VtValue(GfVec3f(v.x, v.y, v.z));
          } },
        { AI_TYPE_VECTOR2,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              const auto& v = pv.VEC2();
              return VtValue(GfVec2f(v.x, v.y));
          } },
        { AI_TYPE_STRING,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              return VtValue(pv.STR().c_str());
          } },
        { AI_TYPE_POINTER, nullptr },
        { AI_TYPE_NODE, nullptr },
        { AI_TYPE_MATRIX,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              return VtValue(convert_matrix(*pv.pMTX()));
          } },
        { AI_TYPE_ENUM,
          [](const AtParamValue& pv, const AtParamEntry* pe) -> VtValue {
              if (pe == nullptr)
              {
                  return VtValue("");
              }
              const auto enums = AiParamGetEnum(pe);
              return VtValue(GetEnum(enums, pv.INT()));
          } },
        { AI_TYPE_CLOSURE, nullptr },
        { AI_TYPE_USHORT,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              return VtValue(pv.UINT());
          } },
        { AI_TYPE_HALF,
          [](const AtParamValue& pv, const AtParamEntry*) -> VtValue {
              return VtValue(pv.FLT());
          } },
    };

    const std::unordered_map<uint8_t, SdfValueTypeName> arnold_type_to_sdf_value_type_name = {
        { AI_TYPE_BYTE, SdfValueTypeNames->UChar },      { AI_TYPE_INT, SdfValueTypeNames->Int },
        { AI_TYPE_UINT, SdfValueTypeNames->UInt },       { AI_TYPE_BOOLEAN, SdfValueTypeNames->Bool },
        { AI_TYPE_FLOAT, SdfValueTypeNames->Float },     { AI_TYPE_RGB, SdfValueTypeNames->Color3f },
        { AI_TYPE_RGBA, SdfValueTypeNames->Color4f },    { AI_TYPE_VECTOR, SdfValueTypeNames->Vector3f },
        { AI_TYPE_VECTOR2, SdfValueTypeNames->Float2 },  { AI_TYPE_STRING, SdfValueTypeNames->String },
        { AI_TYPE_POINTER, SdfValueTypeNames->Token },   { AI_TYPE_NODE, SdfValueTypeNames->Token },
        { AI_TYPE_MATRIX, SdfValueTypeNames->Matrix4d }, { AI_TYPE_ENUM, SdfValueTypeNames->Token },
        { AI_TYPE_CLOSURE, SdfValueTypeNames->Token },   { AI_TYPE_USHORT, SdfValueTypeNames->UInt },
        { AI_TYPE_HALF, SdfValueTypeNames->Half }
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

    VtValue convert_at_value(uint8_t type, const AtParamValue& val)
    {
        auto iter = default_value_conversion_map.find(type);
        if (iter == default_value_conversion_map.end())
            return VtValue();
        return iter->second(val, nullptr);
    }

    SdfValueTypeName convert_at_type(uint8_t type)
    {
        return TfMapLookupByValue(arnold_type_to_sdf_value_type_name, type, SdfValueTypeName());
    }
    void write_metadata(const AtNodeEntry* node_entry, const PXR_NS::UsdPrim& prim)
    {
        auto param_iter = AiNodeEntryGetParamIterator(node_entry);
        while (!AiParamIteratorFinished(param_iter))
        {
            const auto param = AiParamIteratorGetNext(param_iter);
            const auto param_name = AiParamGetName(param);
            const auto param_type = AiParamGetType(param);
            const auto arnold_default_value = AiParamGetDefault(param);

            SdfValueTypeName usd_type;
            if (param_type == AI_TYPE_ARRAY)
                usd_type = SdfValueTypeNameWrapper(AiParamGetTypeName(AiArrayGetType(arnold_default_value->ARRAY())));
            else
                usd_type = SdfValueTypeNameWrapper(AiParamGetTypeName(param_type));

            auto attr = prim.CreateAttribute(TfToken(param_name.c_str()), usd_type, false);

            VtDictionary display_widget_hints;
            auto metadata_iter = AiNodeEntryGetMetaDataIterator(node_entry, param_name);
            while (!AiMetaDataIteratorFinished(metadata_iter))
            {
                const auto metadata = AiMetaDataIteratorGetNext(metadata_iter);
                const auto metadata_value = convert_at_value(metadata->type, metadata->value);
                const std::string metadata_name = metadata->name.c_str();
                if (TfStringStartsWith(metadata_name, "hints."))
                    display_widget_hints[metadata_name.c_str() + 6] = metadata_value;
                else if (SdfSchema::GetInstance().IsRegistered(TfToken(metadata_name)))
                    attr.SetMetadata(TfToken(metadata_name), metadata_value);
            }
            if (!display_widget_hints.empty())
                attr.SetMetadata(UsdUIExtTokens->displayWidgetHints, display_widget_hints);
            AiMetaDataIteratorDestroy(metadata_iter);
        }
        const auto output_type = AiNodeEntryGetOutputType(node_entry);
        std::string comp_list;
        switch (output_type)
        {
        case AI_TYPE_VECTOR2:
            comp_list = "xy";
            break;
        case AI_TYPE_VECTOR:
            comp_list = "xyz";
            break;
        case AI_TYPE_RGB:
            comp_list = "rgb";
            break;
        case AI_TYPE_RGBA:
            comp_list = "rgba";
            break;
        }
        for (auto comp : comp_list)
        {
            prim.CreateAttribute(TfToken(std::string("outputs:") + comp), SdfValueTypeNames->Float, false);
        }

        const auto output_sdf_type = convert_at_type(output_type);
        if (output_sdf_type && comp_list.empty())
            prim.CreateAttribute(TfToken("outputs:out"), output_sdf_type, false);
        AiParamIteratorDestroy(param_iter);
    }
};

PXR_NS::UsdStageRefPtr get_arnold_metadata()
{
    static const auto stage = []() -> UsdStageRefPtr {
        auto result = UsdStage::CreateInMemory("__arnold_node_metadata.usda");
#if AI_VERSION_ARCH_NUM >= 7 && AI_VERSION_MAJOR_NUM >= 2
        const auto universe_is_active = AiArnoldIsActive();
#else
        const auto universe_is_active = AiUniverseIsActive();
#endif
        if (!universe_is_active)
        {
            AiBegin(AI_SESSION_BATCH);
#if AI_VERSION_ARCH_NUM >= 7 && AI_VERSION_MAJOR_NUM >= 2
            AiMsgSetConsoleFlags(nullptr, AI_LOG_NONE);
#else
            AiMsgSetConsoleFlags(AI_LOG_NONE);
#endif
        }

        auto plugin = PlugRegistry::GetInstance().GetPluginForType(TfType::Find<ArnoldUsdPropertyFactory>());
        TF_AXIOM(plugin);
        const auto mtd_file_path = plugin->GetResourcePath() + "/metadata.mtd";
        AiMetaDataLoadFile(mtd_file_path.c_str());

        auto iter = AiUniverseGetNodeEntryIterator(AI_NODE_SHADER);
        while (!AiNodeEntryIteratorFinished(iter))
        {
            auto node_entry = AiNodeEntryIteratorGetNext(iter);
            auto prim = result->DefinePrim(SdfPath(TfStringPrintf("/%s", AiNodeEntryGetName(node_entry))));
            write_metadata(node_entry, prim);
        }
        AiNodeEntryIteratorDestroy(iter);

        if (!universe_is_active)
            AiEnd();

        return result;
    }();
    return stage;
}

OPENDCC_NAMESPACE_CLOSE
