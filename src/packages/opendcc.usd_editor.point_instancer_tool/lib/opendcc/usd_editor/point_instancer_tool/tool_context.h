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
#include <ImathVec.h>
#include "pxr/usd/usdGeom/pointInstancer.h"
#include "opendcc/app/core/mesh_bvh.h"
#include <random>

OPENDCC_NAMESPACE_OPEN

class PointInstancerToolContext : public IViewportToolContext
{
public:
    enum class Mode
    {
        Single = 0,
        Random = 1
    };

    struct Properties
    {
        int32_t current_proto_idx = 0;
        Imath::V3f scale = { 1, 1, 1 };
        Imath::V3f scale_randomness = { 0, 0, 0 };
        float vertical_offset = 0;
        float bend_randomness = 0;
        float rotation_min = 0;
        float rotation_max = 0;
        float density = 1;
        float radius = 1;
        float falloff = 0.3;
        bool rotate_to_normal = false;
        Mode mode = Mode::Random;
        void read_from_settings(const std::string& prefix);
        void write_to_settings(const std::string& prefix) const;
    };

    PointInstancerToolContext();
    virtual ~PointInstancerToolContext();

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

    void add_selected_items();

    PXR_NS::UsdGeomPointInstancer& instancer() { return m_instancer; }
    void set_on_instancer_changed_callback(std::function<void()> on_instancer_changed) { m_on_instancer_changed = on_instancer_changed; }
    Properties properties() const { return m_properties; }
    void set_properties(const Properties& properties);
    static std::string settings_prefix() { return "point_instancer.properties"; }

private:
    Properties m_properties;
    std::vector<Imath::V2f> generate_uv();
    void update_context();
    Imath::V3f main_direction() const;
    Imath::V3f compute_scale();
    Imath::V3f compute_point(const Imath::V3f& direction);
    PXR_NS::GfQuath compute_quat(const Imath::V3f& direction);

    void generate(PXR_NS::VtVec3fArray& new_points, PXR_NS::VtQuathArray& new_orientations, PXR_NS::VtVec3fArray& new_scales);

    static bool s_factory_registration;
    Application::CallbackHandle m_selection_event_hndl;
    PXR_NS::UsdGeomPointInstancer m_instancer;
    bool m_is_intersect = false;
    Imath::V3f m_p;
    Imath::V3f m_n;
    std::mt19937 m_rand_engine;
    std::unique_ptr<MeshBvh> m_bvh;
    PXR_NS::SdfPath m_geom_id;
    std::function<void()> m_on_instancer_changed = []() {
    };
    bool is_b_key_pressed = false;
    bool is_adjust_radius = false;
    float m_start_radius = 1;
    int m_start_x = 0;
    std::vector<Imath::V2f> m_generated_uv;
    static const uint32_t points_in_unit_radius = 50;

    QCursor* m_cursor = nullptr;
};

OPENDCC_NAMESPACE_CLOSE
