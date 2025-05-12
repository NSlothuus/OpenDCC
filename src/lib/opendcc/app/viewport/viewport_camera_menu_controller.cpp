// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_camera_menu_controller.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"
#include "opendcc/app/viewport/viewport_camera_controller.h"
#include <QMenu>
#include <QActionGroup>
#include "opendcc/app/core/application.h"
#include "opendcc/app/ui/application_ui.h"
#include <pxr/usd/usdGeom/camera.h>
#include "pxr/usd/usd/primRange.h"
#include "opendcc/app/viewport/viewport_overlay.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

ViewportUsdCameraMenuController::ViewportUsdCameraMenuController(ViewportCameraControllerPtr camera_controller, ViewportOverlayWidget* overlay,
                                                                 QObject* parent /*= nullptr*/)
    : ViewportCameraMenuController(camera_controller, parent)
{
    m_camera_menu = new QMenu(i18n("viewport.menu_bar.view", "Camera"));
    m_overlay = overlay;

    m_camera_select_group = new QActionGroup(this);
    m_camera_select_group->setExclusive(true);

    m_look_through_action = new QAction(i18n("viewport.menu_bar.view", "Look Through Selected"), this);
    connect(m_look_through_action, &QAction::triggered, this, &ViewportUsdCameraMenuController::on_look_through);

    if (TF_VERIFY(m_camera_controller, "Failed to initialize viewport camera menu controller. GLWidget is null."))
    {
        connect(m_camera_controller.get(), &ViewportCameraController::camera_changed, this, &ViewportUsdCameraMenuController::on_camera_changed);
    }

    m_current_stage_object_changed_cid = Application::instance().get_session()->register_stage_changed_callback(
        Session::StageChangedEventType::CURRENT_STAGE_OBJECT_CHANGED,
        std::bind(&ViewportUsdCameraMenuController::on_current_stage_object_changed, this, std::placeholders::_1));
    on_rebuild_ui();
}

ViewportUsdCameraMenuController::~ViewportUsdCameraMenuController()
{
    Application::instance().get_session()->unregister_stage_changed_callback(Session::StageChangedEventType::CURRENT_STAGE_OBJECT_CHANGED,
                                                                             m_current_stage_object_changed_cid);
}

void ViewportUsdCameraMenuController::on_current_stage_object_changed(const UsdNotice::ObjectsChanged& notice)
{
    auto stage = Application::instance().get_session()->get_current_stage();

    auto has_camera_changes = false;
    for (const auto& camera : m_camera_actions)
    {
        if (notice.ResyncedObject(camera.first))
        {
            has_camera_changes = true;
            break;
        }
    }
    if (!has_camera_changes)
    {
        for (const auto& resync_path : notice.GetResyncedPaths())
        {
            auto prim = stage->GetPrimAtPath(resync_path.GetPrimPath());
            if (!prim)
                continue;

            if (UsdGeomCamera(prim))
            {
                has_camera_changes = true;
                break;
            }

            const auto descendants = prim.GetAllDescendants();
            auto camera_prim =
                std::find_if(descendants.begin(), descendants.end(), [](const UsdPrim& descendant) { return UsdGeomCamera(descendant); });

            if (camera_prim != descendants.end())
            {
                has_camera_changes = true;
                break;
            }
        }
    }
    if (has_camera_changes)
        on_rebuild_ui();
}

void ViewportUsdCameraMenuController::on_look_through(bool)
{
    if (auto stage = Application::instance().get_session()->get_current_stage())
    {
        auto selection = Application::instance().get_prim_selection();
        if (!selection.empty())
        {
            auto prim = stage->GetPrimAtPath(selection[0]);
            if (prim.IsValid() && prim.IsA<UsdGeomXformable>())
            {
                m_camera_controller->set_follow_prim(prim.GetPath());
                if (!prim.IsA<UsdGeomCamera>())
                {
                    m_look_through_prim = prim.GetPath();
                    on_rebuild_ui();
                }
            }
        }
    }
}

void ViewportUsdCameraMenuController::on_rebuild_ui()
{
    m_camera_actions.clear();
    m_camera_menu->clear();
    for (auto action : m_camera_select_group->actions())
        action->deleteLater();
    m_def_cam_action = new QAction(i18n("viewport.camera", "Def Cam"), this);
    m_def_cam_action->setCheckable(true);
    m_def_cam_action->setData(QString::fromStdString(""));
    m_camera_select_group->addAction(m_def_cam_action);
    m_camera_menu->addAction(m_def_cam_action);
    m_overlay->add_camera(m_def_cam_action);

    const auto is_scene_cam_mode = m_camera_controller->get_follow_mode() != ViewportCameraController::FollowMode::DefCam;
    m_def_cam_action->setChecked(!is_scene_cam_mode);

    auto stage = Application::instance().get_session()->get_current_stage();
    if (!stage)
        return;

    if (!m_look_through_prim.IsEmpty())
    {
        auto look_through_prim = stage->GetPrimAtPath(m_look_through_prim);
        if (look_through_prim)
        {
            auto look_through_prim_action = new QAction(m_look_through_prim.GetName().c_str(), this);
            look_through_prim_action->setData(QString::fromStdString(m_look_through_prim.GetString()));
            look_through_prim_action->setCheckable(true);
            m_camera_actions[look_through_prim] = look_through_prim_action;
            m_camera_menu->addAction(look_through_prim_action);
            m_overlay->add_camera(look_through_prim_action);
            m_camera_select_group->addAction(look_through_prim_action);
            look_through_prim_action->setChecked(m_camera_controller->get_follow_mode() == ViewportCameraController::FollowMode::StageXformablePrim);
        }
        else
        {
            m_look_through_prim = SdfPath::EmptyPath();
        }
    }
    bool have_scene_cameras = false;
    for (UsdPrim prim : stage->TraverseAll())
    {
        if (!prim.IsA<UsdGeomCamera>())
            continue;

        if (!have_scene_cameras)
        {
            have_scene_cameras = true;
            m_camera_menu->addSection("Scene Cam");
        }

        auto scene_cam_action = new QAction(prim.GetName().GetText(), this);
        scene_cam_action->setData(QString::fromStdString(prim.GetPath().GetString()));
        scene_cam_action->setCheckable(true);

        m_camera_actions[prim] = scene_cam_action;
        m_camera_select_group->addAction(scene_cam_action);
        m_camera_menu->addAction(scene_cam_action);
        m_overlay->add_camera(scene_cam_action);
    }
    on_camera_changed(m_camera_controller->get_follow_prim_path());
    connect(m_camera_select_group, &QActionGroup::triggered, this, [this](QAction* action) {
        const auto& camera_path_str = action->data().toString().toStdString();
        camera_path_str.empty() ? m_camera_controller->set_default_camera() : m_camera_controller->set_follow_prim(SdfPath(camera_path_str));
    });
}

void ViewportUsdCameraMenuController::on_camera_changed(PXR_NS::SdfPath follow_path)
{
    if (!m_look_through_prim.IsEmpty() && m_look_through_prim != follow_path)
    {
        m_look_through_prim = SdfPath::EmptyPath();
        on_rebuild_ui();
        return;
    }
    if (follow_path.IsEmpty())
    {
        m_def_cam_action->setChecked(true);
        return;
    }

    auto stage = Application::instance().get_session()->get_current_stage();
    auto prim = stage->GetPrimAtPath(follow_path);
    auto iter = m_camera_actions.find(prim);
    if (iter != m_camera_actions.end())
        iter->second->setChecked(true);
    else if (auto checked_action = m_camera_select_group->checkedAction())
        checked_action->setChecked(false);
}

OPENDCC_NAMESPACE_CLOSE
