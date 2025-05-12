// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "node_definitions.h"
#include <pxr/base/plug/registry.h>
#include <pxr/base/plug/plugin.h>
#include "discovery.h"
#include "scene/shader_nodes.h"
#include "session/session.h"
#include "pxr/usd/usd/attribute.h"
#include "pxr/usd/usdShade/shader.h"

PXR_NAMESPACE_USING_DIRECTIVE
OPENDCC_NAMESPACE_OPEN
using namespace ccl;

namespace
{
    template <class T>
    T to_cpp_value(const T& val)
    {
        return val;
    }

    GfVec4f to_cpp_value(const ccl::float4& val)
    {
        return GfVec4f(val.x, val.y, val.z, val.w);
    }
    GfVec3f to_cpp_value(const ccl::float3& val)
    {
        return GfVec3f(val.x, val.y, val.z);
    }
    GfVec2f to_cpp_value(const ccl::float2& val)
    {
        return GfVec2f(val.x, val.y);
    }
    std::string to_cpp_value(const ccl::ustring& val)
    {
        return val.string();
    }
    GfMatrix4d to_cpp_value(const ccl::Transform& val)
    {
        const GfMatrix4d transform(val.x.x, val.y.x, val.z.x, 0, val.x.y, val.y.y, val.z.y, 0, val.x.z, val.y.z, val.z.z, 0, 0, 0, 0, 1);
        return transform;
    }

    template <class T>
    auto to_cpp_array_value(const T& val) -> VtArray<decltype(to_cpp_value(T()[0]))>
    {
        VtArray<decltype(to_cpp_value(val[0]))> result;
        for (const auto& entry : val)
            result.push_back(to_cpp_value(entry));
        return result;
    }

    VtValue to_vt_value(ccl::SocketType::Type type, const void* default_value, const ccl::NodeEnum* enum_values)
    {
        if (!default_value)
            return VtValue(TfToken());
        using Type = SocketType::Type;
#define CAST_TO_VTVALUE(socket_type, cpp_type) \
    case socket_type:                          \
        return VtValue(to_cpp_value(*reinterpret_cast<const cpp_type*>(default_value)))
#define CAST_ARRAY_TO_VTVALUE(socket_type, cpp_type) \
    case socket_type:                                \
        return VtValue(to_cpp_array_value(*reinterpret_cast<const cpp_type*>(default_value)))

        switch (type)
        {
            CAST_TO_VTVALUE(Type::BOOLEAN, bool);
            CAST_TO_VTVALUE(Type::FLOAT, float);
            CAST_TO_VTVALUE(Type::INT, int);
            CAST_TO_VTVALUE(Type::UINT, ccl::uint);
            CAST_TO_VTVALUE(Type::COLOR, ccl::float3);
            CAST_TO_VTVALUE(Type::VECTOR, ccl::float3);
            CAST_TO_VTVALUE(Type::POINT, ccl::float3);
            CAST_TO_VTVALUE(Type::NORMAL, ccl::float3);
            CAST_TO_VTVALUE(Type::POINT2, ccl::float2);
            CAST_TO_VTVALUE(Type::STRING, ccl::ustring);
            CAST_TO_VTVALUE(Type::TRANSFORM, ccl::Transform);
            CAST_ARRAY_TO_VTVALUE(Type::BOOLEAN_ARRAY, ccl::array<bool>);
            CAST_ARRAY_TO_VTVALUE(Type::FLOAT_ARRAY, ccl::array<float>);
            CAST_ARRAY_TO_VTVALUE(Type::INT_ARRAY, ccl::array<int>);
            CAST_ARRAY_TO_VTVALUE(Type::COLOR_ARRAY, ccl::array<ccl::float3>);
            CAST_ARRAY_TO_VTVALUE(Type::VECTOR_ARRAY, ccl::array<ccl::float3>);
            CAST_ARRAY_TO_VTVALUE(Type::POINT_ARRAY, ccl::array<ccl::float3>);
            CAST_ARRAY_TO_VTVALUE(Type::NORMAL_ARRAY, ccl::array<ccl::float3>);
            CAST_ARRAY_TO_VTVALUE(Type::POINT2_ARRAY, ccl::array<ccl::float2>);
            CAST_ARRAY_TO_VTVALUE(Type::STRING_ARRAY, ccl::array<ccl::ustring>);
            CAST_ARRAY_TO_VTVALUE(Type::TRANSFORM_ARRAY, ccl::array<ccl::Transform>);
        case Type::ENUM:
        {
            auto val = (*reinterpret_cast<const int*>(default_value));
            if (!enum_values)
                return VtValue(TfToken());
            return VtValue(TfToken((*enum_values)[val].string()));
        }
        case Type::CLOSURE:
        case Type::NODE:
        case Type::NODE_ARRAY:
        default:
            return VtValue(TfToken());
        }
#undef CAST_TO_VTVALUE
#undef CAST_ARRAY_TO_VTVALUE
    }

    SdfValueTypeName get_sdf_type_name(SocketType::Type type)
    {
        using Type = SocketType::Type;
#define CONVERT_TO_SDF_TYPE(socket_type, sdf_type) \
    case socket_type:                              \
        return sdf_type

        switch (type)
        {
            CONVERT_TO_SDF_TYPE(Type::BOOLEAN, SdfValueTypeNames->Bool);
            CONVERT_TO_SDF_TYPE(Type::FLOAT, SdfValueTypeNames->Float);
            CONVERT_TO_SDF_TYPE(Type::INT, SdfValueTypeNames->Int);
            CONVERT_TO_SDF_TYPE(Type::UINT, SdfValueTypeNames->UInt);
            CONVERT_TO_SDF_TYPE(Type::COLOR, SdfValueTypeNames->Color3f);
            CONVERT_TO_SDF_TYPE(Type::VECTOR, SdfValueTypeNames->Vector3f);
            CONVERT_TO_SDF_TYPE(Type::POINT, SdfValueTypeNames->Point3f);
            CONVERT_TO_SDF_TYPE(Type::NORMAL, SdfValueTypeNames->Normal3f);
            CONVERT_TO_SDF_TYPE(Type::POINT2, SdfValueTypeNames->Float2);
            CONVERT_TO_SDF_TYPE(Type::CLOSURE, SdfValueTypeNames->Token);
            CONVERT_TO_SDF_TYPE(Type::STRING, SdfValueTypeNames->String);
            CONVERT_TO_SDF_TYPE(Type::ENUM, SdfValueTypeNames->Token);
            CONVERT_TO_SDF_TYPE(Type::NODE, SdfValueTypeNames->Token);
            CONVERT_TO_SDF_TYPE(Type::TRANSFORM, SdfValueTypeNames->Matrix4d);
            CONVERT_TO_SDF_TYPE(Type::BOOLEAN_ARRAY, SdfValueTypeNames->BoolArray);
            CONVERT_TO_SDF_TYPE(Type::FLOAT_ARRAY, SdfValueTypeNames->FloatArray);
            CONVERT_TO_SDF_TYPE(Type::INT_ARRAY, SdfValueTypeNames->IntArray);
            CONVERT_TO_SDF_TYPE(Type::COLOR_ARRAY, SdfValueTypeNames->Color3fArray);
            CONVERT_TO_SDF_TYPE(Type::VECTOR_ARRAY, SdfValueTypeNames->Vector3fArray);
            CONVERT_TO_SDF_TYPE(Type::POINT_ARRAY, SdfValueTypeNames->Point3fArray);
            CONVERT_TO_SDF_TYPE(Type::NORMAL_ARRAY, SdfValueTypeNames->Normal3fArray);
            CONVERT_TO_SDF_TYPE(Type::POINT2_ARRAY, SdfValueTypeNames->Float2Array);
            CONVERT_TO_SDF_TYPE(Type::STRING_ARRAY, SdfValueTypeNames->StringArray);
            CONVERT_TO_SDF_TYPE(Type::NODE_ARRAY, SdfValueTypeNames->Token);
            CONVERT_TO_SDF_TYPE(Type::TRANSFORM_ARRAY, SdfValueTypeNames->Matrix4dArray);
        default:
            return SdfValueTypeName();
        }
#undef CONVERT_TO_SDF_TYPE
    }

    void set_usd_value(const SocketType& socket_type, const UsdAttribute& attr)
    {
        if (socket_type.enum_values)
        {
            VtTokenArray allowed_tokens;
            std::map<int, std::string> vals;
            for (const auto& val : *socket_type.enum_values)
                vals[val.second] = val.first.string();
            for (const auto& val : vals)
                allowed_tokens.push_back(TfToken(val.second));
            attr.SetMetadata(SdfFieldKeys->AllowedTokens, VtValue(allowed_tokens));
        }
        if (socket_type.default_value)
        {
            const auto val = to_vt_value(socket_type.type, socket_type.default_value, socket_type.enum_values);
            if (attr.GetTypeName() == SdfValueTypeNames->Asset)
            {
                SdfAssetPath asset_path_val { val.Get<std::string>() };
                attr.Set(asset_path_val);
            }
            else
            {
                attr.Set(val);
            }
        }
    }

    TfToken format_attr_name(const ccl::ustring& name)
    {
        std::string result = name.string();
        if (result == "curves")
            volatile auto stop = 5;
        std::replace(result.begin(), result.end(), '.', ':');
        return TfToken(result);
    }
};

UsdStageRefPtr get_node_definitions()
{
    static const auto result = []() -> UsdStageRefPtr {
        // Needed to force link cycles static libs
        auto _ = std::make_unique<Session>(SessionParams(), SceneParams());

        auto stage = UsdStage::CreateInMemory();
        for (const auto t : NodeType::types())
        {
            const auto node_name = t.first.string();
            if (t.second.type == NodeType::Type::SHADER)
            {
                auto prim = stage->DefinePrim(SdfPath("/" + node_name), TfToken("Shader"));
                auto shader = UsdShadeShader(prim);
                shader.CreateIdAttr(VtValue(TfToken("cycles:" + node_name)));
                for (const auto& input : t.second.inputs)
                {
                    SdfValueTypeName inp_type;
                    if (input.name == "filename")
                    {
                        inp_type = SdfValueTypeNames->Asset;
                    }
                    else
                    {
                        inp_type = get_sdf_type_name(input.type);
                    }

                    auto shader_input = shader.CreateInput(format_attr_name(input.name), inp_type);
                    set_usd_value(input, shader_input.GetAttr());
                }
                for (const auto& output : t.second.outputs)
                {
                    auto shader_output = shader.CreateOutput(format_attr_name(output.name), get_sdf_type_name(output.type));
                    set_usd_value(output, shader_output.GetAttr());
                }
            }
            else
            {
                auto prim = stage->DefinePrim(SdfPath("/" + node_name));
                for (const auto& input : t.second.inputs)
                {
                    auto attr = prim.CreateAttribute(format_attr_name(input.name), get_sdf_type_name(input.type));
                    set_usd_value(input, attr);
                }
                for (const auto& output : t.second.outputs)
                {
                    auto attr = prim.CreateAttribute(format_attr_name(output.name), get_sdf_type_name(output.type));
                    set_usd_value(output, attr);
                }
            }
        }

        return stage;
    }();

    return result;
}

OPENDCC_NAMESPACE_CLOSE
