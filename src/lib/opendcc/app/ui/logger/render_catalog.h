/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/base/qt_python.h"
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN

struct OPENDCC_API CatalogData
{
    std::string log;
    std::string terminal_node;
    std::string elapsed_time;
    float frame_time;
};
using CatalogDataPtr = std::shared_ptr<CatalogData>;

class OPENDCC_API RenderCatalog
{
    RenderCatalog() = default;
    RenderCatalog(const RenderCatalog&) = delete;
    RenderCatalog(RenderCatalog&&) = delete;
    RenderCatalog& operator=(const RenderCatalog&) = delete;
    RenderCatalog& operator=(RenderCatalog&&) = delete;
    ~RenderCatalog() {};

public:
    enum class EventType
    {
        NEW_CATALOG,
        ADD_MSG,
        ACTIVATE_CATALOG,
        UPDATE_CATALOG,
    };
    using NewCatalogDispatcher = eventpp::EventDispatcher<EventType, void(std::string catalog)>;
    using NewCatalogHandle = NewCatalogDispatcher::Handle;
    using UpdateCatalogDispatcher = eventpp::EventDispatcher<EventType, void(std::string catalog)>;
    using UpdateCatalogHandle = UpdateCatalogDispatcher::Handle;
    using ActivateDispatcher = eventpp::EventDispatcher<EventType, void(std::string catalog)>;
    using ActivateHandle = ActivateDispatcher::Handle;
    using AddMsgDispatcher = eventpp::EventDispatcher<EventType, void(std::string catalog, std::string msg)>;
    using AddMsgHandle = AddMsgDispatcher::Handle;

    static RenderCatalog& instance();

    bool create_new_catalog(const std::string& catalog, CatalogDataPtr data);
    void activate_catalog(const std::string& catalog);
    void add_msg(const std::string& catalog, const std::string& msg);
    std::string& get_log(const std::string& catalog);

    const std::string& current_catalog() const;
    std::vector<std::string> catalogs() const;
    const CatalogDataPtr get_info(const std::string& catalog) const;

    NewCatalogHandle at_new_catalg(const std::function<void(std::string catalog)>& callback);
    ActivateHandle at_activate_catalg(const std::function<void(std::string catalog)>& callback);
    AddMsgHandle at_add_msg(const std::function<void(std::string catalog, std::string msg)>& callback);
    UpdateCatalogHandle at_catalg_update(const std::function<void(std::string catalog)>& callback);

    void unregister_new_catalog(NewCatalogHandle handle);
    void unregister_activate_catalog(ActivateHandle handle);
    void unregister_add_msg(AddMsgHandle handle);
    void unregister_catalog_update(UpdateCatalogHandle handle);

private:
    std::unordered_map<std::string, CatalogDataPtr> m_catalog;
    std::string m_current_catalog;
    NewCatalogDispatcher m_new_catalog;
    ActivateDispatcher m_activate;
    AddMsgDispatcher m_add_msg;
    UpdateCatalogDispatcher m_catalog_update;

public:
    void update_catalog_info(const std::string& m_catalog);
};

OPENDCC_NAMESPACE_CLOSE
