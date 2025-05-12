/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"

#include <pxr/pxr.h>
#include <pxr/usd/usdUtils/stageCache.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <vector>
#include <string>

#include "opendcc/base/vendor/eventpp/eventdispatcher.h"
#include <pxr/usd/usd/notice.h>
#include "opendcc/usd/layer_tree_watcher/layer_tree_watcher.h"
#include "opendcc/app/core/api.h"
#include "opendcc/app/core/topology_cache.h"
#include "opendcc/app/core/half_edge_cache.h"

OPENDCC_NAMESPACE_OPEN

class Application;
class StageObjectChangedWatcher;
class StageEditTargetChangedWatcher;
class UsdEditUndoStackChangedWatcher;
class SdfLayerDirtinessChangedWatcher;
class LayerStateDelegatesHolder;
class ShareEditsContext;

/**
 * @brief The Session class provides a means of simultaneous
 * interaction with one or multiple UsdStages within the application.
 *
 * The Session class includes a specialized object for managing
 * the stages and retaining the UsdStageCache.
 * It allows to switch between the stages and close them as required.
 *
 * Furthermore, the object can be utilized to subscribe to the events,
 * such as stage opening or closing (e.g. facilitate the user interface updating).
 *
 * The ownership over the UsdStages that were opened outside of the Session.
 *
 * This class contains shared UsdGeomXformCache and UsdGeomBBoxCache.
 *
 * Additionally, this class manages the ``Live Share`` feature which
 * allows to transfer the USD delta between two processes within the operating system.
 *
 *
 */
class Session
{
public:
    /**
     * @brief Defines the common Session events.
     *
     */
    enum class EventType
    {
        /**
         * @brief Triggered on changing the live-sharing mode state.
         *
         */
        LIVE_SHARE_STATE_CHANGED
    };

    /**
     * @brief Defines the stage changes event types.
     *
     */
    enum class StageChangedEventType
    {
        /**
         * @brief Triggered on changing an object in the current stage.
         *
         */
        CURRENT_STAGE_OBJECT_CHANGED
    };

    using EventDispatcher = eventpp::EventDispatcher<EventType, void()>;
    using CallbackHandle = EventDispatcher::Handle;

    using StageChangedEventDispatcher = eventpp::EventDispatcher<StageChangedEventType, void(PXR_NS::UsdNotice::ObjectsChanged const& notice)>;
    using StageChangedCallbackHandle = eventpp::EventDispatcher<StageChangedEventType, void(PXR_NS::UsdNotice::ObjectsChanged const& notice)>::Handle;

    /**
     * @brief Gets the stage cache.
     *
     * @return Stage cache.
     */
    OPENDCC_API PXR_NS::UsdStageCache& get_stage_cache();
    /**
     * @brief Sets the specified stage as current.
     *
     * It triggers the ``Application::EventType::CURRENT_STAGE_CHANGED`` and ``Application::EventType::EDIT_TARGET_CHANGED`` events.
     *
     * If the specified stage is not in the list of the session stages, adds it
     * and triggers the ``Application::EventType::SESSION_STAGE_LIST_CHANGED`` event.
     *
     * @param stage The USD stage to set.
     */
    OPENDCC_API void set_current_stage(const PXR_NS::UsdStageRefPtr& stage);
    /**
     * @brief Sets the specified stage as current using it's cache id.
     *
     * It triggers the ``Application::EventType::CURRENT_STAGE_CHANGED`` and ``Application::EventType::EDIT_TARGET_CHANGED`` events.
     *
     * If the specified stage is not in the list of the session stages, adds it
     * and triggers the ``Application::EventType::SESSION_STAGE_LIST_CHANGED`` event.
     *
     * @param id The cache id of the USD stage to set.
     */
    OPENDCC_API void set_current_stage(PXR_NS::UsdStageCache::Id id);
    /**
     * @brief Returns a pointer to the active UsdStage.
     *
     */
    OPENDCC_API PXR_NS::UsdStageRefPtr get_current_stage();
    /**
     * @brief Returns the id associated with the stage in the stage cache.
     * If the stage is not present in the cache, returns an invalid Id.
     *
     * @param stage The USD stage to retrieve id from.
     */
    OPENDCC_API PXR_NS::UsdStageCache::Id get_stage_id(const PXR_NS::UsdStageRefPtr& stage);
    /**
     * @brief Returns the current stage layer tree.
     *
     */
    OPENDCC_API std::shared_ptr<LayerTreeWatcher> get_current_stage_layer_tree() const;

    /**
     * @brief Returns the id of the current stage.
     *
     */
    OPENDCC_API PXR_NS::UsdStageCache::Id get_current_stage_id();
    /**
     * @brief Attempts to open a stage with the specified root layer.
     * If succeed, triggers the ``Application::EventType::SESSION_STAGE_LIST_CHANGED`` event.
     *
     * @param layer The SdfLayer to be used as a root layer of the UsdStage.
     */
    OPENDCC_API PXR_NS::UsdStageRefPtr open_stage(const PXR_NS::SdfLayerHandle& layer);
    /**
     * @brief Opens the stage using an asset path.
     *
     * @param asset_path The path to the asset.
     */
    OPENDCC_API PXR_NS::UsdStageRefPtr open_stage(const std::string& asset_path);
    /**
     * @brief Closes a stage with the specified id in the stage cache.
     *
     * @param id The id of the stage to be closed.
     * @return true if the stage with the specified id is found and closed, false otherwise.
     *
     * If it closes the current stage, it triggers the
     * ``Application::EventType::BEFORE_CURRENT_STAGE_CLOSED`` event.
     *
     * If the id is present in the Session's stage cache,
     * it also triggers the ``Application::EventType::CURRENT_STAGE_CHANGED``
     * and ``Application::EventType::EDIT_TARGET_CHANGED`` events.
     */
    OPENDCC_API bool close_stage(PXR_NS::UsdStageCache::Id id);
    /**
     * @brief Closes the specified stage in the stage cache.
     *
     * @param stage The stage to be closed.
     * @return true if the specified stage is found and closed, false otherwise.
     *
     * If it closes the current stage, it triggers the
     * ``Application::EventType::BEFORE_CURRENT_STAGE_CLOSED`` event.
     *
     * If the stage is present in the Session's stage cache,
     * it also triggers the ``Application::EventType::CURRENT_STAGE_CHANGED``
     * and ``Application::EventType::EDIT_TARGET_CHANGED`` events.
     */
    OPENDCC_API bool close_stage(const PXR_NS::UsdStageRefPtr& stage);
    /**
     * @brief Closes all opened stages.
     *
     * Triggers the following events:
     *
     *		- ``Application::EventType::BEFORE_CURRENT_STAGE_CLOSED``;
     *		- ``Application::EventType::CURRENT_STAGE_CHANGED``;
     * 		- ``Application::EventType::EDIT_TARGET_CHANGED``;
     * 		- ``Application::EventType::SESSION_STAGE_LIST_CHANGED``.
     *
     */
    OPENDCC_API void close_all();
    /**
     * @brief Force updates the stage list.
     *
     * Triggers the ``Application::EventType::SESSION_STAGE_LIST_CHANGED`` events.
     *
     */
    OPENDCC_API void force_update_stage_list();

    /**
     * @brief Gets a list of the opened stages.
     *
     * @return A list of the opened stages.
     */
    OPENDCC_API std::vector<PXR_NS::UsdStageRefPtr> get_stage_list();
    /**
     * @brief Returns the bounding box cache associated
     * with the specified id from the stage cache.
     *
     * @param id The id of the stage in the stage cache.
     *
     * @returns UsdGeomBBoxCache associated with the specified stage id.
     */
    OPENDCC_API PXR_NS::UsdGeomBBoxCache& get_stage_bbox_cache(PXR_NS::UsdStageCache::Id id);
    /**
     * @brief Returns the xform cache associated with
     * the specified id from the stage cache.
     *
     * @param id The id of the stage in the stage cache.
     * @returns UsdGeomXformCache associated with the specified stage id.
     */
    OPENDCC_API PXR_NS::UsdGeomXformCache& get_stage_xform_cache(PXR_NS::UsdStageCache::Id id);
    /**
     * @brief Returns a geometry topology cache associated
     * with the specified id from the stage cache.
     *
     * @param id The id of the stage in the stage cache.
     * @returns TopologyCache associated with the specified stage id.
     */
    OPENDCC_API TopologyCache& get_stage_topology_cache(PXR_NS::UsdStageCache::Id id);
    /**
     * @brief Returns a HalfEdge cache associated
     * with the specified id from the stage cache.
     *
     * @param id The id of the stage in the stage cache.
     * @returns HalfEdge associated with the specified stage id.
     */
    OPENDCC_API HalfEdgeCache& get_half_edge_cache(PXR_NS::UsdStageCache::Id id);
    /**
     * @brief Updates the current stage BBox cache time.
     *
     */
    void update_current_stage_bbox_cache_time();
    /**
     * @brief Updates current stage Xform cache time.
     *
     */
    void update_current_stage_xform_cache_time();
    /**
     * @brief Subscribes a callback to the stage changing events.
     *
     * @param event_type The stage event type.
     * @param callback The function to be called when event is triggered.
     *
     * @return A callback handle for the event
     * which is required for unsubscription from the event.
     */
    OPENDCC_API StageChangedCallbackHandle register_stage_changed_callback(
        const StageChangedEventType& event_type, const std::function<void(PXR_NS::UsdNotice::ObjectsChanged const& notice)> callback);
    /**
     * @brief Unsubscribes a callback from the stage changing events.
     *
     * @param event_type The event type to remove the callback from.
     * @param handle The callback handle.
     */
    OPENDCC_API void unregister_stage_changed_callback(const StageChangedEventType& event_type, StageChangedCallbackHandle& handle);
    /**
     * @brief Subscribes a callback to the common session events.
     *
     * @param event_type The event type to set callback for.
     * @param callback The callback to be called when event is triggered.
     * @return A callback handle for the event
     * which is required for unsubscription from the event.
     */
    Session::CallbackHandle register_event_callback(EventType event_type, std::function<void()> callback);
    /**
     * @brief Unsubscribes a callback from the common session events.
     *
     * @param event_type The event type from which to remove a callback.
     * @param handle The callback handle to remove.
     */
    void unregister_event_callback(EventType event_type, const Session::CallbackHandle& handle);
    /**
     * @brief Toggles the live-sharing mode.
     *
     * Triggers the ``EventType::LIVE_SHARE_STATE_CHANGED`` event.
     *
     * @param enable A flag which defines whether the
     * live-sharing mode should be enabled or disabled.
     */
    OPENDCC_API void enable_live_sharing(bool enable);
    /**
     * @brief Checks whether the live-sharing mode is enabled.
     *
     * @return true if the live-sharing mode is enabled, false otherwise.
     */
    OPENDCC_API bool is_live_sharing_enabled();
    /**
     * @brief Processes the incoming live-sharing events.
     *
     */
    OPENDCC_API void process_events();
    /**
     * @brief Sets the transferred content directory.
     *
     * The transferred content directory stores intermediate state of the
     * live sharing edits and interprocess communications.
     *
     * @param dir The transferred content directory path to set.
     */
    OPENDCC_API void set_transferred_content_dir(const std::string& dir);
    /**
     * @brief Returns the transferred content directory.
     *
     */
    OPENDCC_API std::string get_transferred_content_dir() const;

private:
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    void create_stage_half_edge_cache(PXR_NS::UsdStageCache::Id id);

    void clear_stage_bbox_cache(PXR_NS::UsdStageCache::Id id);
    void clear_stage_xform_cache(PXR_NS::UsdStageCache::Id id);
    void clear_stage_topology_cache(PXR_NS::UsdStageCache::Id id);
    void clear_stage_half_edge_cache(PXR_NS::UsdStageCache::Id id);
    void update_watcher();

    std::string m_transferred_content_dir;
    PXR_NS::UsdStageCache::Id m_current_stage_id;
    std::vector<PXR_NS::UsdStageCache::Id> m_stage_list_ids;
    std::unique_ptr<StageObjectChangedWatcher> m_stage_watcher;
    std::unique_ptr<StageEditTargetChangedWatcher> m_edit_target_watcher;
    std::unique_ptr<SdfLayerDirtinessChangedWatcher> m_edit_target_dirty_watcher;
    std::unique_ptr<UsdEditUndoStackChangedWatcher> m_undo_stack_watcher;
    std::map<PXR_NS::UsdStageCache::Id, PXR_NS::UsdGeomBBoxCache> m_per_stage_bbox_cache;
    std::map<PXR_NS::UsdStageCache::Id, PXR_NS::UsdGeomXformCache> m_per_stage_xform_cache;
    std::map<PXR_NS::UsdStageCache::Id, TopologyCache> m_per_stage_topology_cache;
    using HalfEdgeCachePerStage = std::unordered_map<PXR_NS::UsdStageCache::Id, HalfEdgeCache, PXR_NS::TfHash>;
    HalfEdgeCachePerStage m_half_edge_cache;
    StageChangedEventDispatcher m_stage_changed_event_dispatcher;
    std::shared_ptr<LayerTreeWatcher> m_layer_tree_watcher;
    std::shared_ptr<LayerStateDelegatesHolder> m_layer_state_delegates;
    std::unique_ptr<ShareEditsContext> m_share_context;
    EventDispatcher m_event_dispatcher;
    Session();
    virtual ~Session();
    friend class Application;
};

OPENDCC_NAMESPACE_CLOSE
