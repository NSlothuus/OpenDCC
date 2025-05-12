// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/ui/logger/render_catalog.h"

#include "opendcc/app/ui/application_ui.h"

OPENDCC_NAMESPACE_OPEN

RenderCatalog& RenderCatalog::instance()
{
    static RenderCatalog inst;
    return inst;
}

bool RenderCatalog::create_new_catalog(const std::string& catalog, CatalogDataPtr data)
{
    if (m_catalog.find(catalog) != m_catalog.end())
    {
        return false;
    }

    data->log = i18n("logger.render_catalog", "Time: ").toStdString() + catalog + "\n" + i18n("logger.render_catalog", "Frame: ").toStdString() +
                std::to_string((int)data->frame_time) + "\n" + i18n("logger.render_catalog", "Output: ").toStdString() + data->terminal_node + "\n";

    m_current_catalog = catalog;
    m_catalog[catalog] = data;
    m_new_catalog.dispatch(EventType::NEW_CATALOG, catalog);

    return true;
}

void RenderCatalog::activate_catalog(const std::string& catalog)
{
    m_activate.dispatch(EventType::ACTIVATE_CATALOG, catalog);
}

void RenderCatalog::add_msg(const std::string& catalog, const std::string& msg)
{
    auto catalog_it = m_catalog.find(catalog);
    if (catalog_it == m_catalog.end())
        return;
    catalog_it->second->log += (msg + '\n');
    std::string catalog_val = catalog;
    std::string msg_val = msg;
    m_add_msg.dispatch(EventType::ADD_MSG, catalog_val, msg_val);
}

std::string& RenderCatalog::get_log(const std::string& catalog)
{
    auto catalog_it = m_catalog.find(catalog);
    if (catalog_it == m_catalog.end())
    {
        static std::string empty;
        return empty;
    }

    return catalog_it->second->log;
}

RenderCatalog::NewCatalogHandle RenderCatalog::at_new_catalg(const std::function<void(std::string catalog)>& callback)
{
    return m_new_catalog.appendListener(EventType::NEW_CATALOG, callback);
}

RenderCatalog::ActivateHandle RenderCatalog::at_activate_catalg(const std::function<void(std::string catalog)>& callback)
{
    return m_activate.appendListener(EventType::ACTIVATE_CATALOG, callback);
}

RenderCatalog::AddMsgHandle RenderCatalog::at_add_msg(const std::function<void(std::string catalog, std::string msg)>& callback)
{
    return m_add_msg.appendListener(EventType::ADD_MSG, callback);
}

void RenderCatalog::unregister_new_catalog(NewCatalogHandle handle)
{
    m_new_catalog.removeListener(EventType::NEW_CATALOG, handle);
}

void RenderCatalog::unregister_activate_catalog(ActivateHandle handle)
{
    m_activate.removeListener(EventType::ACTIVATE_CATALOG, handle);
}

void RenderCatalog::unregister_add_msg(AddMsgHandle handle)
{
    m_add_msg.removeListener(EventType::ADD_MSG, handle);
}

const std::string& RenderCatalog::current_catalog() const
{
    return m_current_catalog;
}

std::vector<std::string> RenderCatalog::catalogs() const
{
    std::vector<std::string> catalogs;
    for (auto item : m_catalog)
        catalogs.push_back(item.first);
    return std::move(catalogs);
}

const CatalogDataPtr RenderCatalog::get_info(const std::string& catalog) const
{
    auto info_it = m_catalog.find(catalog);
    if (info_it == m_catalog.end())
        return nullptr;

    return info_it->second;
}

void RenderCatalog::update_catalog_info(const std::string& m_catalog)
{
    m_catalog_update.dispatch(EventType::UPDATE_CATALOG, m_catalog);
}

RenderCatalog::UpdateCatalogHandle RenderCatalog::at_catalg_update(const std::function<void(std::string catalog)>& callback)
{
    return m_catalog_update.appendListener(EventType::UPDATE_CATALOG, callback);
}

void RenderCatalog::unregister_catalog_update(UpdateCatalogHandle handle)
{
    m_catalog_update.removeListener(EventType::UPDATE_CATALOG, handle);
}

OPENDCC_NAMESPACE_CLOSE
