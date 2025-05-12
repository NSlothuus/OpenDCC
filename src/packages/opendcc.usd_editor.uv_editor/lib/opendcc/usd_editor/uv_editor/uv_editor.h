/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/viewport/prim_material_override.h"
#include "opendcc/usd_editor/uv_editor/prim_info.h"

#include <pxr/imaging/hd/material.h>
#include <pxr/usd/sdf/path.h>

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;

OPENDCC_NAMESPACE_OPEN

class UVEditorGLWidget;
class ColorManagementWidget;

class UVEditor : public QWidget
{
    Q_OBJECT
public:
    UVEditor(QWidget* parent = nullptr);
    ~UVEditor() override;

private:
    void load_texture();
    void update_texture_names(int id);
    void on_selection_changed();
    void gather_textures_from_mat_overrides();

    void insert_material_override(const PXR_NS::SdfAssetPath& texture_path, size_t mat_id);
    void insert_mat_over_texture(const PXR_NS::HdMaterialNetworkMap& mat_resource, size_t mat_id);
    void remove_texture(size_t mat_id, const PXR_NS::HdMaterialNetworkMap& mat_resource);
    void update_textures(size_t mat_id, const PXR_NS::HdMaterialNetworkMap& mat_resource);

    void fill_prim_info(const PXR_NS::UsdGeomMesh& mesh);
    void collect_geometry(const PXR_NS::SdfPathVector& paths);

    UVEditorGLWidget* m_gl_widget = nullptr;
    QComboBox* m_custom_texture_cb = nullptr;
    QComboBox* m_uv_primvar_cb = nullptr;
    QComboBox* m_view_transform_cb = nullptr;
    QDoubleSpinBox* m_gamma_sb = nullptr;
    QDoubleSpinBox* m_exposure_sb = nullptr;
    int m_material_overrides_index = -1;

    Application::CallbackHandle m_selection_changed_cid;
    Application::CallbackHandle m_tool_changed_cid;
    PrimMaterialOverride::MaterialDispatcherHandle m_material_changed_cid;

    PrimInfoMap m_prims_info;
};

OPENDCC_NAMESPACE_CLOSE
