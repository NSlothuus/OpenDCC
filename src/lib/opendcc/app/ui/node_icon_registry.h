/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/app/viewport/viewport_scene_context.h"
#include <QPixmap>

OPENDCC_NAMESPACE_OPEN
enum IconFlags
{
    NONE,
    NOT_ON_EDIT_TARGET = 1 << 0
};

class NodeIcon
{
public:
    NodeIcon() {}
    NodeIcon(QPixmap icon)
        : m_pixmap(icon)
    {
    }
    const QPixmap& get_icon() { return m_pixmap; }
    bool has_svg() { return m_has_svg; }
    void set_svg(const std::string& svg_path)
    {
        m_svg = svg_path;
        m_has_svg = true;
    }
    const std::string& get_svg() { return m_svg; }

private:
    QPixmap m_pixmap;
    std::string m_svg;
    bool m_has_svg = false;
};

class OPENDCC_API NodeIconRegistry
{
public:
    static NodeIconRegistry& instance();

    void register_icon(const PXR_NS::TfToken& context_type, const std::string& node_type, const std::string& icon_path,
                       const std::string& svg_path = "");
    void register_icon(const PXR_NS::TfToken& context_type, const std::string& node_type, const QPixmap& pixmap, const std::string& svg_path = "");
    void unregister_icon(const PXR_NS::TfToken& context_type, const std::string& node_type);

    const QPixmap& get_icon(const PXR_NS::TfToken& context_type, const std::string& node_type, IconFlags flags = IconFlags::NONE);
    const std::string& get_svg(const PXR_NS::TfToken& context_type, const std::string& node_type, IconFlags flags = IconFlags::NONE);

    bool is_icon_exists(const PXR_NS::TfToken& context_type, const std::string& node_type, IconFlags flags = IconFlags::NONE);
    bool is_svg_exists(const PXR_NS::TfToken& context_type, const std::string& node_type, IconFlags flags = IconFlags::NONE);

private:
    NodeIconRegistry() = default;

    using PerFlags = std::unordered_map<IconFlags, NodeIcon>;
    using PerNodeType = std::unordered_map<std::string, PerFlags>;
    using ContextMap = std::unordered_map<PXR_NS::TfToken, PerNodeType, PXR_NS::TfToken::HashFunctor>;

    ContextMap m_registry;
};
OPENDCC_NAMESPACE_CLOSE
