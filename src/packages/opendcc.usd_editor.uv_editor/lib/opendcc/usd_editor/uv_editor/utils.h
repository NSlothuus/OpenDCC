/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include <pxr/base/gf/frustum.h>

#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/matrix4f.h>

#include <pxr/usd/usdGeom/primvarsAPI.h>

OPENDCC_NAMESPACE_OPEN

class ViewportUiDrawManager;

class DrawScope
{
public:
    DrawScope(ViewportUiDrawManager* manager, const uint32_t id = 0);
    ~DrawScope();

private:
    uint32_t m_id = 0;
    ViewportUiDrawManager* m_manager;
};

struct BaseDrawInfo
{
    BaseDrawInfo();

    PXR_NS::GfVec4f color;
    PXR_NS::GfMatrix4f mvp;
};

struct AxisInfo : public BaseDrawInfo
{
    PXR_NS::GfVec2f begin;
    PXR_NS::GfVec2f direction;

    float length;

    float width = 2.0f;
};
void draw_axis(ViewportUiDrawManager* manager, const AxisInfo& info, uint32_t id);

struct CircleInfo : public BaseDrawInfo
{
    PXR_NS::GfVec2f origin;
    PXR_NS::GfVec3f right;
    PXR_NS::GfVec3f up;

    int depth_priority = 0;

    float width = 1.0f;
};
void draw_circle(ViewportUiDrawManager* manager, const CircleInfo& info, uint32_t id);

struct PieInfo : public BaseDrawInfo
{
    PXR_NS::GfVec3f origin;
    PXR_NS::GfVec3f start;
    PXR_NS::GfVec3f end;
    PXR_NS::GfVec3f view;

    int depth_priority = 0;
    float width = 1.0f;
    float point_size = 1.0f;
    double radius = 1.0f;
    double angle = 0.0f;
};
void draw_pie(ViewportUiDrawManager* manager, const PieInfo& info, uint32_t id);

struct QuadInfo : public BaseDrawInfo
{
    PXR_NS::GfVec2f max;
    PXR_NS::GfVec2f min;

    int depth_priority = 0;

    bool outlined = true;
    float outlined_width = 1.0f;
    PXR_NS::GfVec4f outlined_color;
};
void draw_quad(ViewportUiDrawManager* manager, const QuadInfo& info, uint32_t id, float x_offset = 0, float y_offset = 0);

struct TriangleInfo : public BaseDrawInfo
{
    PXR_NS::GfVec2f a;
    PXR_NS::GfVec2f b;
    PXR_NS::GfVec2f c;
};
void draw_triangle(ViewportUiDrawManager* manager, const TriangleInfo& info, uint32_t id);

PXR_NS::GfVec2f screen_to_clip(const int x, const int y, const int width, const int height);
PXR_NS::GfVec2f screen_to_clip(const PXR_NS::GfVec2f pos, const int width, const int height);

PXR_NS::GfVec2f screen_to_world(const int x, const int y, const PXR_NS::GfFrustum& frustum, const int width, const int height);
PXR_NS::GfVec2f screen_to_world(PXR_NS::GfVec2f screen, const PXR_NS::GfFrustum& frustum, const int width, const int height);

PXR_NS::GfVec2f world_to_screen(const int x, const int y, const PXR_NS::GfFrustum& frustum, const int width, const int height);
PXR_NS::GfVec2f world_to_screen(PXR_NS::GfVec2f world, const PXR_NS::GfFrustum& frustum, const int width, const int height);

PXR_NS::UsdTimeCode get_non_varying_time(const PXR_NS::UsdGeomPrimvar& primvar);

OPENDCC_NAMESPACE_CLOSE
