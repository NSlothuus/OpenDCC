// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <pxr/pxr.h>
#if PXR_VERSION < 2108
#include <pxr/imaging/glf/glew.h>
#else
#include <pxr/imaging/garch/glApi.h>
#endif
#include <pxr/imaging/cameraUtil/conformWindow.h>

#include "opendcc/usd_editor/uv_editor/uv_editor_gl_widget.h"
#include "opendcc/usd_editor/uv_editor/uv_scene_delegate.h"
#include "opendcc/usd_editor/uv_editor/uv_rotate_tool.h"
#include "opendcc/usd_editor/uv_editor/uv_scale_tool.h"
#include "opendcc/usd_editor/uv_editor/uv_move_tool.h"
#include "opendcc/usd_editor/uv_editor/uv_select_tool.h"

#include "opendcc/app/viewport/viewport_camera_mapper_factory.h"
#include "opendcc/app/viewport/viewport_camera_controller.h"
#include "opendcc/app/viewport/viewport_color_correction.h"
#include "opendcc/app/viewport/iviewport_tool_context.h"
#include "opendcc/app/viewport/viewport_engine_proxy.h"
#include "opendcc/app/viewport/viewport_grid.h"

#include "opendcc/app/ui/application_ui.h"

#include "opendcc/app/core/application.h"

#include <QGuiApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTimer>

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    template <class Container, class T>
    bool update_if_differs(Container& container, const std::string& key, const T& value)
    {
        auto iter = container.find(key);
        if (iter == container.end())
        {
            container[key] = value;
            return true;
        }
        if (iter->second.template Get<T>() != value)
        {
            iter->second = value;
            return true;
        }

        return false;
    }

    template <class T>
    T map_lookup(const std::unordered_map<std::string, VtValue>& container, const std::string& key, const T& default_value = T())
    {
        return TfMapLookupByValue(container, key, VtValue(default_value)).template Get<T>();
    }

    TfToken qstring_to_tiling_token(const QString& mode)
    {
        if (mode == "UDIM")
            return TfToken("udim");
        return TfToken("none");
    }
};

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget
//////////////////////////////////////////////////////////////////////////

UVEditorGLWidget::UVEditorGLWidget(QWidget* parent /*= nullptr*/)
    : QOpenGLWidget(parent)
{
    setProperty("unfocusedKeyEvent_enable", true);
    QSurfaceFormat surface_format;
    // surface_format.setSampleBuffers(true);
    surface_format.setSamples(4);

    surface_format.setProfile(QSurfaceFormat::CoreProfile);
    setMouseTracking(true);
    setFormat(surface_format);
    setFocusPolicy(Qt::StrongFocus);
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
    setAcceptDrops(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setContextMenuPolicy(Qt::ContextMenuPolicy::PreventContextMenu);

    m_time_changed_cid = Application::instance().register_event_callback(Application::EventType::CURRENT_TIME_CHANGED, [this] {
        m_engine_params.frame = Application::instance().get_current_time();
        update();
    });
    m_engine_params = {};
    m_engine_params.highlight = true;
    m_engine_params.frame = Application::instance().get_current_time();
    m_engine_params.current_stage_root = SdfPath::AbsoluteRootPath();
    m_engine_params.enable_scene_materials = true;
    m_engine_params.enable_lighting = true; // if lighting is off then Storm ignores shading nodes and applies base color
    m_engine_params.enable_sample_alpha_to_coverage = true;
    m_engine_params.depth_func = HdCompareFunction::HdCmpFuncLEqual;
    m_engine_params.color_correction_mode = TfToken(Application::instance().get_settings()->get("colormanagement.color_management", "openColorIO"));
    m_engine_params.input_color_space = Application::instance().get_settings()->get("colormanagement.ocio_rendering_space", "linear");
    m_engine_params.view_OCIO = Application::instance().get_settings()->get("colormanagement.ocio_view_transform", "sRGB");
    m_engine_params.user_data["uv.tiling_mode"] = TfToken("none");
    m_engine_params.user_data["uv.texture_file"] = std::string();
    m_engine_params.user_data["uv.show_texture"] = false;
    m_engine_params.user_data["uv.uv_primvar"] = TfToken();
    m_engine_params.user_data["uv.prim_paths"] = SdfPathVector();
    m_engine_params.user_data["uv.prims_info"] = PrimInfoMap();

    m_camera_controller = std::make_shared<ViewportCameraController>(ViewportCameraMapperFactory::create_camera_mapper(TfToken("UI")));
    m_camera_controller->frame_selection(GfRange3d(GfVec3d(0), GfVec3d(1)));

    m_tool = std::make_unique<UvSelectTool>(this);

    m_truck_cursor = QCursor(QPixmap(":/icons/cursor_track"), -12, -12);
    m_dolly_cursor = QCursor(QPixmap(":/icons/cursor_dolly"), -12, -12);
}

UVEditorGLWidget::~UVEditorGLWidget()
{
    Application::instance().unregister_event_callback(Application::EventType::SELECTION_CHANGED, m_selection_changed_cid);
    Application::instance().unregister_event_callback(Application::EventType::SELECTION_MODE_CHANGED, m_selection_mode_changed_cid);
    Application::instance().unregister_event_callback(Application::EventType::CURRENT_VIEWPORT_TOOL_CHANGED, m_current_viewport_tool_changed_cid);
    Application::instance().unregister_event_callback(Application::EventType::CURRENT_TIME_CHANGED, m_time_changed_cid);
    Application::instance().unregister_event_callback(Application::EventType::CURRENT_STAGE_CHANGED, m_current_stage_changed_cid);
    Application::instance().unregister_event_callback(Application::EventType::BEFORE_CURRENT_STAGE_CLOSED, m_current_stage_closed_cid);

    makeCurrent();

    m_tool.reset();
    m_grid.reset();
    m_draw_manager.reset();
    m_color_correction.reset();
    m_camera_controller.reset();
    m_engine.reset();

    doneCurrent();
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::tiling_mode
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::set_tiling_mode(const QString& tiling_mode)
{
    const auto new_tiling_mode = qstring_to_tiling_token(tiling_mode);
    if (update_if_differs(m_engine_params.user_data, "uv.tiling_mode", new_tiling_mode))
        update();
}

QString UVEditorGLWidget::get_tiling_mode() const
{
    return map_lookup<TfToken>(m_engine_params.user_data, "uv.tiling_mode").GetText();
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::background_texture
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::set_background_texture(const QString& texture_path)
{
    const auto new_texture_path = texture_path.toLocal8Bit().toStdString();
    if (update_if_differs(m_engine_params.user_data, "uv.texture_file", new_texture_path))
        update();
}

QString UVEditorGLWidget::get_background_texture() const
{
    return QString::fromStdString(map_lookup<std::string>(m_engine_params.user_data, "uv.texture_file"));
}

void UVEditorGLWidget::show_background_texture(bool show /*= true*/)
{
    if (update_if_differs(m_engine_params.user_data, "uv.show_texture", show))
        update();
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::uv_primvar
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::set_uv_primvar(const QString& uv_primvar)
{
    const auto primvar_token = TfToken(uv_primvar.toLocal8Bit().toStdString());
    if (update_if_differs(m_engine_params.user_data, "uv.uv_primvar", primvar_token))
        update();
}

QString UVEditorGLWidget::get_uv_primvar() const
{
    return map_lookup<TfToken>(m_engine_params.user_data, "uv.uv_primvar").GetText();
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::prim_paths
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::set_prim_paths(const SdfPathVector& prim_paths)
{
    if (update_if_differs(m_engine_params.user_data, "uv.prim_paths", prim_paths))
        update();
}

SdfPathVector UVEditorGLWidget::get_prim_paths() const
{
    return map_lookup<SdfPathVector>(m_engine_params.user_data, "uv.prim_paths");
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::gamma
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::set_gamma(float gamma)
{
    m_engine_params.gamma = gamma;
    m_color_correction->set_gamma(gamma);
    update();
}

float UVEditorGLWidget::get_gamma() const
{
    return m_engine_params.gamma;
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::exposure
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::set_exposure(float exposure)
{
    m_engine_params.exposure = exposure;
    m_color_correction->set_exposure(exposure);
    update();
}

float UVEditorGLWidget::get_exposure() const
{
    return m_engine_params.exposure;
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::view_transform
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::set_view_transform(const std::string& view_transform)
{
    if (view_transform == "None")
        m_color_correction->set_mode(ViewportColorCorrection::DISABLED);
    else
        m_color_correction->set_mode(ViewportColorCorrection::OCIO);
    m_engine_params.view_OCIO = view_transform;
    m_color_correction->set_ocio_view(view_transform);
    update();
}

std::string UVEditorGLWidget::get_view_transform() const
{
    return m_engine_params.view_OCIO;
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::prims_info
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::set_prims_info(const PrimInfoMap& prims_info)
{
    if (update_if_differs(m_engine_params.user_data, "uv.prims_info", prims_info))
    {
        update();
    }
}

PrimInfoMap UVEditorGLWidget::get_prims_info() const
{
    return map_lookup<PrimInfoMap>(m_engine_params.user_data, "uv.prims_info");
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::uv_selection
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::set_uv_selection(const SelectionList& selection, const RichSelection& rich_selection)
{
    m_uv_selection = selection;
    m_global_uv_selection.update(m_uv_selection);
    m_engine->set_selected(selection, rich_selection);
}

const SelectionList& UVEditorGLWidget::get_uv_selection() const
{
    return m_uv_selection;
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::ignore_next_selection_changed
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::ignore_next_selection_changed()
{
    ++m_ignore_selection_changed;
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::intersect
//////////////////////////////////////////////////////////////////////////

std::pair<PXR_NS::HdxPickHit, bool> UVEditorGLWidget::intersect(const PXR_NS::GfVec2f& point, SelectionList::SelectionMask pick_target)
{
    UsdStageRefPtr stage = Application::instance().get_session()->get_current_stage();
    if (!stage)
    {
        return { {}, false };
    }

    makeCurrent();

    ViewportHydraIntersectionParams pick_params;
    pick_params.engine_params = m_engine_params;
    pick_params.engine_params.enable_id_render = true;
    pick_params.engine_params.gamma_correct_colors = false;
    pick_params.engine_params.enable_sample_alpha_to_coverage = false;
    pick_params.pick_target = pick_target;
    pick_params.use_custom_render_tags = false;

    auto frustum = m_camera_controller->get_frustum();
    const auto width = this->width() * devicePixelRatio();
    const auto height = this->height() * devicePixelRatio();
    CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit, height != 0.0 ? static_cast<double>(width) / height : 1.0);

    pick_params.view_matrix = frustum.ComputeViewMatrix();
    pick_params.proj_matrix = frustum.ComputeProjectionMatrix();

    GfVec2f pick_point(point);
    GfVec2f start = pick_point - GfVec2f(2, -2);
    GfVec2f end = pick_point + GfVec2f(2, -2);
    start[1] = height - start[1];
    end[1] = height - end[1];

    const float select_rect_width = (end[0] - start[0]);
    const float select_rect_height = (end[1] - start[1]);

    GfMatrix4d selection_matrix;
    selection_matrix.SetIdentity();

    selection_matrix[0][0] = width / select_rect_width;
    selection_matrix[1][1] = height / select_rect_height;
    selection_matrix[3][0] = (width - (start[0] * 2 + select_rect_width)) / select_rect_width;
    selection_matrix[3][1] = (height - (start[1] * 2 + select_rect_height)) / select_rect_height;

    pick_params.proj_matrix *= selection_matrix;

    const auto mode = Application::instance().get_selection_mode();
    const auto points = mode == Application::SelectionMode::POINTS || mode == Application::SelectionMode::UV;
    const auto repr = HdReprSelector(HdReprTokens->refinedWireOnSurf, TfToken(), points ? HdReprTokens->points : TfToken());

    pick_params.use_custom_collection = true;
    pick_params.collection.SetForcedRepr(true);
    pick_params.collection.SetName(HdTokens->geometry);
    pick_params.collection.SetRootPath(SdfPath::AbsoluteRootPath());
    pick_params.collection.SetReprSelector(repr);
    pick_params.resolve_mode = HdxPickTokens->resolveNearestToCenter;

    HdxPickHitVector out;
    const auto result = m_engine->test_intersection_batch(pick_params, out);

    doneCurrent();
    if (result && !out.empty())
    {
        return { out[0], result };
    }

    return { HdxPickHit(), false };
}

std::pair<PXR_NS::HdxPickHitVector, bool> UVEditorGLWidget::intersect(const PXR_NS::GfVec2f& start, const PXR_NS::GfVec2f& end,
                                                                      SelectionList::SelectionMask pick_target)
{
    UsdStageRefPtr stage = Application::instance().get_session()->get_current_stage();
    if (!stage)
    {
        return { {}, false };
    }

    makeCurrent();

    ViewportHydraIntersectionParams pick_params;
    pick_params.engine_params = m_engine_params;
    pick_params.engine_params.enable_id_render = true;
    pick_params.engine_params.gamma_correct_colors = false;
    pick_params.engine_params.enable_sample_alpha_to_coverage = false;
    pick_params.pick_target = pick_target;
    pick_params.use_custom_render_tags = false;

    auto frustum = m_camera_controller->get_frustum();
    const auto width = this->width() * devicePixelRatio();
    const auto height = this->height() * devicePixelRatio();
    CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit, height != 0.0 ? static_cast<double>(width) / height : 1.0);

    pick_params.view_matrix = frustum.ComputeViewMatrix();
    pick_params.proj_matrix = frustum.ComputeProjectionMatrix();

    GfVec2f _start = { std::min(start[0], end[0]), std::max(start[1], end[1]) };
    GfVec2f _end = { std::max(start[0], end[0]), std::min(start[1], end[1]) };

    _start[1] = height - _start[1];
    _end[1] = height - _end[1];

    const float select_rect_width = _end[0] - _start[0];
    const float select_rect_height = _end[1] - _start[1];
    pick_params.resolution = GfVec2i(select_rect_width, select_rect_height);

    GfMatrix4d selection_matrix;
    selection_matrix.SetIdentity();

    selection_matrix[0][0] = width / select_rect_width;
    selection_matrix[1][1] = height / select_rect_height;
    selection_matrix[3][0] = (width - (_start[0] * 2 + select_rect_width)) / select_rect_width;
    selection_matrix[3][1] = (height - (_start[1] * 2 + select_rect_height)) / select_rect_height;

    pick_params.proj_matrix *= selection_matrix;

    const auto mode = Application::instance().get_selection_mode();
    const auto points = mode == Application::SelectionMode::POINTS || mode == Application::SelectionMode::UV;
    const auto repr = HdReprSelector(HdReprTokens->refinedWireOnSurf, TfToken(), points ? HdReprTokens->points : TfToken());

    pick_params.use_custom_collection = true;
    pick_params.collection.SetForcedRepr(true);
    pick_params.collection.SetName(HdTokens->geometry);
    pick_params.collection.SetRootPath(SdfPath::AbsoluteRootPath());
    pick_params.collection.SetReprSelector(repr);
    pick_params.resolve_mode = HdxPickTokens->resolveUnique;

    HdxPickHitVector out;

    const auto result = m_engine->test_intersection_batch(pick_params, out);

    doneCurrent();

    return { out, result };
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::pick_*_prim
//////////////////////////////////////////////////////////////////////////

SelectionList UVEditorGLWidget::pick_single_prim(const PXR_NS::GfVec2f& point, SelectionList::SelectionMask pick_target)
{
    auto result = intersect(point, pick_target);
    return result.second ? make_selection_list({ result.first }, pick_target) : SelectionList();
}

SelectionList UVEditorGLWidget::pick_multiple_prims(const PXR_NS::GfVec2f& start, const PXR_NS::GfVec2f& end,
                                                    SelectionList::SelectionMask pick_target)
{
    auto result = intersect(start, end, pick_target);
    return result.second ? make_selection_list(result.first, pick_target) : SelectionList();
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::set_color_mode(const std::string& color_mode)
{
    m_engine_params.color_correction_mode = TfToken(color_mode);
    if (color_mode == "openColorIO")
        m_color_correction->set_mode(ViewportColorCorrection::OCIO);
    else if (color_mode == "sRGB")
        m_color_correction->set_mode(ViewportColorCorrection::SRGB);
    else
        m_color_correction->set_mode(ViewportColorCorrection::DISABLED);
    update();
}

void UVEditorGLWidget::reload_current_texture()
{
    m_engine_params.user_data["uv.force_reload_texture"] = true;
    update();
}

ViewportUiDrawManager* UVEditorGLWidget::get_draw_manager()
{
    return m_draw_manager.get();
}

const ViewportUiDrawManager* UVEditorGLWidget::get_draw_manager() const
{
    return m_draw_manager.get();
}

const ViewportCameraController* UVEditorGLWidget::get_camera_controller() const
{
    return m_camera_controller.get();
}

ViewportCameraController* UVEditorGLWidget::get_camera_controller()
{
    return m_camera_controller.get();
}

void UVEditorGLWidget::update_range(const PXR_NS::SdfPath& path, const PXR_NS::VtVec2fArray& st)
{
    auto prims_info = get_prims_info();

    auto find = prims_info.find(path);
    if (find == prims_info.end())
    {
        return;
    }

    find->second.range = std::accumulate(st.begin(), st.end(), GfRange3d(), [](GfRange3d& range, const GfVec2f& st) {
        range.ExtendBy(GfVec3d(st[0], st[1], 0));
        return range;
    });

    set_prims_info(prims_info);
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::QOpenGLWidget
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::initializeGL()
{
#if PXR_VERSION < 2108
    GlfGlewInit();
#else
    GarchGLApiLoad();
#endif
    m_draw_manager = std::make_unique<ViewportUiDrawManager>(width() * devicePixelRatio(), height() * devicePixelRatio());

    std::unordered_set<TfType, TfHash> scene_delegates = { TfType::Find<UVSceneDelegate>() };
    m_grid = std::make_unique<ViewportGrid>(PXR_NS::GfVec4f(0.59462, 0.59462, 0.59462, 1), 1.0f, true, UsdGeomTokens->z);
    m_engine = std::make_unique<ViewportHydraEngine>(scene_delegates);
    m_engine->set_renderer_plugin(ViewportHydraEngine::get_default_render_plugin());
    m_selection_changed_cid = Application::instance().register_event_callback(Application::EventType::SELECTION_CHANGED, [this] {
        if (m_ignore_selection_changed)
        {
            --m_ignore_selection_changed;
            return;
        }

        auto selection = Application::instance().get_selection();
        auto uv_selection = mesh_to_uv(selection, get_prims_info());
        auto is_uv_mode = Application::instance().get_selection_mode() == Application::SelectionMode::UV;
        if (Application::instance().is_soft_selection_enabled())
        {
            auto rich_selection = Application::instance().get_rich_selection();
            rich_selection.set_soft_selection(uv_selection);
            is_uv_mode ? set_uv_selection(uv_selection, rich_selection) : m_engine->set_selected(selection, rich_selection);
        }
        else
        {
            is_uv_mode ? set_uv_selection(uv_selection) : m_engine->set_selected(selection);
        }

        update();
    });

    m_selection_mode_changed_cid = Application::instance().register_event_callback(Application::EventType::SELECTION_MODE_CHANGED, [this] {
        if (Application::instance().get_selection_mode() == Application::SelectionMode::UV)
        {
            const auto extract = m_global_uv_selection.extract(Application::instance().get_highlighted_prims());
            auto selection = uv_to_mesh(extract, get_prims_info());
            m_ignore_selection_changed += 2;
            Application::instance().set_selection(selection);
            m_engine->set_selected(extract);
            m_engine_params.point_color = { 100.0f / 255.0f, 54.0f / 255.0f, 38.0f / 255.0f, 1.0f };
            m_engine->set_selection_color({ 0.0f, 1.0f, 0.0f, 1.0f });
        }
        else
        {
            m_engine->set_selected(SelectionList());
            m_engine_params.point_color = { 0.0f, 0.0f, 0.0f, 1.0f };

            const auto color = Application::instance().get_settings()->get("viewport.selection_color", GfVec4f(1, 1, 0, 0.5));
            m_engine->set_selection_color(color);
        }
        update();
    });

    m_current_viewport_tool_changed_cid =
        Application::instance().register_event_callback(Application::EventType::CURRENT_VIEWPORT_TOOL_CHANGED, [this] {
            auto tool = ApplicationUI::instance().get_current_viewport_tool();
            if (!tool)
            {
                m_tool.reset();
            }
            else if (tool->get_name() == "select_tool")
            {
                if (Application::instance().get_selection_mode() == Application::SelectionMode::UV)
                {
                    set_uv_selection(m_prev_uv_selection);
                }

                m_tool = std::make_unique<UvSelectTool>(this);
            }
            else if (tool->get_name() == "rotate_tool")
            {
                if (Application::instance().get_selection_mode() == Application::SelectionMode::UV)
                {
                    set_uv_selection(m_prev_uv_selection);
                }

                m_tool = std::make_unique<UvRotateTool>(this);
            }
            else if (tool->get_name() == "move_tool")
            {
                if (Application::instance().get_selection_mode() == Application::SelectionMode::UV)
                {
                    set_uv_selection(m_prev_uv_selection);
                }

                m_tool = std::make_unique<UvMoveTool>(this);
            }
            else if (tool->get_name() == "scale_tool")
            {
                if (Application::instance().get_selection_mode() == Application::SelectionMode::UV)
                {
                    set_uv_selection(m_prev_uv_selection);
                }

                m_tool = std::make_unique<UvScaleTool>(this);
            }
        });

    m_current_stage_changed_cid = Application::instance().register_event_callback(Application::EventType::CURRENT_STAGE_CHANGED, [this]() {
        m_uv_selection.clear();
        m_global_uv_selection.clear();
    });

    m_current_stage_closed_cid = Application::instance().register_event_callback(Application::EventType::BEFORE_CURRENT_STAGE_CLOSED, [this]() {
        m_uv_selection.clear();
        m_global_uv_selection.clear();
    });

    GlfSimpleMaterial material;
    material.SetAmbient(GfVec4f(0.9, 0.9, 0.9, 1));
    material.SetSpecular(GfVec4f(0));
    material.SetShininess(0.0);
    m_engine->set_lighting_state({}, material, GfVec4f(0.81, 0.81, 0.81, 1.0));
    ViewportColorCorrection::ColorCorrectionMode mode;
    if (m_engine_params.color_correction_mode == "openColorIO")
        mode = ViewportColorCorrection::OCIO;
    else if (m_engine_params.color_correction_mode == "sRGB")
        mode = ViewportColorCorrection::SRGB;
    else
        mode = ViewportColorCorrection::DISABLED;
    m_color_correction = std::make_unique<ViewportColorCorrection>(mode, m_engine_params.view_OCIO, m_engine_params.input_color_space,
                                                                   m_engine_params.gamma, m_engine_params.exposure);
}

void UVEditorGLWidget::paintGL()
{
    const auto w = width();
    const auto h = height();
    auto clear_color = m_color_correction->get_mode() == ViewportColorCorrection::DISABLED ? GfVec3f(0.3) : GfVec3f(0.07);
    glClearColor(clear_color[0], clear_color[1], clear_color[2], 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, w, h);

    auto frustum = m_camera_controller->get_frustum();
    CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit, h != 0 ? w / (double)h : 1.0);

    m_grid->draw(frustum);

    const auto view = frustum.ComputeViewMatrix();
    const auto proj = frustum.ComputeProjectionMatrix();

    m_engine->set_camera_state(view, proj);
#if PXR_VERSION >= 2108
    CameraUtilFraming framing { GfRange2f(GfVec2f(0, 0), GfVec2f(w, h)), GfRect2i() };
    m_engine->set_framing(framing);
#else
    m_engine->set_render_viewport(GfVec4d(0, 0, w, h));
#endif
    m_engine_params.render_resolution = GfVec2i(w, h);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    m_engine_params.user_data["uv.render_mode"] = TfToken("hull");
    m_engine->update_init(m_engine_params);
    m_engine->update_delegates(m_engine_params);
    m_engine->render(m_engine_params);

    m_engine_params.user_data["uv.render_mode"] = TfToken("wire");
    m_engine->update_init(m_engine_params);
    m_engine->update_delegates(m_engine_params);
    m_engine->render(m_engine_params);

    if (m_tool)
    {
        m_tool->draw(m_draw_manager.get());
    }

    const auto view_f = GfMatrix4f(view);
    const auto proj_f = GfMatrix4f(proj);

    m_draw_manager->execute_draw_queue(w, h, m_mouse_x, m_mouse_y, proj_f, view_f);

    if (!m_engine->is_converged())
    {
        QTimer::singleShot(5, this, qOverload<>(&QOpenGLWidget::update));
    }

    m_color_correction->apply(w, h);
    m_engine_params.user_data["uv.force_reload_texture"] = false;
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::QWidget
//////////////////////////////////////////////////////////////////////////

void UVEditorGLWidget::mousePressEvent(QMouseEvent* event)
{
    m_mouse_mode = MouseMode::None;

    if (event->modifiers() & Qt::AltModifier)
    {
        const auto btn = event->button();
        if (btn == Qt::LeftButton || btn == Qt::MiddleButton)
        {
            m_mouse_mode = MouseMode::Truck;
            QGuiApplication::setOverrideCursor(m_truck_cursor);
        }
        else if (btn == Qt::RightButton)
        {
            m_mouse_mode = MouseMode::Zoom;
            QGuiApplication::setOverrideCursor(m_dolly_cursor);
        }
    }
    else if (m_mouse_mode != MouseMode::None || (m_tool && m_tool->on_mouse_press(event)))
    {
        update();
        return;
    }

    QOpenGLWidget::mousePressEvent(event);
}

void UVEditorGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    auto pos = event->pos();
    double dx = (pos.x() - m_mouse_x);
    double dy = (pos.y() - m_mouse_y);
    m_mouse_x = pos.x();
    m_mouse_y = pos.y();

    if (m_mouse_mode == MouseMode::Truck)
    {
        const auto px_to_world = m_camera_controller->compute_pixels_to_world_factor(height());
        m_camera_controller->truck(-dx * px_to_world, dy * px_to_world);
    }
    else if (m_mouse_mode == MouseMode::Zoom)
    {
        double zoom_delta = -0.002 * (dx + dy);
        m_camera_controller->adjust_distance(1 + zoom_delta);
    }

    if (m_mouse_mode != MouseMode::None || (m_tool && m_tool->on_mouse_move(event)))
    {
        update();
        return;
    }

    QOpenGLWidget::mouseMoveEvent(event);
}

void UVEditorGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_mouse_mode != MouseMode::None || (m_tool && m_tool->on_mouse_release(event)))
    {
        m_mouse_mode = MouseMode::None;
        QGuiApplication::restoreOverrideCursor();

        update();

        return;
    }

    QOpenGLWidget::mouseReleaseEvent(event);
}

void UVEditorGLWidget::wheelEvent(QWheelEvent* event)
{
    if (m_tool && m_tool->is_working())
    {
        return;
    }

    const auto zoom_delta = 1 - (double)event->angleDelta().y() / 1000;
    m_camera_controller->adjust_distance(zoom_delta);
    update();
}

void UVEditorGLWidget::keyPressEvent(QKeyEvent* event)
{
    if (m_tool && m_tool->is_working())
    {
        return;
    }

    if (event->key() == Qt::Key_F)
    {
        GfRange3d selection_range;
        for (const auto& selection : Application::instance().get_prim_selection())
            selection_range.ExtendBy(m_engine->get_bbox(selection));
        if (selection_range.IsEmpty())
            selection_range = m_engine->get_bbox(SdfPath::AbsoluteRootPath());
        m_camera_controller->frame_selection(selection_range.IsEmpty() ? GfRange3d(GfVec3d(0), GfVec3d(1)) : selection_range, 0.8f);
        update();
        return;
    }
    else if (event->key() == Qt::Key_B)
    {
        if (!event->isAutoRepeat())
        {
            m_key_press_timepoint = event->timestamp();
        }
    }

    QOpenGLWidget::keyPressEvent(event);
}

void UVEditorGLWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_B)
    {
        if (m_key_press_timepoint != static_cast<unsigned long long>(-1) && (event->timestamp() - m_key_press_timepoint) < 300)
        {
            auto enabled = Application::instance().is_soft_selection_enabled();
            Application::instance().enable_soft_selection(!enabled);
        }
    }
}

void UVEditorGLWidget::resizeEvent(QResizeEvent* e)
{
    m_camera_controller->set_display_size(e->size().width(), e->size().height());
    QOpenGLWidget::resizeEvent(e);
}

//////////////////////////////////////////////////////////////////////////
// UVEditorGLWidget::make_selection_list
//////////////////////////////////////////////////////////////////////////

SelectionList UVEditorGLWidget::make_selection_list(const PXR_NS::HdxPickHitVector& pick_hits, SelectionList::SelectionMask selection_mask) const
{
    using SelectionFlags = SelectionList::SelectionFlags;

    struct Data
    {
        std::vector<SelectionList::IndexType> points;
        std::vector<SelectionList::IndexType> edges;
        std::vector<SelectionList::IndexType> elements;
        std::vector<SelectionList::IndexType> instances;
        bool full = false;
    };
    std::map<SdfPath, Data> sel_data;
    auto stage = Application::instance().get_session()->get_current_stage();
    for (const auto& hit : pick_hits)
    {
        auto& val = sel_data[hit.objectId.ReplacePrefix(hit.delegateId, SdfPath::AbsoluteRootPath())];
        if ((selection_mask & SelectionFlags::POINTS) && hit.pointIndex >= 0 && hit.instancerId.IsEmpty())
            val.points.push_back(hit.pointIndex);
        if ((selection_mask & SelectionFlags::EDGES) && hit.edgeIndex >= 0 && hit.instancerId.IsEmpty())
            val.edges.push_back(hit.edgeIndex);
        if ((selection_mask & SelectionFlags::ELEMENTS) && hit.elementIndex >= 0 && hit.instancerId.IsEmpty())
            val.elements.push_back(hit.elementIndex);
        if ((selection_mask & SelectionFlags::INSTANCES) && hit.instanceIndex >= 0 && !hit.instancerId.IsEmpty())
        {
#if PXR_VERSION >= 2005
            HdInstancerContext instancer_context;
            auto real_path = m_engine->get_prim_path_from_instance_index(hit.objectId, hit.instanceIndex, &instancer_context);
            if (!instancer_context.empty())
            {
                sel_data[instancer_context[0].first].instances.push_back(instancer_context[0].second);
            }
            else
            {
                auto tmp = stage->GetPrimAtPath(real_path);
                while (!tmp.IsInstance())
                    tmp = tmp.GetParent();
                real_path = tmp.GetPrimPath();
                sel_data[real_path].instances.push_back(hit.instanceIndex);
            }
#else
            int global_id = -1;
            auto real_path = m_engine->get_prim_path_from_instance_index(hit.objectId, hit.instanceIndex, &global_id);
            sel_data[real_path].instances.push_back(
                { global_id == -1 ? static_cast<SelectionList::IndexType>(hit.instanceIndex) : static_cast<SelectionList::IndexType>(global_id) });
#endif
        }
        if (selection_mask & SelectionFlags::FULL_SELECTION)
        {
            SdfPath real_path;
#if PXR_VERSION >= 2005
            if (hit.instancerId.IsEmpty())
            {
                real_path = hit.objectId.ReplacePrefix(hit.delegateId, SdfPath::AbsoluteRootPath());
            }
            else
            {
                HdInstancerContext ctx;
                real_path = m_engine->get_prim_path_from_instance_index(hit.objectId, hit.instanceIndex, &ctx);
                if (!ctx.empty())
                {
                    real_path = ctx[0].first;
                }
                else
                {
                    auto tmp = stage->GetPrimAtPath(real_path);
                    while (tmp && !tmp.IsInstance())
                    {
                        tmp = tmp.GetParent();
                    }
                    real_path = tmp.GetPrimPath();
                }
            }
#else
            if (hit.instancerId.IsEmpty())
                real_path = hit.objectId.ReplacePrefix(hit.delegateId, SdfPath::AbsoluteRootPath());
            else
                real_path = m_engine->get_prim_path_from_instance_index(hit.objectId, hit.instanceIndex);
#endif
            sel_data[real_path].full = true;
        }
    }

    SelectionList list;
    for (const auto entry : sel_data)
    {
        SelectionData data(entry.second.full, entry.second.points, entry.second.edges, entry.second.elements, entry.second.instances);
        list.set_selection_data(entry.first, std::move(data));
    }

    return list;
}

OPENDCC_NAMESPACE_CLOSE
