// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/material_editor/shader_node.h"
#include "opendcc/usd_editor/material_editor/model.h"
#include "opendcc/usd_editor/material_editor/model.h"
#include "opendcc/ui/node_editor/connection.h"
#include "opendcc/ui/node_editor/thumbnail_cache.h"
#include "opendcc/app/ui/shader_node_registry.h"
#include "opendcc/app/viewport/prim_material_override.h"
#include "opendcc/app/ui/application_ui.h"
#include "usd_fallback_proxy/core/usd_prim_fallback_proxy.h"
#include <pxr/usd/ar/resolver.h>
#include <pxr/base/tf/stringUtils.h>
#include <pxr/usd/usdUI/tokens.h>
#include <pxr/usd/usdShade/shader.h>
#include <pxr/usd/sdr/shaderProperty.h>
#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QGraphicsSceneEvent>
#include <QGraphicsItem>
#include <QGraphicsSvgItem>
#include <QSvgRenderer>
#include "usdUIExt/nodeDisplayGroupUIAPI.h"

PXR_NAMESPACE_USING_DIRECTIVE
OPENDCC_NAMESPACE_OPEN

const QColor s_fallback_port_color = QColor(179, 179, 179);
const std::unordered_map<TfType, QColor, TfHash> s_port_color = {
    { SdfValueTypeNames->Asset.GetType(), QColor(40, 48, 76) },     { SdfValueTypeNames->AssetArray.GetType(), QColor(40, 48, 76) },
    { SdfValueTypeNames->Bool.GetType(), QColor(73, 113, 147) },    { SdfValueTypeNames->BoolArray.GetType(), QColor(73, 113, 147) },
    { SdfValueTypeNames->Int.GetType(), QColor(15, 120, 130) },     { SdfValueTypeNames->IntArray.GetType(), QColor(15, 120, 130) },
    { SdfValueTypeNames->Int2.GetType(), QColor(43, 70, 99) },      { SdfValueTypeNames->Int2Array.GetType(), QColor(43, 70, 99) },
    { SdfValueTypeNames->Int3.GetType(), QColor(36, 4, 124) },      { SdfValueTypeNames->Int3Array.GetType(), QColor(36, 4, 124) },
    { SdfValueTypeNames->Int4.GetType(), QColor(103, 53, 147) },    { SdfValueTypeNames->Int4Array.GetType(), QColor(103, 53, 147) },
    { SdfValueTypeNames->Half.GetType(), QColor(68, 234, 129) },    { SdfValueTypeNames->HalfArray.GetType(), QColor(68, 234, 129) },
    { SdfValueTypeNames->Half2.GetType(), QColor(76, 53, 56) },     { SdfValueTypeNames->Half2Array.GetType(), QColor(76, 53, 56) },
    { SdfValueTypeNames->Half3.GetType(), QColor(191, 95, 164) },   { SdfValueTypeNames->Half3Array.GetType(), QColor(191, 95, 164) },
    { SdfValueTypeNames->Half4.GetType(), QColor(246, 247, 220) },  { SdfValueTypeNames->Half4Array.GetType(), QColor(246, 247, 220) },
    { SdfValueTypeNames->Float.GetType(), QColor(140, 105, 126) },  { SdfValueTypeNames->FloatArray.GetType(), QColor(140, 105, 126) },
    { SdfValueTypeNames->Float2.GetType(), QColor(181, 101, 115) }, { SdfValueTypeNames->Float2Array.GetType(), QColor(181, 101, 115) },
    { SdfValueTypeNames->Float3.GetType(), QColor(153, 147, 234) }, { SdfValueTypeNames->Float3Array.GetType(), QColor(153, 147, 234) },
    { SdfValueTypeNames->Float4.GetType(), QColor(214, 151, 102) }, { SdfValueTypeNames->Float4Array.GetType(), QColor(214, 151, 102) },
    { SdfValueTypeNames->Double.GetType(), QColor(89, 48, 81) },    { SdfValueTypeNames->DoubleArray.GetType(), QColor(89, 48, 81) },
    { SdfValueTypeNames->Double2.GetType(), QColor(109, 114, 42) }, { SdfValueTypeNames->Double2Array.GetType(), QColor(109, 114, 42) },
    { SdfValueTypeNames->Double3.GetType(), QColor(35, 83, 109) },  { SdfValueTypeNames->Double3Array.GetType(), QColor(35, 83, 109) },
    { SdfValueTypeNames->Double4.GetType(), QColor(91, 58, 135) },  { SdfValueTypeNames->Double4Array.GetType(), QColor(91, 58, 135) },
    { SdfValueTypeNames->Token.GetType(), QColor(179, 179, 179) },  { SdfValueTypeNames->TokenArray.GetType(), QColor(179, 179, 179) },
    { SdfValueTypeNames->String.GetType(), QColor(64, 224, 208) },  { SdfValueTypeNames->StringArray.GetType(), QColor(64, 224, 208) }
};

namespace
{
    bool is_texture_attribute(const std::string& shader_type, const PXR_NS::TfToken& name)
    {
        if (shader_type == "cycles:image_texture" && name == "inputs:filename")
            return true;
        else if (shader_type == "arnold:image" && name == "inputs:filename")
            return true;
        else if (shader_type == "PxrTexture" && name == "inputs:filename")
            return true;
        else if (shader_type == "UsdUVTexture" && name == "inputs:file")
            return true;
        return false;
    };

    static constexpr qreal s_texture_size = 100;
};

LiveShaderNodeItem::LiveShaderNodeItem(UsdGraphModel& model, const PXR_NS::TfToken& name, const PXR_NS::TfToken& shader_id,
                                       const PXR_NS::SdfPath& parent_path, QGraphicsItem* parent /*= nullptr*/)
    : UsdLiveNodeItem(model, name, TfToken("Shader"), parent_path, parent)
    , m_shader_id(shader_id)
{
}

void LiveShaderNodeItem::on_prim_created(const PXR_NS::UsdPrim& prim)
{
    auto shader = UsdShadeShader(prim);
    if (!shader)
        return;

    shader.SetShaderId(m_shader_id);
}

TextureLayoutItem::TextureLayoutItem(UsdGraphModel& model, UsdPrimNodeItemBase* node, const PortId& id, const PXR_NS::TfToken& name,
                                     Port::Type port_type, ThumbnailCache* cache, const std::string& texture_path)
    : NamedPropertyLayoutItem(model, node, id, name, port_type)
    , m_texture_path(texture_path)
{
    const auto img_path = QString::fromStdString(texture_path);
    if (cache->has_image(img_path))
    {
        read_image(cache, img_path);
    }
    else
    {
        connect(cache, &ThumbnailCache::image_read, this, [this, cache, source_img = img_path](const QString& image_path) {
            if (source_img == image_path)
            {
                read_image(cache, image_path);
            }
        });
        cache->read_image_async(img_path);
    }
}

void TextureLayoutItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /* = nullptr */)
{
    NamedPropertyLayoutItem::paint(painter, option, widget);
    painter->save();
    const auto rect = boundingRect();
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(QPointF(rect.center().x() - s_texture_size / 2, 14), m_pixmap);

    painter->restore();
}

QSizeF TextureLayoutItem::sizeHint(Qt::SizeHint which, const QSizeF& constraint /*= QSizeF()*/) const
{
    auto result = NamedPropertyLayoutItem::sizeHint(which, constraint);
    if (m_pixmap.isNull())
        return result;
    return QSizeF(result.width(), 14 + s_texture_size);
}

void TextureLayoutItem::read_image(ThumbnailCache* cache, const QString& path)
{
    assert(cache);
    if (auto img = cache->read_image(path))
    {
        m_pixmap = QPixmap::fromImage(*img).scaled(QSize(s_texture_size, s_texture_size), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        updateGeometry();
        get_node_item()->invalidate_layout();
    }
}

ShaderNodeItem::ShaderNodeItem(MaterialGraphModel& model, const NodeId& node_id, const std::string& display_name, bool is_external)
    : UsdPrimNodeItemBase(model, node_id, display_name, Orientation::Horizontal, true, is_external)
{
    UsdUIExtNodeDisplayGroupUIAPI::Apply(model.get_prim_for_node(node_id));
}

void ShaderNodeItem::enable_preview(bool enable)
{
    if (m_enable_mat_preview == enable || !m_preview_mat_button)
        return;

    m_enable_mat_preview = enable;

    if (m_enable_mat_preview)
        m_preview_mat_button->renderer()->load(QString(":/icons/node_editor/shader_preview_active"));
    else
        m_preview_mat_button->renderer()->load(QString(":/icons/node_editor/shader_preview"));
}

MaterialGraphModel& ShaderNodeItem::get_model()
{
    return static_cast<MaterialGraphModel&>(UsdPrimNodeItemBase::get_model());
}

const MaterialGraphModel& ShaderNodeItem::get_model() const
{
    return static_cast<const MaterialGraphModel&>(UsdPrimNodeItemBase::get_model());
}

QVector<PropertyLayoutItem*> ShaderNodeItem::make_ports(const PXR_NS::UsdPrim& prim)
{
    UsdPrimFallbackProxy proxy(prim);
    std::vector<UsdPropertyProxyPtr> inputs;
    std::vector<UsdPropertyProxyPtr> outputs;
    auto& prop_groups = get_prop_groups();
    for (const auto prop : proxy.get_all_property_proxies())
    {
        const auto metadata = prop->get_all_metadata();
        const auto conectability = metadata.find(SdrPropertyMetadata->Connectable);
        if (conectability != metadata.end() && conectability->second == VtValue(false))
            continue;
        if (TfStringStartsWith(prop->get_name_token(), "inputs:"))
        {
            inputs.push_back(prop);
            auto group = prop->get_display_group();

            if (!group.empty() && prop_groups[group] == nullptr)
            {
                prop_groups[group] = new PropertyGroupItem(this, QString::fromStdString(group));
            }
        }
        else if (TfStringStartsWith(prop->get_name_token(), "outputs:"))
            outputs.push_back(prop);
    }

    QVector<PropertyLayoutItem*> result;
    result.reserve(inputs.size() + outputs.size());
    for (const auto col : { &outputs, &inputs })
    {
        for (const auto prop : *col)
        {
            const auto name = prop->get_name_token();
            const auto prop_path = prim.GetPath().AppendProperty(name);
            std::vector<ConnectionItem*> connections;
            for (auto connection : get_prop_connections())
            {
                if (connection->get_id().start_port == prop_path.GetString() || connection->get_id().end_port == prop_path.GetString())
                    connections.push_back(connection);
            }

            const auto stripped_name = TfToken(name.data() + name.GetString().find(SdfPathTokens->namespaceDelimiter) + 1);
            PropertyWithPortsLayoutItem* item = nullptr;
            if (is_texture_attribute(m_shader_type, name))
            {
                VtValue val;
                std::string file_path;
                if (prop->get(&val))
                {
                    if (val.IsHolding<SdfAssetPath>())
#if PXR_VERSION < 2205
                        file_path = ArGetResolver().Resolve(val.UncheckedGet<SdfAssetPath>().GetAssetPath());
#else
                        file_path = ArGetResolver().Resolve(val.UncheckedGet<SdfAssetPath>().GetAssetPath()).GetPathString();
#endif
                    else if (val.IsHolding<std::string>())
                        file_path = val.UncheckedGet<std::string>();
                }
                item = new TextureLayoutItem(get_model(), this, prop_path.GetString(), stripped_name,
                                             col == &outputs ? Port::Type::Output : Port::Type::Input, get_scene()->get_thumbnail_cache(), file_path);
            }
            else
            {
                item = new NamedPropertyLayoutItem(get_model(), this, prop_path.GetString(), stripped_name,
                                                   col == &outputs ? Port::Type::Output : Port::Type::Input);
            }

            for (auto connection : connections)
                item->add_connection(connection);

            auto color_it = s_port_color.find(prop->get_type_name().GetType());
            item->set_port_brush(color_it == s_port_color.end() ? s_fallback_port_color : color_it->second);

            auto display_group = prop_groups[prop->get_display_group()];
            if ((item->get_port_type() == Port::Type::Input) && (display_group != nullptr))
            {
                display_group->add_item(item);
            }
            else
                result.push_back(item);
        }
    }
    return std::move(result);
}

QString ShaderNodeItem::get_icon_path(const UsdPrim& prim) const
{
    static const QString fallback_type = ":/icons/node_editor/shader";
    auto shader = UsdShadeShader(prim);
    TfToken id;
    if (!shader.GetIdAttr().Get(&id))
        return fallback_type;
    const auto plug_name = ShaderNodeRegistry::get_node_plugin_name(id);
    if (plug_name == "usdShaders")
        return ":/icons/node_editor/render_usd";
    else if (plug_name == "usdMtlx")
        return ":/icons/node_editor/render_materialx";
    else if (plug_name == "ndrArnold")
        return ":/icons/node_editor/render_arnold";
    else if (plug_name == "rmanDiscovery")
        return ":/icons/node_editor/render_renderman";
    else if (plug_name == "ndrCycles")
        return ":/icons/node_editor/render_cycles";
    else if (plug_name == "sdrKarmaDiscovery")
        return ":/icons/node_editor/render_karma";
    return fallback_type;
}

void ShaderNodeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_preview_mat_button || event->buttons() != Qt::LeftButton)
    {
        UsdPrimNodeItemBase::mousePressEvent(event);
        return;
    }
    const auto click_pos = m_preview_mat_button->mapFromScene(event->scenePos());
    if (!m_preview_mat_button->contains(click_pos))
    {
        UsdPrimNodeItemBase::mousePressEvent(event);
        return;
    }

    get_model().set_preview_shader(m_enable_mat_preview ? SdfPath::EmptyPath() : SdfPath(get_id()));
}

void ShaderNodeItem::update_node()
{
    auto prim = get_model().get_prim_for_node(get_id());
    assert(prim);
    delete m_preview_mat_button;

    TfToken type;
    if (auto shader = UsdShadeShader(prim))
        shader.GetIdAttr().Get(&type);

    delete m_shader_type_text;
    m_shader_type = type.GetString();
    if (m_shader_type.empty())
    {
        UsdPrimNodeItemBase::update_node();
        return;
    }
    m_shader_type_text = new QGraphicsTextItem(m_shader_type.c_str(), get_display_name_item());
    m_shader_type_text->setDefaultTextColor(QColor(102, 102, 102));
    m_shader_type_text->setTextInteractionFlags(Qt::NoTextInteraction);
    const auto text_rect = m_shader_type_text->boundingRect();
    const auto x_off = m_shader_type_text->mapFromItem(this, s_node_width / 2 - text_rect.width() / 2, 0);
    m_shader_type_text->setPos(x_off.x(), -0.8 * text_rect.height());
    if (auto full_path = get_full_path_text_item())
    {
        full_path->setParentItem(m_shader_type_text);
        const auto x_off = full_path->mapFromItem(this, 0, 0);
        full_path->setPos(x_off.x(), full_path->y());
    }

    bool can_preview_mat = false;
    UsdPrimFallbackProxy proxy(prim);
    for (const auto& prop : proxy.get_all_property_proxies())
    {
        const auto name = prop->get_name_token();
        const auto path = SdfPath(get_id()).AppendProperty(name);
        const auto stripped_name = TfToken(name.data() + name.GetString().find(SdfPathTokens->namespaceDelimiter) + 1);
        if (TfStringStartsWith(prop->get_name_token(), "outputs:"))
        {
            can_preview_mat = true;
            break;
        }
    }

    if (can_preview_mat)
    {
        m_preview_mat_button = new QGraphicsSvgItem(":/icons/node_editor/shader_preview", this);
        enable_preview(get_model().get_preview_shader() == SdfPath(get_id()));

        m_preview_mat_button->setPos(
            s_node_width - 20 - s_port_width - s_port_spacing - s_port_spacing - m_preview_mat_button->boundingRect().width(), s_port_vert_offset);
        m_preview_mat_button->setScale(0.8);
    }

    UsdPrimNodeItemBase::update_node();
}

PropertyLayoutItem* ShaderNodeItem::make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim, int& position)
{
    UsdPrimFallbackProxy proxy(prim);
    auto prop = proxy.get_property_proxy(SdfPath(port_id).GetNameToken());
    if (!prop)
        return nullptr;

    bool is_output = TfStringStartsWith(port_id, "outputs:");

    // not a shader attribute
    if (!is_output && !TfStringStartsWith(port_id, "inputs:"))
        return nullptr;

    const auto metadata = prop->get_all_metadata();
    const auto conectability = metadata.find(SdrPropertyMetadata->Connectable);
    if (conectability != metadata.end() && conectability->second == VtValue(false))
        return nullptr;

    const auto name = SdfPath(port_id).GetNameToken();
    const auto stripped_name = TfToken(name.data() + name.GetString().find(SdfPathTokens->namespaceDelimiter) + 1);
    std::vector<ConnectionItem*> connections;
    for (auto connection : get_prop_connections())
    {
        if (connection->get_id().start_port == port_id || connection->get_id().end_port == port_id)
            connections.push_back(connection);
    }

    PropertyWithPortsLayoutItem* item = nullptr;
    if (is_texture_attribute(m_shader_type, prop->get_name_token()))
    {
        VtValue val;
        std::string file_path;
        if (prop->get(&val))
        {
            if (val.IsHolding<SdfAssetPath>())
#if PXR_VERSION < 2205
                file_path = ArGetResolver().Resolve(val.UncheckedGet<SdfAssetPath>().GetAssetPath());
#else
                file_path = ArGetResolver().Resolve(val.UncheckedGet<SdfAssetPath>().GetAssetPath()).GetPathString();
#endif
            else if (val.IsHolding<std::string>())
                file_path = val.UncheckedGet<std::string>();
        }
        if (!file_path.empty())
        {
            item = new TextureLayoutItem(get_model(), this, port_id, stripped_name, is_output ? Port::Type::Output : Port::Type::Input,
                                         get_scene()->get_thumbnail_cache(), file_path);
        }
    }
    if (!item)
        item = new NamedPropertyLayoutItem(get_model(), this, port_id, stripped_name, is_output ? Port::Type::Output : Port::Type::Input);

    for (auto con : connections)
        item->add_connection(con);

    return item;
}

void ShaderNodeItem::update_port(const PortId& port_id)
{
    const auto prop_name = get_model().get_property_name(port_id);
    // changing this attribute requires full node update, including ports, icon
    if (prop_name == UsdShadeTokens->infoId)
    {
        update_node();
        return;
    }
    else if (is_texture_attribute(m_shader_type, TfToken(prop_name)))
    {
        // TODO: update texture path
    }

    if (auto layout_item = dynamic_cast<PropertyWithPortsLayoutItem*>(get_layout_item_for_port(port_id)))
    {
        auto prim = get_model().get_prim_for_node(get_model().get_node_id_from_port(port_id));
        UsdPrimFallbackProxy proxy(prim);
        if (auto prop = proxy.get_property_proxy(TfToken(prop_name)))
        {
            auto color_it = s_port_color.find(prop->get_type_name().GetType());
            layout_item->set_port_brush(color_it == s_port_color.end() ? s_fallback_port_color : color_it->second);
        }
    }
    UsdPrimNodeItemBase::update_port(port_id);
}

OPENDCC_NAMESPACE_CLOSE
