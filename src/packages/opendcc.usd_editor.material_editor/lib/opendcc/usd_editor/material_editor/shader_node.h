/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/usd_editor/usd_node_editor/node.h"
#include "opendcc/usd_editor/material_editor/api.h"

class QGraphicsSvgItem;
OPENDCC_NAMESPACE_OPEN

extern const QColor s_fallback_port_color;
extern const std::unordered_map<PXR_NS::TfType, QColor, PXR_NS::TfHash> s_port_color;

class MaterialGraphModel;

class OPENDCC_MATERIAL_EDITOR_API TextureLayoutItem : public NamedPropertyLayoutItem
{
public:
    TextureLayoutItem(UsdGraphModel& model, UsdPrimNodeItemBase* node, const PortId& id, const PXR_NS::TfToken& name, Port::Type port_type,
                      ThumbnailCache* cache, const std::string& texture_path);

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /* = nullptr */) override;

protected:
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const override;

private:
    void read_image(ThumbnailCache* cache, const QString& path);

    std::string m_texture_path;
    QPixmap m_pixmap;
};

class OPENDCC_MATERIAL_EDITOR_API LiveShaderNodeItem : public UsdLiveNodeItem
{
public:
    LiveShaderNodeItem(UsdGraphModel& model, const PXR_NS::TfToken& name, const PXR_NS::TfToken& shader_id, const PXR_NS::SdfPath& parent_path,
                       QGraphicsItem* parent = nullptr);

protected:
    void on_prim_created(const PXR_NS::UsdPrim& prim) override;

private:
    PXR_NS::TfToken m_shader_id;
};

class OPENDCC_MATERIAL_EDITOR_API ShaderNodeItem : public UsdPrimNodeItemBase
{
public:
    ShaderNodeItem(MaterialGraphModel& model, const NodeId& node_id, const std::string& display_name, bool is_external);

    void enable_preview(bool enable);

    const MaterialGraphModel& get_model() const;
    MaterialGraphModel& get_model();

protected:
    virtual QVector<PropertyLayoutItem*> make_ports(const PXR_NS::UsdPrim& prim) override;
    virtual PropertyLayoutItem* make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim, int& position) override;
    virtual QString get_icon_path(const PXR_NS::UsdPrim& prim) const override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    std::string m_shader_type;
    QGraphicsSvgItem* m_preview_mat_button = nullptr;
    QGraphicsTextItem* m_shader_type_text = nullptr;
    bool m_enable_mat_preview = false;

public:
    virtual void update_port(const PortId& port_id) override;
    virtual void update_node() override;
};

OPENDCC_NAMESPACE_CLOSE
