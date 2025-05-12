/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/pxr.h>
#include <pxr/imaging/hd/sceneDelegate.h>
#include <pxr/imaging/hd/renderIndex.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/imaging/hd/meshTopology.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/imaging/hd/selection.h>
#include <pxr/imaging/hio/glslfx.h>
#if PXR_VERSION >= 2002
#include <pxr/imaging/hd/material.h>
#endif
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN

class ViewportLocatorDelegate;

class OPENDCC_API LocatorRenderData
{
public:
    virtual void update(const std::unordered_map<std::string, PXR_NS::VtValue>& prim) = 0;
    virtual const PXR_NS::VtArray<int>& vertex_per_curve() const = 0;
    virtual const PXR_NS::VtArray<int>& vertex_indexes() const = 0;
    virtual const PXR_NS::VtVec3fArray& vertex_positions() const = 0;
    virtual const PXR_NS::GfRange3d& bbox() const = 0;

    virtual const bool as_mesh() const { return false; };
    virtual const bool is_double_sided() const { return false; };
    virtual const PXR_NS::TfToken& topology() const;
    virtual ~LocatorRenderData() {}
};
typedef std::shared_ptr<LocatorRenderData> LocatorRenderDataPtr;

class OPENDCC_API ViewportLocatorData
{
public:
    ViewportLocatorData() = default;
    virtual ~ViewportLocatorData() = default;

    virtual LocatorRenderDataPtr create_locator(const PXR_NS::SdfPath& path, const PXR_NS::UsdTimeCode& time = PXR_NS::UsdTimeCode::Default()) = 0;
    virtual bool is_locator(const PXR_NS::SdfPath& path) const = 0;
    virtual void insert_light(const PXR_NS::SdfPath& path, const PXR_NS::UsdTimeCode& time = PXR_NS::UsdTimeCode::Default()) = 0;
    virtual void toggle_light(const PXR_NS::SdfPath& path, bool enable) = 0;
    virtual void remove_light(const PXR_NS::SdfPath& path) = 0;
    virtual void insert_locator(const PXR_NS::SdfPath& path, const PXR_NS::UsdTimeCode& time = PXR_NS::UsdTimeCode::Default()) = 0;
    virtual void remove_locator(const PXR_NS::SdfPath& path) = 0;
    virtual void mark_locator_dirty(const PXR_NS::SdfPath& path, PXR_NS::HdDirtyBits bits) = 0;
    virtual void toggle_all_lights(bool enable) = 0;
#if PXR_VERSION >= 2002
    virtual PXR_NS::HdMaterialNetworkMap get_material_resource(const PXR_NS::SdfPath& prim_path) const;
#else
    virtual std::string get_surface_shader_source(const PXR_NS::SdfPath& prim_path) const;
    virtual PXR_NS::HdMaterialParamVector get_material_params(const PXR_NS::SdfPath& material_path) const;
#endif
    virtual std::string get_texture_path(const PXR_NS::SdfPath& texture_path) const;
    virtual void update(const PXR_NS::SdfPath& path, const PXR_NS::UsdTimeCode& time = PXR_NS::UsdTimeCode::Default()) = 0;
    bool contains_material(const PXR_NS::SdfPath& path) const;
    bool contains_texture(const PXR_NS::SdfPath& path) const;
    bool contains_light(const PXR_NS::SdfPath& path) const;

protected:
    virtual std::unordered_map<std::string, PXR_NS::VtValue> get_data(const PXR_NS::SdfPath& path,
                                                                      const PXR_NS::UsdTimeCode& time = PXR_NS::UsdTimeCode::Default()) = 0;

    PXR_NS::HdSceneDelegate* m_delegate = nullptr;

    std::unordered_map<PXR_NS::SdfPath, PXR_NS::TfToken, PXR_NS::SdfPath::Hash> m_lights;
    std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash> m_materials;
    std::unordered_map<PXR_NS::SdfPath, std::string, PXR_NS::SdfPath::Hash> m_textures;
};
typedef std::shared_ptr<ViewportLocatorData> ViewportLocatorDataPtr;

OPENDCC_NAMESPACE_CLOSE