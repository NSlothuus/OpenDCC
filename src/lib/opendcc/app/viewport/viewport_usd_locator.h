/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/app/viewport/viewport_locator_data.h"

OPENDCC_NAMESPACE_OPEN

class ViewportLocatorDelegate;

class OPENDCC_API ViewportUsdLocator
{
public:
    ViewportUsdLocator(ViewportLocatorDelegate* scene_delegate, const PXR_NS::UsdPrim& prim);

    LocatorRenderDataPtr get_locator_item() const { return m_locator_item; };
    virtual void initialize(PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()) = 0;
    virtual void mark_dirty(PXR_NS::HdDirtyBits bits) = 0;
    virtual void update(PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()) = 0;
    virtual PXR_NS::SdfPath get_material_id() const { return PXR_NS::SdfPath(); }
#if PXR_VERSION >= 2002
    virtual PXR_NS::HdMaterialNetworkMap get_material_resource() const { return PXR_NS::HdMaterialNetworkMap(); }
#else
    virtual PXR_NS::HdMaterialParamVector get_material_params() const { return PXR_NS::HdMaterialParamVector(); }
    virtual std::string get_surface_shader_source() const { return std::string(); }
#endif
#if PXR_VERSION < 2108
    virtual PXR_NS::HdTextureResource::ID get_texture_resource_id(const PXR_NS::SdfPath& texture_id) const
    {
        return PXR_NS::HdTextureResource::ID();
    };
    virtual PXR_NS::HdTextureResourceSharedPtr get_texture_resource(const PXR_NS::SdfPath& texture_id) const
    {
        return PXR_NS::HdTextureResourceSharedPtr();
    };
#endif
protected:
    PXR_NS::SdfPath get_index_prim_path() const;
    PXR_NS::UsdPrim m_prim;
    LocatorRenderDataPtr m_locator_item;
    ViewportLocatorDelegate* m_scene_delegate = nullptr;
};

using ViewportUsdLocatorPtr = std::shared_ptr<ViewportUsdLocator>;

class OPENDCC_API ViewportUsdLightLocator : public ViewportUsdLocator
{
public:
    ViewportUsdLightLocator(ViewportLocatorDelegate* scene_delegate, const PXR_NS::UsdPrim& prim, const PXR_NS::TfToken& light_type);
    ~ViewportUsdLightLocator();
    virtual void initialize(PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()) override;
    virtual void mark_dirty(PXR_NS::HdDirtyBits bits) override;
    virtual PXR_NS::SdfPath get_material_id() const override;
#if PXR_VERSION >= 2002
    virtual PXR_NS::HdMaterialNetworkMap get_material_resource() const override;
#else
    std::string get_surface_shader_source() const override;
#endif
    virtual void update(PXR_NS::UsdTimeCode time) override;

private:
    PXR_NS::TfToken m_light_type;
};

class OPENDCC_API ViewportUsdGeometryLocator : public ViewportUsdLocator
{
public:
    ViewportUsdGeometryLocator(ViewportLocatorDelegate* scene_delegate, const PXR_NS::UsdPrim& prim);
    ~ViewportUsdGeometryLocator();
    virtual void initialize(PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()) override;
    virtual void mark_dirty(PXR_NS::HdDirtyBits bits) override;
    PXR_NS::SdfPath get_material_id() const override;
#if PXR_VERSION >= 2002
    virtual PXR_NS::HdMaterialNetworkMap get_material_resource() const override;
#else
    std::string get_surface_shader_source() const override;
#endif
};
OPENDCC_NAMESPACE_CLOSE
