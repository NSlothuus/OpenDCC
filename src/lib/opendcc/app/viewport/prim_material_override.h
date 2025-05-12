/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/imaging/hd/sceneDelegate.h>
#include <pxr/usd/usd/stage.h>
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"
#include "opendcc/base/vendor/eventpp/utilities/scopedremover.h"

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API PrimMaterialDescriptor
{
public:
    using PrimvarDescriptorMap = std::unordered_map<PXR_NS::HdInterpolation, PXR_NS::HdPrimvarDescriptorVector>;
    PrimMaterialDescriptor() = default;
#if PXR_VERSION < 2002
    PrimMaterialDescriptor(const std::string& surface_shader_src,
                           const std::unordered_map<PXR_NS::HdInterpolation, PXR_NS::HdPrimvarDescriptorVector>& primvar_descriptors);
    const std::string& get_surface_shader_source() const;
#else
    PrimMaterialDescriptor(const PXR_NS::VtValue& mat_resource, std::weak_ptr<std::function<PXR_NS::VtValue()>> updater,
                           const std::unordered_map<PXR_NS::HdInterpolation, PXR_NS::HdPrimvarDescriptorVector>& primvar_descriptors);
    PrimMaterialDescriptor(const PXR_NS::VtValue& mat_resource,
                           const std::unordered_map<PXR_NS::HdInterpolation, PXR_NS::HdPrimvarDescriptorVector>& primvar_descriptors = {});
    const PXR_NS::VtValue& get_material_resource() const;
    bool update_material_resource();
#endif
    bool has_primvar_descriptor(PXR_NS::HdInterpolation interpolation) const;
    PXR_NS::HdPrimvarDescriptorVector get_primvar_descriptors(PXR_NS::HdInterpolation interpolation) const;

private:
#if PXR_VERSION < 2002
    std::string m_surface_shader_source;
#else
    PXR_NS::VtValue m_material_resource;
    std::weak_ptr<std::function<PXR_NS::VtValue()>> m_updater;
#endif
    std::unordered_map<PXR_NS::HdInterpolation, PXR_NS::HdPrimvarDescriptorVector> m_primvar_descriptors;
};

class OPENDCC_API PrimMaterialOverride
{
public:
    enum class Status
    {
        NEW,
        CHANGED,
        REMOVED
    };
    enum class EventType
    {
        MATERIAL,
        MATERIAL_RESOURCE,
        ASSIGNMENT,
        COUNT
    };
    using MaterialDispatcher = eventpp::EventDispatcher<EventType, void(size_t material, const PrimMaterialDescriptor& descr, Status status)>;
    using AssignmentDispatcher = eventpp::EventDispatcher<EventType, void(size_t material, const PXR_NS::SdfPath& assignment, Status status)>;
    using MaterialResourceDispatcher =
        eventpp::EventDispatcher<EventType, void(PXR_NS::SdfPath mat_path, const PrimMaterialDescriptor& descr, Status status)>;
    using MaterialDispatcherHandle = MaterialDispatcher::Handle;
    using MaterialResourceDispatcherHandle = MaterialResourceDispatcher::Handle;
    using AssignmentDispatcherHandle = AssignmentDispatcher::Handle;

    ~PrimMaterialOverride();
    size_t insert_material(const PrimMaterialDescriptor& descr);
    void update_material(size_t material_id, const PrimMaterialDescriptor& descr);
    void assign_material(size_t material_id, PXR_NS::SdfPath path);
    void material_resource_override(const PXR_NS::SdfPath& mat_path, const PrimMaterialDescriptor& descr);
    void clear_material_resource_override(const PXR_NS::SdfPath& mat_path);
    void remove_material(size_t material_id);
    void clear_override(PXR_NS::SdfPath path);
    void clear_all();

    template <EventType event>
    MaterialDispatcherHandle register_callback(
        std::enable_if_t<event == EventType::MATERIAL, std::function<void(size_t, const PrimMaterialDescriptor&, Status)>> callback)
    {
        return m_material_dispatcher.appendListener(event, callback);
    }

    template <EventType event>
    MaterialResourceDispatcherHandle register_callback(
        std::enable_if_t<event == EventType::MATERIAL_RESOURCE, std::function<void(PXR_NS::SdfPath, const PrimMaterialDescriptor&, Status)>> callback)
    {
        return m_material_resource_dispatcher.appendListener(event, callback);
    }
    template <EventType event>
    AssignmentDispatcherHandle register_callback(
        std::enable_if_t<event == EventType::ASSIGNMENT, std::function<void(size_t, const PXR_NS::SdfPath&, Status)>> callback)
    {
        return m_assignment_dispatcher.appendListener(event, callback);
    }

    template <class THandle>
    std::enable_if_t<std::is_same<THandle, MaterialDispatcherHandle>::value> unregister_callback(THandle handle)
    {
        m_material_dispatcher.removeListener(EventType::MATERIAL, handle);
    }

    template <class THandle>
    std::enable_if_t<std::is_same<THandle, MaterialResourceDispatcherHandle>::value> unregister_callback(THandle handle)
    {
        m_material_resource_dispatcher.removeListener(EventType::MATERIAL_RESOURCE, handle);
    }

    template <class THandle>
    std::enable_if_t<std::is_same<THandle, AssignmentDispatcherHandle>::value> unregister_callback(THandle handle)
    {
        m_assignment_dispatcher.removeListener(EventType::ASSIGNMENT, handle);
    }

    const std::unordered_map<size_t, PrimMaterialDescriptor>& get_materials() const;
    const std::unordered_map<PXR_NS::SdfPath, size_t, PXR_NS::SdfPath::Hash>& get_assignments() const;
    const std::unordered_map<PXR_NS::SdfPath, PrimMaterialDescriptor, PXR_NS::SdfPath::Hash>& get_material_resource_overrides() const;

private:
    MaterialDispatcher m_material_dispatcher;
    AssignmentDispatcher m_assignment_dispatcher;
    MaterialResourceDispatcher m_material_resource_dispatcher;

    std::unordered_map<size_t, PrimMaterialDescriptor> m_materials;
    std::unordered_map<PXR_NS::SdfPath, size_t, PXR_NS::SdfPath::Hash> m_assignments;
    std::unordered_map<PXR_NS::SdfPath, PrimMaterialDescriptor, PXR_NS::SdfPath::Hash> m_mat_resource_overrides;
    size_t m_material_id = 0;
};

OPENDCC_NAMESPACE_CLOSE
