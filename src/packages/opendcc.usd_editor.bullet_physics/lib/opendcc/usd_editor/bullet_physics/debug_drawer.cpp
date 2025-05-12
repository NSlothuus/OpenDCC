// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/bullet_physics/debug_drawer.h"
#include "opendcc/base/logging/logger.h"
#include "pxr/base/gf/matrix4f.h"
#include <pxr/imaging/cameraUtil/conformWindow.h>
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"
#include "opendcc/app/viewport/viewport_view.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"
#include "opendcc/app/viewport/viewport_ui_draw_manager.h"
#include "opendcc/app/ui/application_ui.h"
#include "opendcc/base/export.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include "session.h"

PXR_NAMESPACE_USING_DIRECTIVE

OPENDCC_NAMESPACE_OPEN

namespace
{
    GfMatrix4f get_vp_matrix(GfFrustum frustum, int width, int height)
    {
        GfVec4d viewport_resolution(0, 0, width, height);
        CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit,
                                viewport_resolution[3] != 0.0 ? viewport_resolution[2] / viewport_resolution[3] : 1.0);

        GfMatrix4d m = frustum.ComputeViewMatrix() * frustum.ComputeProjectionMatrix();

        return GfMatrix4f((float)m[0][0], (float)m[0][1], (float)m[0][2], (float)m[0][3], (float)m[1][0], (float)m[1][1], (float)m[1][2],
                          (float)m[1][3], (float)m[2][0], (float)m[2][1], (float)m[2][2], (float)m[2][3], (float)m[3][0], (float)m[3][1],
                          (float)m[3][2], (float)m[3][3]);
    }
}

struct DrawData
{
    ViewportUiDrawManager* draw_manager;
};

DebugDrawer::DebugDrawer()
{
    setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints);
}

DebugDrawer::~DebugDrawer() {}

void DebugDrawer::draw(ViewportUiDrawManager* draw_manager, const PXR_NS::GfFrustum& frustum, int width, int height)
{
    if (!m_enabled || !draw_manager)
        return;

    m_vp_matrix = get_vp_matrix(frustum, width, height);
    m_draw_manager = draw_manager;

    auto engin = BulletPhysicsSession::instance().current_engine();
    if (!engin)
        return;

    engin->set_debug_drawer(this);
    engin->draw_world();
    m_draw_manager = nullptr;
}

void DebugDrawer::setDebugMode(int debug_mode)
{
    m_debug_mode = debug_mode;
}

int DebugDrawer::getDebugMode() const
{
    return m_debug_mode;
}

void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    if (!m_draw_manager)
        return;
    m_draw_manager->begin_drawable();
    m_draw_manager->set_mvp_matrix(m_vp_matrix);
    m_draw_manager->set_color(GfVec3f(color[0], color[1], color[2]));
    m_draw_manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeLines);
    auto from_gf = GfVec3f(from[0], from[1], from[2]);
    auto to_gf = GfVec3f(to[0], to[1], to[2]);
    m_draw_manager->line(from_gf, to_gf);
    m_draw_manager->end_drawable();
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
    if (!m_draw_manager)
        return;
    m_draw_manager->begin_drawable();
    m_draw_manager->set_mvp_matrix(m_vp_matrix);
    m_draw_manager->set_color(GfVec3f(color[0], color[1], color[2]));
    m_draw_manager->set_point_size(5);
    std::vector<GfVec3f> points;
    points.push_back(GfVec3f(PointOnB[0], PointOnB[1], PointOnB[2]));
    m_draw_manager->mesh(ViewportUiDrawManager::PrimitiveTypePoints, points);
    m_draw_manager->end_drawable();
}

void DebugDrawer::reportErrorWarning(const char* warningString)
{
    OPENDCC_ERROR(warningString);
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString) {}

btIDebugDraw::DefaultColors DebugDrawer::getDefaultColors() const
{
    DefaultColors colors;
    colors.m_disabledDeactivationObject = btVector3(124.0 / 255, 207.0 / 255, 92.0 / 255);
    colors.m_contactPoint = btVector3(124.0 / 255, 207.0 / 255, 92.0 / 255);
    colors.m_deactivatedObject = btVector3(237.0 / 255, 141.0 / 255, 63.0 / 255);
    return colors;
}

void DebugDrawer::set_enabled(bool enable)
{
    m_enabled = enable;
}

OPENDCC_NAMESPACE_CLOSE
