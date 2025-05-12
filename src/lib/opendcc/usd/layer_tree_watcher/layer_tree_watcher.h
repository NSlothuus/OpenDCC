/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/usd/layer_tree_watcher/api.h"
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/sdf/layer.h>
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"
OPENDCC_NAMESPACE_OPEN
class USD_LAYER_TREE_WATCHER_API LayerTreeWatcher
{
public:
    enum class SublayerChangeType
    {
        Added,
        Removed
    };

    using SublayersChangedDispatcher = eventpp::EventDispatcher<std::string, void(std::string, std::string, SublayerChangeType)>;
    using SublayersChangedDispatcherHandle = typename SublayersChangedDispatcher::Handle;

    LayerTreeWatcher(PXR_NS::UsdStageRefPtr stage);
    LayerTreeWatcher(const LayerTreeWatcher&) = delete;
    LayerTreeWatcher(LayerTreeWatcher&&) = default;

    const std::set<std::string>& get_child_layers(PXR_NS::SdfLayerHandle layer) const;
    const std::set<std::string>& get_child_layers(const std::string& identifier) const;

    bool contains(PXR_NS::SdfLayerHandle layer) const;
    bool contains(const std::string& identifier) const;

    PXR_NS::SdfLayerRefPtrVector get_all_layers() const;
    PXR_NS::SdfLayerRefPtr get_layer(const std::string& identifier, const std::string& anchor = std::string()) const;
    SublayersChangedDispatcherHandle register_sublayers_changed_callback(
        const std::function<void(std::string layer, std::string parent, SublayerChangeType)>& callback);
    void unregister_sublayers_changed_callback(const SublayersChangedDispatcherHandle& handle);
    bool operator=(const LayerTreeWatcher&) const = delete;

private:
    void add_sublayer(PXR_NS::SdfLayerHandle layer, const std::string& parent);
    void remove_sublayer(const std::string& layer, const std::string& parent);
    void rename_layer(const std::string& old_identifier, const std::string& new_identifier);
    std::string get_layer_identifier(const std::string& identifier, const std::string& anchor = std::string()) const;
    class SublayersChangedWatcher : public PXR_NS::TfWeakBase
    {
    public:
        SublayersChangedWatcher(LayerTreeWatcher* layer_tree);

    private:
        void on_layers_changed(const PXR_NS::SdfNotice::LayersDidChange& notice);

        LayerTreeWatcher* m_layer_tree = nullptr;
        PXR_NS::TfNotice::Key m_key;
    };
    std::unique_ptr<SublayersChangedWatcher> m_watcher;

    struct LayerData
    {
        std::set<std::string> sublayers;
        std::unordered_set<std::string> parents;
        LayerData() = default;
        LayerData(const std::set<std::string>& _sublayers, std::string parent)
            : sublayers(_sublayers)
            , parents({ parent })
        {
        }
    };
    std::unordered_map<std::string, LayerData> m_layers;

    SublayersChangedDispatcher m_sublayers_changed_dispatcher;
};
OPENDCC_NAMESPACE_CLOSE
