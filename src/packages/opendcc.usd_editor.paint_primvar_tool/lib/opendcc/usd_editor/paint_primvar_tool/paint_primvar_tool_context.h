/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
// workaround
#include "opendcc/base/qt_python.h"

#include "opendcc/app/core/application.h"
#include "opendcc/app/viewport/iviewport_tool_context.h"
#include <pxr/base/gf/vec3f.h>

OPENDCC_NAMESPACE_OPEN

struct MeshData;

enum class PrimvarType
{
    None,
    Float,
    Vec3f
};

class PaintPrimvarToolContext : public IViewportToolContext
{
public:
    enum class Mode
    {
        Set = 0,
        Add = 1,
        Smooth = 2,
    };

    struct Properties
    {
        PXR_NS::GfVec3f vec3f_value = PXR_NS::GfVec3f(1.0f);
        float float_value = 1.0f;
        float radius = 1;
        float falloff = 0.3;
        Mode mode = Mode::Set;
        void read_from_settings(const std::string& prefix);
        void write_to_settings(const std::string& prefix) const;
    };

    PaintPrimvarToolContext();
    virtual ~PaintPrimvarToolContext();

    virtual bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                               ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) override;
    virtual bool on_key_press(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    virtual bool on_key_release(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

    virtual QCursor* get_cursor() override { return m_cursor; }

    void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    virtual PXR_NS::TfToken get_name() const override;
    void set_on_mesh_changed_callback(std::function<void()> on_mesh_changed) { m_on_mesh_changed = on_mesh_changed; }

    Properties properties() const { return m_properties; }
    void set_properties(const Properties& properties);
    const std::vector<PXR_NS::TfToken>& primvars_names() const;
    void set_primvar_index(size_t idx);
    static std::string settings_prefix() { return "paint_primvar_tool_context.properties"; }
    bool empty() const { return m_mesh_data == nullptr; }
    PrimvarType get_primvar_type();
    virtual std::shared_ptr<PrimMaterialOverride> get_prim_material_override() override;

private:
    void draw_in_mesh();
    void update_context();
    PXR_NS::GfVec3f m_n;
    PXR_NS::GfVec3f m_p;
    Properties m_properties;
    bool is_b_key_pressed = false;
    bool is_adjust_radius = false;
    float m_start_radius = 1;
    int m_start_x = 0;
    size_t m_primvar_material_id = 0;
    std::shared_ptr<PrimMaterialOverride> m_prim_material_override;
    bool m_is_intersect;
    std::function<void()> m_on_mesh_changed = []() {
    };
    Application::CallbackHandle m_selection_event_hndl;
    std::unique_ptr<MeshData> m_mesh_data;

    QCursor* m_cursor;
};

OPENDCC_NAMESPACE_CLOSE
