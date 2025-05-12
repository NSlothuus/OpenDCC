// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd/layer_tree_watcher/layer_tree_watcher.h"
#include <pxr/usd/pcp/node.h>
#include <pxr/usd/pcp/layerStack.h>
#include "pxr/usd/sdf/notice.h"
#include "pxr/usd/ar/resolver.h"
#include <pxr/usd/usd/prim.h>

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    const std::string SUBLAYERS_CHANGED = "sublayers_changed";
};

LayerTreeWatcher::SublayersChangedWatcher::SublayersChangedWatcher(LayerTreeWatcher* layer_tree)
    : m_layer_tree(layer_tree)
{
    m_key = TfNotice::Register(TfWeakPtr<LayerTreeWatcher::SublayersChangedWatcher>(this), &SublayersChangedWatcher::on_layers_changed);
}

void LayerTreeWatcher::SublayersChangedWatcher::on_layers_changed(const PXR_NS::SdfNotice::LayersDidChange& notice)
{
    auto& layers = m_layer_tree->m_layers;

#if PXR_VERSION >= 2002
    auto change_list = notice.GetChangeListVec();
#else
    auto change_list = notice.GetChangeListMap();
#endif
    for (const auto& change : change_list)
    {
        const auto& changed_layer_id = change.first->GetIdentifier();
        for (const auto& entry : change.second.GetEntryList())
        {
            if (entry.second.flags.didChangeIdentifier)
            {
                m_layer_tree->rename_layer(entry.second.oldIdentifier, changed_layer_id);
            }

            for (const auto& sublayer_change : entry.second.subLayerChanges)
            {
                auto changed_sublayer = m_layer_tree->get_layer(sublayer_change.first, changed_layer_id);
                if (!changed_sublayer && sublayer_change.second == SdfChangeList::SubLayerChangeType::SubLayerAdded)
                {
                    TF_CODING_ERROR("Failed to find layer with identifier '%s' and anchor '%s'.", sublayer_change.first.c_str(),
                                    changed_layer_id.c_str());
                    continue;
                }
                const auto& changed_sublayer_id = changed_sublayer ? changed_sublayer->GetIdentifier()
                                                                   : m_layer_tree->get_layer_identifier(sublayer_change.first, changed_layer_id);
                switch (sublayer_change.second)
                {
                case SdfChangeList::SubLayerChangeType::SubLayerAdded:
                    m_layer_tree->add_sublayer(changed_sublayer, changed_layer_id);
                    break;
                case SdfChangeList::SubLayerChangeType::SubLayerRemoved:
                    m_layer_tree->remove_sublayer(changed_sublayer_id, changed_layer_id);
                    break;
                }
            }
        }
    }
}

LayerTreeWatcher::LayerTreeWatcher(PXR_NS::UsdStageRefPtr stage)
{
    if (!stage)
        return;

    auto root_prim = stage->GetPseudoRoot();
    if (!root_prim)
        return;

    add_sublayer(stage->GetRootLayer(), "");
    add_sublayer(stage->GetSessionLayer(), "");
    m_watcher = std::make_unique<SublayersChangedWatcher>(this);
}

const std::set<std::string>& LayerTreeWatcher::get_child_layers(PXR_NS::SdfLayerHandle layer) const
{
    static const std::set<std::string> empty;
    if (!layer)
        return empty;

    return get_child_layers(layer->GetIdentifier());
}

const std::set<std::string>& LayerTreeWatcher::get_child_layers(const std::string& identifier) const
{
    static const std::set<std::string> empty;

    auto iter = m_layers.find(identifier);
    return iter == m_layers.end() ? empty : iter->second.sublayers;
}

bool LayerTreeWatcher::contains(PXR_NS::SdfLayerHandle layer) const
{
    return layer && contains(layer->GetIdentifier());
}

bool LayerTreeWatcher::contains(const std::string& identifier) const
{
    return m_layers.find(identifier) != m_layers.end();
}

SdfLayerRefPtrVector LayerTreeWatcher::get_all_layers() const
{
    SdfLayerRefPtrVector result(m_layers.size());
    std::transform(m_layers.begin(), m_layers.end(), result.begin(), [this](const std::pair<std::string, LayerData>& val) {
        const auto parent_id = val.second.parents.empty() ? "" : *val.second.parents.begin();
        const auto layer = get_layer(val.first, parent_id);
        TF_VERIFY(layer, "Failed to find layer with identifier '%s'. Layer tree might be corrupted.", val.first.c_str());
        return layer;
    });
    return result;
}

SdfLayerRefPtr LayerTreeWatcher::get_layer(const std::string& identifier, const std::string& anchor) const
{
    return SdfLayer::FindOrOpen(get_layer_identifier(identifier, anchor));
}

LayerTreeWatcher::SublayersChangedDispatcherHandle LayerTreeWatcher::register_sublayers_changed_callback(
    const std::function<void(std::string, std::string, SublayerChangeType)>& callback)
{
    return m_sublayers_changed_dispatcher.appendListener("sublayers_changed", callback);
}

void LayerTreeWatcher::unregister_sublayers_changed_callback(const SublayersChangedDispatcherHandle& handle)
{
    m_sublayers_changed_dispatcher.removeListener("sublayers_changed", handle);
}

void LayerTreeWatcher::add_sublayer(SdfLayerHandle layer, const std::string& parent)
{
    if (!layer)
        return;

    m_sublayers_changed_dispatcher.dispatch(SUBLAYERS_CHANGED, layer->GetIdentifier(), parent, LayerTreeWatcher::SublayerChangeType::Added);

    auto parent_iter = m_layers.find(parent);
    if (parent_iter != m_layers.end())
        parent_iter->second.sublayers.insert(layer->GetIdentifier());

    const auto layer_iter = m_layers.find(layer->GetIdentifier());
    if (layer_iter != m_layers.end())
    {
        layer_iter->second.parents.insert(parent);

        m_sublayers_changed_dispatcher.dispatch(SUBLAYERS_CHANGED, layer->GetIdentifier(), parent, LayerTreeWatcher::SublayerChangeType::Added);
        return;
    }

    const auto external_references = layer->GetExternalReferences();
    m_layers[layer->GetIdentifier()] = LayerData { {}, parent };

    for (const auto& identifier : external_references)
    {
        add_sublayer(get_layer(identifier, layer->GetIdentifier()), layer->GetIdentifier());
    }
}

void LayerTreeWatcher::remove_sublayer(const std::string& layer, const std::string& parent)
{
    auto parent_iter = m_layers.find(parent);
    if (parent_iter != m_layers.end())
        parent_iter->second.sublayers.erase(layer);

    m_sublayers_changed_dispatcher.dispatch(SUBLAYERS_CHANGED, layer, parent, LayerTreeWatcher::SublayerChangeType::Removed);

    auto removed_layer_iter = m_layers.find(layer);
    if (removed_layer_iter == m_layers.end())
        return;

    auto& removed_layer = removed_layer_iter->second;
    removed_layer.parents.erase(parent);
    if (removed_layer.parents.empty())
    {
        auto sublayers = removed_layer.sublayers;
        for (const auto& child : sublayers)
        {
            auto& child_layer = m_layers[child];
            if (child_layer.parents.size() == 1)
                remove_sublayer(child, layer);
        }
        m_layers.erase(layer);
    }
}

void LayerTreeWatcher::rename_layer(const std::string& old_identifier, const std::string& new_identifier)
{
    auto layer_iter = m_layers.find(old_identifier);
    if (layer_iter == m_layers.end())
    {
        return;
    }

    for (const auto& parent : layer_iter->second.parents)
    {
        auto it = m_layers.find(parent);
        if (it == m_layers.end())
        {
            continue;
        }
        it->second.sublayers.erase(old_identifier);
        it->second.sublayers.insert(new_identifier);
    }
    for (const auto& child : layer_iter->second.sublayers)
    {
        auto it = m_layers.find(child);
        if (it == m_layers.end())
        {
            continue;
        }
        it->second.parents.erase(old_identifier);
        it->second.parents.insert(new_identifier);
    }

    auto layer_data = layer_iter->second;
    m_layers.erase(old_identifier);
    m_layers.emplace(new_identifier, std::move(layer_data));
}

std::string LayerTreeWatcher::get_layer_identifier(const std::string& identifier, const std::string& anchor /*= std::string()*/) const
{
    if (SdfLayer::IsAnonymousLayerIdentifier(identifier))
        return identifier;
#if !defined(AR_VERSION) || AR_VERSION == 1
    return ArGetResolver().IsRelativePath(identifier) ? ArGetResolver().AnchorRelativePath(anchor, identifier) : identifier;
#else
    return ArGetResolver().CreateIdentifier(identifier, ArResolvedPath(anchor));
#endif
}
OPENDCC_NAMESPACE_CLOSE

#include <opendcc/base/vendor/ghc/filesystem.hpp>
#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
// Note: this define should be used once per shared lib
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#include <doctest/doctest.h>
OPENDCC_NAMESPACE_USING

class SetUp
{
private:
    static ghc::filesystem::path m_tmp_dir;

public:
    SetUp()
    {
        m_tmp_dir = ghc::filesystem::temp_directory_path() / "layer_tree_tests";
        ghc::filesystem::remove_all(m_tmp_dir);

        ghc::filesystem::create_directory(m_tmp_dir);
        ghc::filesystem::create_directory(m_tmp_dir / "sub");
        ghc::filesystem::create_directory(m_tmp_dir / "sub/directory");

        std::ofstream(m_tmp_dir.generic_string() + "/empty.usda") << "#usda 1.0";
        std::ofstream(m_tmp_dir.generic_string() + "/empty2.usda") << "#usda 1.0";
        std::ofstream(m_tmp_dir.generic_string() + "/rel.usda") << "#usda 1.0\n(subLayers = [@./empty.usda@])";
        std::ofstream(m_tmp_dir.generic_string() + "/abs.usda")
            << "#usda 1.0\n(subLayers = [@" << (m_tmp_dir / "empty.usda").generic_string() << "@])";
        std::ofstream(m_tmp_dir.generic_string() + "/sub/directory/subdir.usda") << "#usda 1.0\n(subLayers = [@../../empty2.usda@])";
    }
    ~SetUp() { ghc::filesystem::remove_all(m_tmp_dir); }

    static std::string get_tmp_dir()
    {
        std::string result = m_tmp_dir.generic_string();
        return result;
    }
};

ghc::filesystem::path SetUp::m_tmp_dir;

DOCTEST_TEST_SUITE("LayerTreeWatcherTests")
{
    DOCTEST_TEST_CASE("empty_tree")
    {
        auto stage = UsdStage::CreateInMemory();
        LayerTreeWatcher layer_tree_watcher(stage);

        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 2);
    }

    DOCTEST_TEST_CASE("init_with_anon_sublayers")
    {
        auto sublayer1 = SdfLayer::CreateAnonymous();
        auto sublayer2 = SdfLayer::CreateAnonymous();
        auto stage = UsdStage::CreateInMemory();
        stage->GetRootLayer()->InsertSubLayerPath(sublayer1->GetIdentifier());
        stage->GetRootLayer()->InsertSubLayerPath(sublayer2->GetIdentifier());
        LayerTreeWatcher layer_tree_watcher(stage);

        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 4);
        DOCTEST_CHECK(layer_tree_watcher.contains(sublayer1));
        DOCTEST_CHECK(layer_tree_watcher.contains(sublayer2));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "init_with_existing_sublayers")
    {
        auto stage = UsdStage::Open(SetUp::get_tmp_dir() + "/abs.usda");
        LayerTreeWatcher layer_tree_watcher(stage);

        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 3);
        DOCTEST_CHECK(layer_tree_watcher.contains(SdfLayer::FindOrOpen(SetUp::get_tmp_dir() + "/empty.usda")));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "init_with_nonexported_sublayers")
    {
        auto new_layer = SdfLayer::CreateNew(SetUp::get_tmp_dir() + "/nonexported.usda");
        auto stage = UsdStage::CreateInMemory();
        stage->GetRootLayer()->InsertSubLayerPath(new_layer->GetIdentifier());
        LayerTreeWatcher layer_tree_watcher(stage);

        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 3);
        DOCTEST_CHECK(layer_tree_watcher.contains(SdfLayer::FindOrOpen(SetUp::get_tmp_dir() + "/nonexported.usda")));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "init_with_rel_sublayers")
    {
        auto stage = UsdStage::Open(SetUp::get_tmp_dir() + "/rel.usda");
        LayerTreeWatcher layer_tree_watcher(stage);

        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 3);
        DOCTEST_CHECK(layer_tree_watcher.contains(SdfLayer::FindOrOpen(SetUp::get_tmp_dir() + "/empty.usda")));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "init_with_rel_sublayer_in_subdir")
    {
        auto stage = UsdStage::Open(SetUp::get_tmp_dir() + "/sub/directory/subdir.usda");
        LayerTreeWatcher layer_tree_watcher(stage);

        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 3);
        DOCTEST_CHECK(layer_tree_watcher.contains(SdfLayer::FindOrOpen(SetUp::get_tmp_dir() + "/empty2.usda")));
    }

    DOCTEST_TEST_CASE("add_anon_sublayers_to_root")
    {
        auto sublayer1 = SdfLayer::CreateAnonymous();
        auto sublayer2 = SdfLayer::CreateAnonymous();
        auto stage = UsdStage::CreateInMemory();

        LayerTreeWatcher layer_tree_watcher(stage);
        stage->GetRootLayer()->InsertSubLayerPath(sublayer1->GetIdentifier());
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 3);
        DOCTEST_CHECK(layer_tree_watcher.contains(sublayer1));

        stage->GetRootLayer()->InsertSubLayerPath(sublayer2->GetIdentifier());
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 4);
        DOCTEST_CHECK(layer_tree_watcher.contains(sublayer2));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "add_anon_sublayers_to_existing_stage_root")
    {
        auto sublayer1 = SdfLayer::CreateAnonymous();
        auto sublayer2 = SdfLayer::CreateAnonymous();
        auto stage = UsdStage::Open(SetUp::get_tmp_dir() + "/empty.usda");

        LayerTreeWatcher layer_tree_watcher(stage);
        stage->GetRootLayer()->InsertSubLayerPath(sublayer1->GetIdentifier());
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 3);
        DOCTEST_CHECK(layer_tree_watcher.contains(sublayer1));

        stage->GetRootLayer()->InsertSubLayerPath(sublayer2->GetIdentifier());
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 4);
        DOCTEST_CHECK(layer_tree_watcher.contains(sublayer2));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "add_existing_sublayers_to_root")
    {
        auto stage = UsdStage::CreateInMemory();

        LayerTreeWatcher layer_tree_watcher(stage);
        auto sublayer = SdfLayer::FindOrOpen(SetUp::get_tmp_dir() + "/empty.usda");
        stage->GetRootLayer()->InsertSubLayerPath(sublayer->GetIdentifier());
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 3);
        DOCTEST_CHECK(layer_tree_watcher.contains(sublayer));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "add_nonexported_sublayers_to_root")
    {
        auto stage = UsdStage::CreateInMemory();

        LayerTreeWatcher layer_tree_watcher(stage);
        auto new_layer = SdfLayer::CreateNew(SetUp::get_tmp_dir() + "/nonexported.usda");

        stage->GetRootLayer()->InsertSubLayerPath(new_layer->GetIdentifier());
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 3);
        DOCTEST_CHECK(layer_tree_watcher.contains(new_layer));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "add_from_subdirectory")
    {
        auto stage = UsdStage::Open(SetUp::get_tmp_dir() + "/empty.usda");
        LayerTreeWatcher layer_tree_watcher(stage);
        stage->GetRootLayer()->InsertSubLayerPath("./sub/directory/subdir.usda");

        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 4);
        DOCTEST_CHECK(layer_tree_watcher.contains(SdfLayer::FindOrOpen(SetUp::get_tmp_dir() + "/sub/directory/subdir.usda")));
        DOCTEST_CHECK(layer_tree_watcher.contains(SdfLayer::FindOrOpen(SetUp::get_tmp_dir() + "/empty2.usda")));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "add_to_session_layer")
    {
        auto stage = UsdStage::Open(SetUp::get_tmp_dir() + "/empty.usda");

        LayerTreeWatcher layer_tree_watcher(stage);
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 2);
        stage->GetSessionLayer()->InsertSubLayerPath(SetUp::get_tmp_dir() + "/empty2.usda");

        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 3);
        DOCTEST_CHECK(layer_tree_watcher.contains(SdfLayer::Find(SetUp::get_tmp_dir() + "/empty.usda")));
        DOCTEST_CHECK(layer_tree_watcher.contains(stage->GetSessionLayer()));
        DOCTEST_CHECK(layer_tree_watcher.contains(SdfLayer::Find(stage->GetSessionLayer()->GetSubLayerPaths().front())));
    }

    DOCTEST_TEST_CASE("remove_anon_sublayers_from_root")
    {
        auto sublayer1 = SdfLayer::CreateAnonymous();
        auto sublayer2 = SdfLayer::CreateAnonymous();
        auto stage = UsdStage::CreateInMemory();
        stage->GetRootLayer()->InsertSubLayerPath(sublayer1->GetIdentifier());
        stage->GetRootLayer()->InsertSubLayerPath(sublayer2->GetIdentifier());

        LayerTreeWatcher layer_tree_watcher(stage);
        stage->GetRootLayer()->RemoveSubLayerPath(0);
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 3);
        DOCTEST_CHECK(!layer_tree_watcher.contains(sublayer1));

        stage->GetRootLayer()->RemoveSubLayerPath(0);
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 2);
        DOCTEST_CHECK(!layer_tree_watcher.contains(sublayer2));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "remove_existing_sublayers_from_root")
    {
        auto stage = UsdStage::Open(SetUp::get_tmp_dir() + "/abs.usda");

        LayerTreeWatcher layer_tree_watcher(stage);
        auto sublayer = SdfLayer::FindOrOpen(SetUp::get_tmp_dir() + "/empty.usda");
        stage->GetRootLayer()->RemoveSubLayerPath(0);
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 2);
        DOCTEST_CHECK(!layer_tree_watcher.contains(sublayer));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "remove_nonexported_sublayers_from_root")
    {
        auto stage = UsdStage::CreateInMemory();
        auto new_layer = SdfLayer::CreateNew(SetUp::get_tmp_dir() + "/nonexported.usda");
        stage->GetRootLayer()->InsertSubLayerPath(new_layer->GetIdentifier());

        LayerTreeWatcher layer_tree_watcher(stage);
        stage->GetRootLayer()->RemoveSubLayerPath(0);

        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 2);
        DOCTEST_CHECK(!layer_tree_watcher.contains(new_layer));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "remove_from_subdirectory")
    {
        auto stage = UsdStage::Open(SetUp::get_tmp_dir() + "/empty.usda");
        stage->GetRootLayer()->InsertSubLayerPath("./sub/directory/subdir.usda");
        LayerTreeWatcher layer_tree_watcher(stage);
        stage->GetRootLayer()->RemoveSubLayerPath(0);

        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 2);
        DOCTEST_CHECK(layer_tree_watcher.contains(stage->GetRootLayer()));
    }

    DOCTEST_TEST_CASE_FIXTURE(SetUp, "remove_from_session_layer")
    {
        auto stage = UsdStage::Open(SetUp::get_tmp_dir() + "/empty.usda");
        stage->GetSessionLayer()->InsertSubLayerPath(SetUp::get_tmp_dir() + "/empty2.usda");

        LayerTreeWatcher layer_tree_watcher(stage);
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 3);
        auto sublayer = SdfLayer::FindOrOpen(SetUp::get_tmp_dir() + "/empty2.usda");
        stage->GetSessionLayer()->RemoveSubLayerPath(0);

        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 2);
        DOCTEST_CHECK(!layer_tree_watcher.contains(sublayer));
    }

    DOCTEST_TEST_CASE("change_layer_identifier")
    {
        auto layer = SdfLayer::CreateAnonymous();
        auto child_layer = SdfLayer::CreateAnonymous();
        layer->InsertSubLayerPath(child_layer->GetIdentifier());
        auto stage = UsdStage::CreateInMemory();

        LayerTreeWatcher layer_tree_watcher(stage);
        stage->GetRootLayer()->InsertSubLayerPath(layer->GetIdentifier());
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 4);
        DOCTEST_CHECK(layer_tree_watcher.contains(layer));
        DOCTEST_CHECK(layer_tree_watcher.contains(child_layer));

        auto old_identifier = layer->GetIdentifier();
        layer->SetIdentifier(SetUp::get_tmp_dir() + "/temp.usda");
        auto new_identifier = layer->GetIdentifier();
        DOCTEST_CHECK(layer_tree_watcher.get_all_layers().size() == 4);
        DOCTEST_CHECK(layer_tree_watcher.contains(layer));
        DOCTEST_CHECK(layer_tree_watcher.contains(child_layer));

        DOCTEST_CHECK(!layer_tree_watcher.contains(old_identifier));
        DOCTEST_CHECK(layer_tree_watcher.contains(layer->GetIdentifier()));
        auto child_layers = layer_tree_watcher.get_child_layers(layer->GetIdentifier());
        DOCTEST_CHECK(child_layers.size() == 1);
        DOCTEST_CHECK(*child_layers.begin() == child_layer->GetIdentifier());
        auto root_sublayers = layer_tree_watcher.get_child_layers(stage->GetRootLayer()->GetIdentifier());
        DOCTEST_CHECK(root_sublayers.size() == 1);
        DOCTEST_CHECK(*root_sublayers.begin() == layer->GetIdentifier());
    }
}
