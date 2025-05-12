/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/app/viewport/iviewport_tool_context.h"
#include <pxr/imaging/hgi/types.h>
#include <pxr/base/gf/vec2i.h>
#include <memory>
#include "opendcc/app/core/application.h"
#include <QPoint>
#include <thread>
#include <mutex>
#include <atomic>
#include <pxr/imaging/hio/image.h>
#include "OpenImageIO/imagebuf.h"
#include "OpenImageIO/typedesc.h"
#include "opendcc/base/commands_api/core/command.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec3i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/range2f.h>
#include <pxr/base/gf/rect2i.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/vt/array.h>
#include <pxr/usd/usdGeom/mesh.h>
#include "opendcc/base/commands_api/core/args.h"
#include "opendcc/usd_editor/texture_paint/texture_painter.h"

OPENDCC_NAMESPACE_OPEN

class PrimMaterialDescriptor;
class BrushProperties;

class TexturePaintToolContext : public IViewportToolContext
{
public:
    TexturePaintToolContext();
    virtual ~TexturePaintToolContext();
    virtual std::shared_ptr<PrimMaterialOverride> get_prim_material_override() override;
    virtual PXR_NS::TfToken get_name() const override;
    std::shared_ptr<BrushProperties> get_brush_properties() const;
    void set_texture_file(const std::string& texture);
    void enable_writing_to_file(bool enable);
    void set_occlude(bool occlude);
    const std::string& get_texture_file() const;
    void update_material();
    void bake_textures();

private:
    static unsigned int m_bake_textures_counter;

    void set_material();
    void reset();
    void push_command();
    bool init_paint_data(const ViewportViewPtr& viewport_view);

    int m_mouse_x = -1;
    int m_mouse_y = -1;
    std::shared_ptr<PrimMaterialOverride> m_material_override;
    std::unique_ptr<PrimMaterialDescriptor> m_material_descr;
    size_t m_material_id = -1;

    std::shared_ptr<TextureData> m_texture_data;
    std::unique_ptr<TexturePainter> m_texture_painter;
    std::shared_ptr<BrushProperties> m_brush_properties;
    PXR_NS::SdfPath m_painted_mesh_path;

    Application::CallbackHandle m_current_stage_changed;
    std::unique_ptr<StageObjectChangedWatcher> m_stage_changed;
    bool m_changing_radius = false;
    bool m_radius_editable = false;
    int m_start_radius = 20;
    QPoint m_radius_change_cursor_start_pos;
    bool m_writing_to_file = false;
    bool m_occlude = true;

public:
    virtual bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                ViewportUiDrawManager* draw_manager) override;
    virtual void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                               ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) override;
    virtual bool on_key_press(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    virtual bool on_key_release(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    static std::string settings_prefix();
};

OPENDCC_NAMESPACE_CLOSE
