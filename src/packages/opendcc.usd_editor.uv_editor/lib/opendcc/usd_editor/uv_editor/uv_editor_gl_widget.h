/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/base/qt_python.h"
#include "opendcc/usd_editor/uv_editor/uv_tool.h"
#include "opendcc/usd_editor/uv_editor/prim_info.h"
#include "opendcc/app/core/selection_list.h"
#include "opendcc/app/viewport/viewport_hydra_engine.h"
#include "opendcc/app/viewport/viewport_ui_draw_manager.h"

#include <QOpenGLWidget>
#include <QCursor>

OPENDCC_NAMESPACE_OPEN

class ViewportGrid;
class ViewportCameraController;
class ViewportColorCorrection;
class UvSelectionTool;

class UVEditorGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    UVEditorGLWidget(QWidget* parent = nullptr);
    ~UVEditorGLWidget();

    void set_tiling_mode(const QString& tiling_mode);
    QString get_tiling_mode() const;

    void set_background_texture(const QString& texture_path);
    QString get_background_texture() const;
    void show_background_texture(bool show = true);

    void set_uv_primvar(const QString& uv_primvar);
    QString get_uv_primvar() const;

    void set_prim_paths(const PXR_NS::SdfPathVector& prims);
    PXR_NS::SdfPathVector get_prim_paths() const;

    void set_gamma(float gamma);
    float get_gamma() const;

    void set_exposure(float exposure);
    float get_exposure() const;

    void set_view_transform(const std::string& view_transform);
    std::string get_view_transform() const;

    void set_prims_info(const PrimInfoMap& prims_info);
    PrimInfoMap get_prims_info() const;

    void set_uv_selection(const SelectionList& selection, const RichSelection& rich_selection = RichSelection());
    const SelectionList& get_uv_selection() const;

    void ignore_next_selection_changed();

    std::pair<PXR_NS::HdxPickHit, bool> intersect(const PXR_NS::GfVec2f& point, SelectionList::SelectionMask pick_target);
    std::pair<PXR_NS::HdxPickHitVector, bool> intersect(const PXR_NS::GfVec2f& start, const PXR_NS::GfVec2f& end,
                                                        SelectionList::SelectionMask pick_target);

    SelectionList pick_single_prim(const PXR_NS::GfVec2f& point, SelectionList::SelectionMask pick_target);
    SelectionList pick_multiple_prims(const PXR_NS::GfVec2f& start, const PXR_NS::GfVec2f& end, SelectionList::SelectionMask pick_target);

    void set_color_mode(const std::string& color_mode);
    void reload_current_texture();

    ViewportUiDrawManager* get_draw_manager();
    const ViewportUiDrawManager* get_draw_manager() const;

    const ViewportCameraController* get_camera_controller() const;
    ViewportCameraController* get_camera_controller();

    void update_range(const PXR_NS::SdfPath& path, const PXR_NS::VtVec2fArray& st);

protected:
    void initializeGL() override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    void wheelEvent(QWheelEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void resizeEvent(QResizeEvent* event) override;

private:
    SelectionList make_selection_list(const PXR_NS::HdxPickHitVector& pick_hits, SelectionList::SelectionMask selection_mask) const;

    enum class MouseMode
    {
        None,
        Zoom,
        Truck
    };
    MouseMode m_mouse_mode;

    int m_mouse_x = 0;
    int m_mouse_y = 0;

    std::shared_ptr<ViewportCameraController> m_camera_controller;
    std::unique_ptr<ViewportHydraEngine> m_engine;
    std::unique_ptr<ViewportGrid> m_grid;
    std::unique_ptr<ViewportColorCorrection> m_color_correction;
    std::unique_ptr<ViewportUiDrawManager> m_draw_manager;
    std::unique_ptr<UvTool> m_tool;

    ViewportHydraEngineParams m_engine_params;

    Application::CallbackHandle m_selection_changed_cid;
    Application::CallbackHandle m_selection_mode_changed_cid;
    Application::CallbackHandle m_current_viewport_tool_changed_cid;
    Application::CallbackHandle m_time_changed_cid;
    Application::CallbackHandle m_current_stage_changed_cid;
    Application::CallbackHandle m_current_stage_closed_cid;

    int m_ignore_selection_changed = 0;
    unsigned long long m_key_press_timepoint;

    SelectionList m_uv_selection;
    SelectionList m_global_uv_selection;
    SelectionList m_prev_uv_selection;

    QCursor m_truck_cursor;
    QCursor m_dolly_cursor;
};

OPENDCC_NAMESPACE_CLOSE
