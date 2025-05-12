/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/usd_editor/uv_editor/uv_tool.h"

#include <unordered_map>

#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec4f.h>

class QMouseEvent;

OPENDCC_NAMESPACE_OPEN

class UVEditorGLWidget;
class ViewportUiDrawManager;

class UvScaleManipulator
{
public:
    enum class Direction
    {
        Horizontal = 0,
        Vertical = 1,
        Free = 2,
        None = 3,
    };

    UvScaleManipulator(UvTool* tool);
    ~UvScaleManipulator();

    void on_mouse_press(QMouseEvent* event);
    void on_mouse_move(QMouseEvent* event);
    void on_mouse_release(QMouseEvent* event);

    void draw(ViewportUiDrawManager* manager);

    bool move_started() const;

    const PXR_NS::GfVec2f& get_delta() const;

    void set_pos(const PXR_NS::GfVec2f& pos);

private:
    void create_direction_handles();

    std::unordered_map<Direction, PXR_NS::GfVec4f> get_colors(const uint32_t hover_id) const;

    PXR_NS::GfVec2f screen_to_clip(const int x, const int y) const;
    PXR_NS::GfVec2f screen_to_clip(PXR_NS::GfVec2f pos) const;

    UvTool* m_tool = nullptr;

    Direction m_direction = Direction::None;

    std::unordered_map<Direction, uint32_t> m_direction_to_handle;
    std::unordered_map<uint32_t, Direction> m_handle_to_direction;

    PXR_NS::GfVec2f m_pos = PXR_NS::GfVec2f(0.0f, 0.0f);
    PXR_NS::GfVec2f m_prev_pos = PXR_NS::GfVec2f(0.0f, 0.0f);

    PXR_NS::GfVec2f m_click = PXR_NS::GfVec2f(0.0f, 0.0f);
    PXR_NS::GfVec2f m_delta = PXR_NS::GfVec2f(0.0f, 0.0f);
    PXR_NS::GfVec2f m_click_moved = PXR_NS::GfVec2f(0.0f, 0.0f);
};

OPENDCC_NAMESPACE_CLOSE
