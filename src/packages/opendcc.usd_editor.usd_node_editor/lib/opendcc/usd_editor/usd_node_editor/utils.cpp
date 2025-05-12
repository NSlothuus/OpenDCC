// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/usd_node_editor/utils.h"
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/relationship.h>

PXR_NAMESPACE_USING_DIRECTIVE

OPENDCC_NAMESPACE_OPEN

bool remove_connection(UsdProperty& prop, const SdfPath& connection)
{
    auto edit_layer = prop.GetStage()->GetEditTarget().GetLayer();
    SdfPathVector connections;
    auto attr = prop.As<UsdAttribute>();
    auto rel = prop.As<UsdRelationship>();

    if (auto attr = prop.As<UsdAttribute>())
    {
        auto attr_spec = edit_layer->GetAttributeAtPath(prop.GetPath());
        // remove edits on edit target if the target to delete is on the other layer
        // then just append `delete` op
        if (attr_spec)
        {
            auto cur_layer_targets = attr_spec->GetConnectionPathList().GetAddedOrExplicitItems();
            if (std::find(cur_layer_targets.begin(), cur_layer_targets.end(), connection) != cur_layer_targets.end())
            {
                attr_spec->GetConnectionPathList().RemoveItemEdits(connection);
            }
        }
        SdfPathVector targets;
        attr.GetConnections(&targets);
        if (std::find(targets.begin(), targets.end(), connection) != targets.end())
        {
            return attr.RemoveConnection(connection);
        }
    }
    else if (auto rel = prop.As<UsdRelationship>())
    {
        auto rel_spec = edit_layer->GetRelationshipAtPath(prop.GetPath());
        // remove edits on edit target if the target to delete is on the other layer
        // then just append `delete` op
        if (rel_spec)
        {
            auto cur_layer_targets = rel_spec->GetTargetPathList().GetAddedOrExplicitItems();
            if (std::find(cur_layer_targets.begin(), cur_layer_targets.end(), connection) != cur_layer_targets.end())
            {
                rel_spec->GetTargetPathList().RemoveItemEdits(connection);
                return true;
            }
        }
        SdfPathVector targets;
        rel.GetTargets(&targets);
        if (std::find(targets.begin(), targets.end(), connection) != targets.end())
        {
            return rel.RemoveTarget(connection);
        }
    }
    return false;
}
bool remove_connections(UsdProperty& prop, const SdfPathVector& connections)
{
    bool result = true;
    for (const auto& connect : connections)
    {
        result &= remove_connection(prop, connect);
    }
    return result;
}

OPENDCC_NAMESPACE_CLOSE
