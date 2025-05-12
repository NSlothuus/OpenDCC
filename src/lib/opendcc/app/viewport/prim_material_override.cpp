// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/prim_material_override.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

#if PXR_VERSION < 2002
PrimMaterialDescriptor::PrimMaterialDescriptor(const std::string& surface_shader_src,
                                               const std::unordered_map<HdInterpolation, HdPrimvarDescriptorVector>& primvar_descriptors)
    : m_surface_shader_source(surface_shader_src)
    , m_primvar_descriptors(primvar_descriptors)
{
}

const std::string& PrimMaterialDescriptor::get_surface_shader_source() const
{
    return m_surface_shader_source;
}
#else
PrimMaterialDescriptor::PrimMaterialDescriptor(const VtValue& mat_resource,
                                               const std::unordered_map<HdInterpolation, HdPrimvarDescriptorVector>& primvar_descriptors)
    : m_material_resource(mat_resource)
    , m_primvar_descriptors(primvar_descriptors)
{
}

PrimMaterialDescriptor::PrimMaterialDescriptor(const VtValue& mat_resource, std::weak_ptr<std::function<PXR_NS::VtValue()>> updater,
                                               const std::unordered_map<HdInterpolation, HdPrimvarDescriptorVector>& primvar_descriptors)
    : m_material_resource(mat_resource)
    , m_updater(updater)
    , m_primvar_descriptors(primvar_descriptors)
{
}

const VtValue& PrimMaterialDescriptor::get_material_resource() const
{
    return m_material_resource;
}

bool PrimMaterialDescriptor::update_material_resource()
{
    if (m_updater.expired())
        return false;
    m_material_resource = (*m_updater.lock())();
    return true;
}

#endif

bool PrimMaterialDescriptor::has_primvar_descriptor(PXR_NS::HdInterpolation interpolation) const
{
    return m_primvar_descriptors.find(interpolation) != m_primvar_descriptors.end();
}

HdPrimvarDescriptorVector PrimMaterialDescriptor::get_primvar_descriptors(HdInterpolation interpolation) const
{
    auto iter = m_primvar_descriptors.find(interpolation);
    return iter == m_primvar_descriptors.end() ? HdPrimvarDescriptorVector {} : iter->second;
}

OPENDCC_NAMESPACE::PrimMaterialOverride::~PrimMaterialOverride()
{
    clear_all();
}

size_t PrimMaterialOverride::insert_material(const PrimMaterialDescriptor& descr)
{
    m_materials[m_material_id] = descr;
    m_material_dispatcher.dispatch(EventType::MATERIAL, m_material_id, descr, Status::NEW);
    return m_material_id++;
}

void PrimMaterialOverride::update_material(size_t material_id, const PrimMaterialDescriptor& descr)
{
    auto iter = m_materials.find(material_id);
    Status status;
    if (iter == m_materials.end())
        status = Status::NEW;
    else
        status = Status::CHANGED;

    m_materials[material_id] = descr;
    m_material_dispatcher.dispatch(EventType::MATERIAL, material_id, descr, status);
}

void PrimMaterialOverride::assign_material(size_t material_id, PXR_NS::SdfPath path)
{
    auto iter = m_assignments.find(path);
    if (iter != m_assignments.end() && iter->second == material_id)
        return;

    m_assignments[path] = material_id;
    m_assignment_dispatcher.dispatch(EventType::ASSIGNMENT, material_id, path, Status::NEW);
}

void OPENDCC_NAMESPACE::PrimMaterialOverride::material_resource_override(const PXR_NS::SdfPath& mat_path, const PrimMaterialDescriptor& descr)
{
    auto iter = m_mat_resource_overrides.find(mat_path);
    Status status;
    if (iter != m_mat_resource_overrides.end())
        status = Status::CHANGED;
    else
        status = Status::NEW;
    m_mat_resource_overrides[mat_path] = descr;
    m_material_resource_dispatcher.dispatch(EventType::MATERIAL_RESOURCE, mat_path, descr, status);
}

void OPENDCC_NAMESPACE::PrimMaterialOverride::clear_material_resource_override(const PXR_NS::SdfPath& mat_path)
{
    auto iter = m_mat_resource_overrides.find(mat_path);
    if (iter == m_mat_resource_overrides.end())
        return;
    const auto descr = iter->second;
    m_mat_resource_overrides.erase(mat_path);
    m_material_resource_dispatcher.dispatch(EventType::MATERIAL_RESOURCE, mat_path, descr, Status::REMOVED);
}

void PrimMaterialOverride::remove_material(size_t material_id)
{
    auto iter = m_materials.find(material_id);
    if (iter == m_materials.end())
        return;

    const auto descr = iter->second;
    m_materials.erase(iter);
    m_material_dispatcher.dispatch(EventType::MATERIAL, material_id, descr, Status::REMOVED);
}

void PrimMaterialOverride::clear_override(PXR_NS::SdfPath path)
{
    auto iter = m_assignments.find(path);
    if (iter == m_assignments.end())
        return;

    const auto mat = iter->second;
    m_assignments.erase(iter);
    m_assignment_dispatcher.dispatch(EventType::ASSIGNMENT, mat, path, Status::REMOVED);
}

void PrimMaterialOverride::clear_all()
{
    auto tmp_mats = m_materials;
    auto tmp_assignments = m_assignments;
    auto tmp_mat_resource_overrides = m_mat_resource_overrides;
    m_materials.clear();
    m_assignments.clear();
    m_mat_resource_overrides.clear();
    for (const auto& mat : tmp_mats)
        m_material_dispatcher.dispatch(EventType::MATERIAL, mat.first, mat.second, Status::REMOVED);
    for (const auto& assignment : tmp_assignments)
        m_assignment_dispatcher.dispatch(EventType::ASSIGNMENT, assignment.second, assignment.first, Status::REMOVED);
    for (const auto& mat_res : tmp_mat_resource_overrides)
        m_material_resource_dispatcher.dispatch(EventType::MATERIAL_RESOURCE, mat_res.first, mat_res.second, Status::REMOVED);
    m_materials.clear();
    m_assignments.clear();
    m_mat_resource_overrides.clear();
}

const std::unordered_map<size_t, PrimMaterialDescriptor>& PrimMaterialOverride::get_materials() const
{
    return m_materials;
}

const std::unordered_map<PXR_NS::SdfPath, size_t, PXR_NS::SdfPath::Hash>& PrimMaterialOverride::get_assignments() const
{
    return m_assignments;
}
const std::unordered_map<PXR_NS::SdfPath, OPENDCC_NAMESPACE::PrimMaterialDescriptor, PXR_NS::SdfPath::Hash>& OPENDCC_NAMESPACE::PrimMaterialOverride::
    get_material_resource_overrides() const
{
    return m_mat_resource_overrides;
}

OPENDCC_NAMESPACE_CLOSE
