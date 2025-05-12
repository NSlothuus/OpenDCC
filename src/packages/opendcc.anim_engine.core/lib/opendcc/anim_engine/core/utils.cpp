// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/anim_engine/core/utils.h"
#include "opendcc/anim_engine/curve/curve.h"
#include <iostream>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE;

bool is_attribute_may_animated(const PXR_NS::UsdAttribute& usd_attr)
{
    auto type_name = usd_attr.GetTypeName();
    return type_name == SdfValueTypeNames->Float || type_name == SdfValueTypeNames->Double || type_name == SdfValueTypeNames->Float2 ||
           type_name == SdfValueTypeNames->Double2 || type_name == SdfValueTypeNames->Float3 || type_name == SdfValueTypeNames->Double3 ||
           type_name == SdfValueTypeNames->Float4 || type_name == SdfValueTypeNames->Double4 || type_name == SdfValueTypeNames->Vector3f ||
           type_name == SdfValueTypeNames->Vector3d || type_name == SdfValueTypeNames->Point3f || type_name == SdfValueTypeNames->Point3d ||
           type_name == SdfValueTypeNames->Color3f || type_name == SdfValueTypeNames->Color3d || type_name == SdfValueTypeNames->Int ||
           type_name == SdfValueTypeNames->Bool;
}

AttributeClass attribute_class(const PXR_NS::UsdAttribute& usd_attr)
{
    auto type_name = usd_attr.GetTypeName();
    if (type_name == SdfValueTypeNames->Float || type_name == SdfValueTypeNames->Double || type_name == SdfValueTypeNames->Int ||
        type_name == SdfValueTypeNames->Bool)
    {
        return AttributeClass::Scalar;
    }
    else if (type_name == SdfValueTypeNames->Float2 || type_name == SdfValueTypeNames->Double2 || type_name == SdfValueTypeNames->Float3 ||
             type_name == SdfValueTypeNames->Double3 || type_name == SdfValueTypeNames->Float4 || type_name == SdfValueTypeNames->Double4 ||
             type_name == SdfValueTypeNames->Vector3f || type_name == SdfValueTypeNames->Vector3d || type_name == SdfValueTypeNames->Point3f ||
             type_name == SdfValueTypeNames->Point3d)
    {
        return AttributeClass::Vector;
    }
    else if (type_name == SdfValueTypeNames->Color3f || type_name == SdfValueTypeNames->Color3d)
    {
        return AttributeClass::Color;
    }
    else
        return AttributeClass::Other;
}

uint32_t num_components_in_attribute(const PXR_NS::UsdAttribute& usd_attr)
{
    auto type_name = usd_attr.GetTypeName();
    if (type_name == SdfValueTypeNames->Float || type_name == SdfValueTypeNames->Int || type_name == SdfValueTypeNames->Bool ||
        type_name == SdfValueTypeNames->Double)
        return 1;

    if (type_name == SdfValueTypeNames->Float2 || type_name == SdfValueTypeNames->Double2)
        return 2;

    if (type_name == SdfValueTypeNames->Float3 || type_name == SdfValueTypeNames->Double3 || type_name == SdfValueTypeNames->Vector3f ||
        type_name == SdfValueTypeNames->Vector3d || type_name == SdfValueTypeNames->Point3f || type_name == SdfValueTypeNames->Point3d ||
        type_name == SdfValueTypeNames->Color3f || type_name == SdfValueTypeNames->Color3d)
        return 3;

    if (type_name == SdfValueTypeNames->Float4 || type_name == SdfValueTypeNames->Double4)
        return 4;
    return 0;
}

bool set_usd_attribute_component(PXR_NS::UsdAttribute usd_attr, std::vector<uint32_t> components, std::vector<double> values, UsdTimeCode time)
{
    ANIM_CURVES_CHECK_AND_RETURN_VAL(components.size() == values.size(), false);
    ANIM_CURVES_CHECK_AND_RETURN_VAL(usd_attr, false);
    if (usd_attr.GetTypeName() == SdfValueTypeNames->Float)
    {
        if (components.size() != 1 || components[0] != 0)
            return false;
        usd_attr.Set<float>((float)values[0], time);
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Double)
    {
        if (components.size() != 1 || components[0] != 0)
            return false;
        usd_attr.Set<double>(values[0], time);
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Int)
    {
        if (components.size() != 1 || components[0] != 0)
            return false;
        usd_attr.Set<int>((int)values[0], time);
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Bool)
    {
        if (components.size() != 1 || components[0] != 0)
            return false;
        usd_attr.Set<bool>((bool)values[0], time);
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Float2)
    {
        if (components.size() > 2)
            return false;
        GfVec2f usd_value;
        usd_attr.Get<GfVec2f>(&usd_value, time);
        for (size_t i = 0; i < components.size(); ++i)
            usd_value[components[i]] = (float)values[i];
        usd_attr.Set<GfVec2f>(usd_value, time);
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Double2)
    {
        if (components.size() > 2)
            return false;
        GfVec2d usd_value;
        usd_attr.Get<GfVec2d>(&usd_value, time);
        for (size_t i = 0; i < components.size(); ++i)
            usd_value[components[i]] = values[i];
        usd_attr.Set<GfVec2d>(usd_value, time);
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Float3 || usd_attr.GetTypeName() == SdfValueTypeNames->Point3f ||
             usd_attr.GetTypeName() == SdfValueTypeNames->Color3f || usd_attr.GetTypeName() == SdfValueTypeNames->Vector3f)
    {
        if (components.size() > 3)
            return false;
        GfVec3f usd_value;
        usd_attr.Get<GfVec3f>(&usd_value, time);
        for (size_t i = 0; i < components.size(); ++i)
            usd_value[components[i]] = (float)values[i];
        usd_attr.Set<GfVec3f>(usd_value, time);
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Double3 || usd_attr.GetTypeName() == SdfValueTypeNames->Point3d ||
             usd_attr.GetTypeName() == SdfValueTypeNames->Color3d || usd_attr.GetTypeName() == SdfValueTypeNames->Vector3d)
    {
        if (components.size() > 3)
            return false;
        GfVec3d usd_value;
        usd_attr.Get<GfVec3d>(&usd_value, time);
        for (size_t i = 0; i < components.size(); ++i)
            usd_value[components[i]] = values[i];
        usd_attr.Set<GfVec3d>(usd_value, time);
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Float4)
    {
        if (components.size() > 4)
            return false;
        GfVec4f usd_value;
        usd_attr.Get<GfVec4f>(&usd_value, time);
        for (size_t i = 0; i < components.size(); ++i)
            usd_value[components[i]] = (float)values[i];
        usd_attr.Set<GfVec4f>(usd_value, time);
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Double4)
    {
        if (components.size() > 4)
            return false;
        GfVec4d usd_value;
        usd_attr.Get<GfVec4d>(&usd_value, time);
        for (size_t i = 0; i < components.size(); ++i)
            usd_value[components[i]] = values[i];
        usd_attr.Set<GfVec4d>(usd_value, time);
    }
    return true;
}

bool set_usd_attribute_component(PXR_NS::UsdAttribute usd_attr, uint32_t component, double value)
{
    return set_usd_attribute_component(usd_attr, std::vector<uint32_t>(1, component), std::vector<double>(1, value));
}

bool get_usd_attribute_component(const PXR_NS::UsdAttribute& usd_attr, uint32_t component, double& value, PXR_NS::UsdTimeCode time)
{
    if (usd_attr.GetTypeName() == SdfValueTypeNames->Float)
    {
        if (component > 0)
            return false;
        float usd_value;
        usd_attr.Get<float>(&usd_value, time);
        value = usd_value;
        return true;
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Double)
    {
        if (component > 0)
            return false;
        double usd_value;
        usd_attr.Get<double>(&usd_value, time);
        value = usd_value;
        return true;
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Bool)
    {
        if (component > 0)
            return false;
        bool usd_value;
        usd_attr.Get<bool>(&usd_value, time);
        value = usd_value;
        return true;
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Int)
    {
        if (component > 0)
            return false;
        int usd_value;
        usd_attr.Get<int>(&usd_value, time);
        value = usd_value;
        return true;
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Float2)
    {
        if (component > 1)
            return false;
        GfVec2f usd_value;
        usd_attr.Get<GfVec2f>(&usd_value, time);
        value = usd_value[component];
        return true;
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Double2)
    {
        if (component > 1)
            return false;
        GfVec2d usd_value;
        usd_attr.Get<GfVec2d>(&usd_value, time);
        value = usd_value[component];
        return true;
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Float3 || usd_attr.GetTypeName() == SdfValueTypeNames->Point3f ||
             usd_attr.GetTypeName() == SdfValueTypeNames->Vector3f || usd_attr.GetTypeName() == SdfValueTypeNames->Color3f)
    {
        if (component > 2)
            return false;
        GfVec3f usd_value;
        usd_attr.Get<GfVec3f>(&usd_value, time);
        value = usd_value[component];
        return true;
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Double3 || usd_attr.GetTypeName() == SdfValueTypeNames->Point3d ||
             usd_attr.GetTypeName() == SdfValueTypeNames->Vector3d || usd_attr.GetTypeName() == SdfValueTypeNames->Color3d)
    {
        if (component > 2)
            return false;
        GfVec3d usd_value;
        usd_attr.Get<GfVec3d>(&usd_value, time);
        value = usd_value[component];
        return true;
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Float4)
    {
        if (component > 3)
            return false;
        GfVec4f usd_value;
        usd_attr.Get<GfVec4f>(&usd_value, time);
        value = usd_value[component];
        return true;
    }
    else if (usd_attr.GetTypeName() == SdfValueTypeNames->Double4)
    {
        if (component > 3)
            return false;
        GfVec4d usd_value;
        usd_attr.Get<GfVec4d>(&usd_value, time);
        value = usd_value[component];
        return true;
    }
    return false;
}

AnimEngine::CurveIdToKeysIdsMap keyframes_to_keyIds(const AnimEngine::CurveIdToKeyframesMap& keyframes)
{
    AnimEngine::CurveIdToKeysIdsMap result;
    for (auto it : keyframes)
    {
        auto& current_ids = result[it.first];
        for (auto& key : it.second)
            current_ids.insert(key.id);
    }
    return result;
}

OPENDCC_NAMESPACE_CLOSE
