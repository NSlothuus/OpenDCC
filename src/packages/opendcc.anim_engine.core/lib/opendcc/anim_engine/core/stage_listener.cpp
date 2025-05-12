// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/anim_engine/core/stage_listener.h"
#include <algorithm>
#include <pxr/usd/usd/primRange.h>
#include "pxr/imaging/hd/primGather.h"
#include <iostream>

PXR_NAMESPACE_USING_DIRECTIVE

OPENDCC_NAMESPACE_OPEN

void StageListener::init(PXR_NS::UsdStageRefPtr stage, const PXR_NS::TfTokenVector& filds, IStageListenerClient* client)
{
    m_stage = stage;
    m_client = client;
    m_filds = filds;
    std::unordered_set<SdfPath, SdfPath::Hash> attrs_to_update;

    for (UsdPrim prim : m_stage->Traverse())
    {
        for (UsdAttribute& attr : prim.GetAttributes())
        {
            if (is_attribute_has_required_metadata(attr))
            {
                attrs_to_update.insert(attr.GetPath());
                m_sorted_paths.Insert(attr.GetPath());
            }
        }
    }
    if (attrs_to_update.size() > 0)
    {
        client->update(attrs_to_update, std::unordered_set<SdfPath, SdfPath::Hash>());
    }
    m_objects_changed_notice_key = TfNotice::Register(TfCreateWeakPtr(this), &StageListener::on_objects_changed);
}

bool StageListener::is_attribute_has_required_metadata(const PXR_NS::UsdAttribute& attr) const
{
    if (attr)
    {
        for (auto& token : m_filds)
        {
            if (!attr.HasMetadata(token))
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

void StageListener::on_objects_changed(UsdNotice::ObjectsChanged const& notice)
{
    if (m_mute_recursion_depth > 0)
    {
        return;
    }

    auto mute_scope = create_mute_scope();

    using PathRange = UsdNotice::ObjectsChanged::PathRange;
    const PathRange paths_to_resync = notice.GetResyncedPaths();
    const PathRange paths_to_update = notice.GetChangedInfoOnlyPaths();

    HdPrimGather gather;
    SdfPathVector exists_paths;
    std::unordered_set<SdfPath, SdfPath::Hash> attrs_to_update;
    std::unordered_set<SdfPath, SdfPath::Hash> attrs_to_remove;

    const SdfPathVector& sorted_exists_paths_vector = m_sorted_paths.GetIds();

    for (PathRange::const_iterator path = paths_to_resync.begin(); path != paths_to_resync.end(); ++path)
    {
        if (path->IsPrimPath() || *path == SdfPath::AbsoluteRootPath())
        {
            gather.Subtree(sorted_exists_paths_vector, path->GetAbsoluteRootOrPrimPath(), &exists_paths);
            auto prim = m_stage->GetPrimAtPath(path->GetAbsoluteRootOrPrimPath());
            if (prim)
            {

                UsdPrimRange range(prim);
                for (auto sub_path = range.begin(); sub_path != range.end(); ++sub_path)
                {
                    UsdPrim prim = m_stage->GetPrimAtPath(sub_path->GetPath());
                    if (prim)
                    {
                        auto attrs = prim.GetAttributes();
                        for (auto& attr : attrs)
                        {
                            if (is_attribute_has_required_metadata(attr))
                            {
                                attrs_to_update.insert(attr.GetPath());
                            }
                        }
                    }
                }

                for (auto& path : exists_paths)
                {
                    if (attrs_to_update.find(path) == attrs_to_update.end())
                    {
                        attrs_to_remove.insert(path);
                    }
                }
            }
            else
            {
                for (auto& path : exists_paths)
                    attrs_to_remove.insert(path);
            }
        }
        else // is attr path
        {
            auto attr = m_stage->GetAttributeAtPath(*path);
            if (is_attribute_has_required_metadata(attr))
            {
                attrs_to_update.insert(*path);
            }
            else if (std::binary_search(sorted_exists_paths_vector.begin(), sorted_exists_paths_vector.end(), *path))
            {
                attrs_to_remove.insert(*path);
            }
        }
    }

    for (PathRange::const_iterator path = paths_to_update.begin(); path != paths_to_update.end(); ++path)
    {
        if (path->IsPrimPath())
        {
            continue;
        }

        if (auto attr = m_stage->GetAttributeAtPath(*path))
        {
            auto changed_fields = notice.GetChangedFields(attr);
            for (auto fild : m_filds)
            {
                if (std::find(changed_fields.begin(), changed_fields.end(), fild) != changed_fields.end())
                {
                    if (is_attribute_has_required_metadata(attr))
                    {
                        attrs_to_update.insert(*path);
                    }
                    else if (std::binary_search(sorted_exists_paths_vector.begin(), sorted_exists_paths_vector.end(), *path))
                    {
                        attrs_to_remove.insert(*path);
                    }
                }
            }
        }
        else if (std::binary_search(sorted_exists_paths_vector.begin(), sorted_exists_paths_vector.end(), *path))
        {
            attrs_to_remove.insert(*path);
        }
    }

    for (auto path : attrs_to_update)
    {
        m_sorted_paths.Insert(path);
    }

    for (auto path : attrs_to_remove)
    {
        m_sorted_paths.Remove(path);
    }

    if (attrs_to_update.size() > 0 || attrs_to_remove.size() > 0)
    {
        m_client->update(attrs_to_update, attrs_to_remove);
    }
}

OPENDCC_NAMESPACE_CLOSE
