/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/app/viewport/viewport_locator_data.h"

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API RectLightLocatorRenderData : public LocatorRenderData
{
public:
    RectLightLocatorRenderData();
    virtual void update(const std::unordered_map<std::string, PXR_NS::VtValue>& data) override;
    virtual const PXR_NS::VtArray<int>& vertex_per_curve() const override;
    virtual const PXR_NS::VtArray<int>& vertex_indexes() const override;
    virtual const PXR_NS::VtVec3fArray& vertex_positions() const override;
    virtual const PXR_NS::GfRange3d& bbox() const override;
    virtual const PXR_NS::TfToken& topology() const override;

private:
    float w = 1, h = 1;

    PXR_NS::GfRange3d m_bbox;
    PXR_NS::VtVec3fArray m_points;
    void update_points();
};

class OPENDCC_API DomeLightLocatorRenderData : public LocatorRenderData
{
public:
    DomeLightLocatorRenderData();
    virtual void update(const std::unordered_map<std::string, PXR_NS::VtValue>& data) override;
    virtual const PXR_NS::VtArray<int>& vertex_per_curve() const override;
    virtual const PXR_NS::VtArray<int>& vertex_indexes() const override;
    virtual const PXR_NS::VtVec3fArray& vertex_positions() const override;
    virtual const bool as_mesh() const override;
    virtual const PXR_NS::GfRange3d& bbox() const override;
    virtual const bool is_double_sided() const override;

private:
    bool m_is_double_sided = false;
};

class OPENDCC_API DirectLightLocatorData : public LocatorRenderData
{
public:
    DirectLightLocatorData();
    virtual void update(const std::unordered_map<std::string, PXR_NS::VtValue>& data) override;
    virtual const PXR_NS::VtArray<int>& vertex_per_curve() const override;
    virtual const PXR_NS::VtArray<int>& vertex_indexes() const override;
    virtual const PXR_NS::VtVec3fArray& vertex_positions() const override;
    virtual const PXR_NS::GfRange3d& bbox() const override;
    virtual const PXR_NS::TfToken& topology() const override;
};

class OPENDCC_API SphereLightLocatorRenderData : public LocatorRenderData
{
public:
    SphereLightLocatorRenderData();
    virtual void update(const std::unordered_map<std::string, PXR_NS::VtValue>& data) override;
    virtual const PXR_NS::VtArray<int>& vertex_per_curve() const override;
    virtual const PXR_NS::VtArray<int>& vertex_indexes() const override;
    virtual const PXR_NS::VtVec3fArray& vertex_positions() const override;
    virtual const bool as_mesh() const override;
    virtual const bool is_double_sided() const override;
    virtual const PXR_NS::GfRange3d& bbox() const override;

private:
    float m_radius = 0;

    PXR_NS::VtVec3fArray m_points;
    PXR_NS::GfRange3d m_bbox;

    void update_points();
};

class OPENDCC_API DiskLightLocatorRenderData : public LocatorRenderData
{
public:
    DiskLightLocatorRenderData();
    virtual void update(const std::unordered_map<std::string, PXR_NS::VtValue>& data) override;
    virtual const PXR_NS::VtArray<int>& vertex_per_curve() const override;
    virtual const PXR_NS::VtArray<int>& vertex_indexes() const override;
    virtual const PXR_NS::VtVec3fArray& vertex_positions() const override;
    virtual const bool as_mesh() const override;
    virtual const bool is_double_sided() const override;
    virtual const PXR_NS::GfRange3d& bbox() const override;

private:
    float m_radius = 0.5;

    PXR_NS::VtVec3fArray m_points;
    PXR_NS::GfRange3d m_bbox;

    void update_points();
};

class OPENDCC_API CylinderLightLocatorRenderData : public LocatorRenderData
{
public:
    CylinderLightLocatorRenderData();
    virtual void update(const std::unordered_map<std::string, PXR_NS::VtValue>& data) override;
    virtual const PXR_NS::VtArray<int>& vertex_per_curve() const override;
    virtual const PXR_NS::VtArray<int>& vertex_indexes() const override;
    virtual const PXR_NS::VtVec3fArray& vertex_positions() const override;
    virtual const bool as_mesh() const override;
    virtual const bool is_double_sided() const override;
    virtual const PXR_NS::GfRange3d& bbox() const override;

private:
    float m_radius = 1;
    float m_length = 2;

    PXR_NS::VtVec3fArray m_points;
    PXR_NS::GfRange3d m_bbox;

    void update_points();
};

class OPENDCC_API LightBlockerLocatorRenderData : public LocatorRenderData
{
public:
    LightBlockerLocatorRenderData() = default;

    virtual void update(const std::unordered_map<std::string, PXR_NS::VtValue>& data) override;
    virtual const PXR_NS::VtArray<int>& vertex_per_curve() const override;
    virtual const PXR_NS::VtArray<int>& vertex_indexes() const override;
    virtual const PXR_NS::VtVec3fArray& vertex_positions() const override;
    virtual const bool is_double_sided() const override;
    virtual const PXR_NS::GfRange3d& bbox() const override;
    virtual const PXR_NS::TfToken& topology() const override;

private:
    PXR_NS::TfToken m_type;
};

OPENDCC_NAMESPACE_CLOSE