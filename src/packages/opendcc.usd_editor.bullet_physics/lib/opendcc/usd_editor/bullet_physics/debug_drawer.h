/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "btBulletDynamicsCommon.h"
#include "opendcc/app/viewport/iviewport_draw_extension.h"
#include "pxr/base/gf/matrix4f.h"

OPENDCC_NAMESPACE_OPEN

class ViewportUiDrawManager;

class DebugDrawer
    : public btIDebugDraw
    , public IViewportDrawExtension
{
public:
    DebugDrawer();
    virtual ~DebugDrawer();
    virtual void draw(ViewportUiDrawManager* draw_manager, const PXR_NS::GfFrustum& frustum, int width, int height) override;
    virtual void setDebugMode(int debug_mode) override;
    virtual int getDebugMode() const override;
    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime,
                                  const btVector3& color) override;
    virtual void reportErrorWarning(const char* warningString) override;
    virtual void draw3dText(const btVector3& location, const char* textString) override;
    virtual DefaultColors getDefaultColors() const override;
    void set_enabled(bool enable);
    bool is_enabled() const { return m_enabled; };

private:
    bool m_enabled = false;
    int m_debug_mode = btIDebugDraw::DBG_NoDebug;
    PXR_NS::GfMatrix4f m_vp_matrix;
    ViewportUiDrawManager* m_draw_manager = nullptr;
};

OPENDCC_NAMESPACE_CLOSE
