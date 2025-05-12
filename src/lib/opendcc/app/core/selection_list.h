/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/base/vt/array.h>
#include <pxr/imaging/hdx/pickTask.h>
#include <pxr/usd/sdf/path.h>
#include "opendcc/app/core/interval_vector.h"

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API SelectionData
{
public:
    using IndexType = uint32_t;
    using IndexIntervals = IntervalVector<IndexType>;

private:
    friend class SelectionList;
    IndexIntervals m_instance_indices;
    IndexIntervals m_element_indices;
    IndexIntervals m_edge_indices;
    IndexIntervals m_point_indices;
    std::unordered_set<PXR_NS::TfToken, PXR_NS::TfHash> m_properties;
    mutable uint64_t m_id = 0;
    static uint64_t s_global_id;
    bool m_prim_selected = false;

    uint64_t get_id() const;
    void increment_id() const;

    void set_fully_selected(bool fully_selected);
    void set_point_indices(const IndexIntervals& point_indices);
    void set_edge_indices(const IndexIntervals& edge_indices);
    void set_element_indices(const IndexIntervals& element_indices);
    void set_instance_indices(const IndexIntervals& instance_indices);
    void set_properties(const std::unordered_set<PXR_NS::TfToken, PXR_NS::TfHash>& properties);

    void add_point_indices(const IndexIntervals& point_indices);
    void add_edge_indices(const IndexIntervals& edge_indices);
    void add_element_indices(const IndexIntervals& element_indices);
    void add_instance_indices(const IndexIntervals& instance_indices);
    void add_properties(const std::unordered_set<PXR_NS::TfToken, PXR_NS::TfHash>& properties);

    void remove_point_indices(const IndexIntervals& point_indices);
    void remove_edge_indices(const IndexIntervals& edge_indices);
    void remove_element_indices(const IndexIntervals& element_indices);
    void remove_instance_indices(const IndexIntervals& instance_indices);
    void remove_properties(const std::unordered_set<PXR_NS::TfToken, PXR_NS::TfHash>& properties);

public:
    SelectionData(bool prim_selection = false, const std::vector<IndexType>& point_indices = {}, const std::vector<IndexType>& edge_indices = {},
                  const std::vector<IndexType>& element_indices = {}, const std::vector<IndexType>& instance_indices = {},
                  const std::unordered_set<PXR_NS::TfToken, PXR_NS::TfHash>& properties = {});

    SelectionData(bool prim_selection, const PXR_NS::VtArray<IndexType>& point_indices, const PXR_NS::VtArray<IndexType>& edge_indices,
                  const PXR_NS::VtArray<IndexType>& element_indices, const PXR_NS::VtArray<IndexType>& instance_indices,
                  const PXR_NS::VtArray<PXR_NS::TfToken>& properties);

    SelectionData(bool prim_selection, const IndexIntervals& point_indices, const IndexIntervals& edge_indices, const IndexIntervals& element_indices,
                  const IndexIntervals& instance_indices, const PXR_NS::VtArray<PXR_NS::TfToken>& properties);

    bool empty() const;
    bool is_fully_selected() const;
    IndexIntervals::RangeProxy get_point_indices() const;
    const IndexIntervals& get_point_index_intervals() const;
    IndexIntervals::RangeProxy get_edge_indices() const;
    const IndexIntervals& get_edge_index_intervals() const;
    IndexIntervals::RangeProxy get_element_indices() const;
    const IndexIntervals& get_element_index_intervals() const;
    IndexIntervals::RangeProxy get_instance_indices() const;
    const IndexIntervals& get_instance_index_intervals() const;
    const std::unordered_set<PXR_NS::TfToken, PXR_NS::TfHash>& get_properties() const;

    bool operator==(const SelectionData& data) const;
    bool operator!=(const SelectionData& data) const;
};

/**
 * @brief A class that contains data about selected Prim and their components.
 *
 * The SelectionList class has methods that allow various
 * modifications of the stored data,
 * as well as extracting information about selected components
 * by mask or by Prim path.
 * This class is implemented using the copy-on-write technique.
 */
class OPENDCC_API SelectionList
{
public:
    enum SelectionFlags
    {
        NONE,
        POINTS = 1 << 0,
        EDGES = 1 << 1,
        ELEMENTS = 1 << 2,
        INSTANCES = 1 << 3,
        PROPERTIES = 1 << 4,
        FULL_SELECTION = 1 << 5,
        ALL = POINTS | EDGES | ELEMENTS | INSTANCES | PROPERTIES | FULL_SELECTION
    };
    using SelectionMask = uint32_t;
    using IndexType = SelectionData::IndexType;

    using SelectionMap = std::unordered_map<PXR_NS::SdfPath, SelectionData, PXR_NS::SdfPath::Hash>;
    using iterator = SelectionMap::iterator;
    using const_iterator = SelectionMap::const_iterator;

    /**
     * @brief Initializes an empty selection list.
     *
     */
    SelectionList();
    /**
     * @brief Initializes a selection list with a set of prim paths.
     *
     * @param selected_paths The paths to be added to the selection list.
     */
    SelectionList(const PXR_NS::SdfPathVector& selected_paths);
    /**
     * @brief Initializes a selection list with a selection map.
     *
     * @param sel_map The prim paths and their selection data.
     */
    SelectionList(const SelectionMap& sel_map);

    /**
     * @brief Returns a const iterator pointing to the beginning of the prim selection map.
     *
     */
    const_iterator begin() const { return m_data->m_prim_selection_map.begin(); }
    /**
     * @brief Returns a const iterator pointing to the end of the prim selection map.
     *
     */
    const_iterator end() const { return m_data->m_prim_selection_map.end(); }
    /**
     * @brief Returns an iterator pointing to the beginning of the prim selection map.
     *
     */
    iterator begin() { return m_data->m_prim_selection_map.begin(); }
    /**
     * @brief Returns an iterator pointing to the end of the prim selection map.
     *
     */
    iterator end() { return m_data->m_prim_selection_map.end(); }

    /**
     * @brief Gets the selection data at the specified path.
     *
     * @param path The path from which to retrieve the data.
     * @return The selection data.
     */
    const SelectionData& operator[](const PXR_NS::SdfPath& path) const { return get_selection_data(path); }

    /**
     * @brief Gets the fully selected prim paths.
     *
     * @return The fully selected prim paths.
     */
    PXR_NS::SdfPathVector get_fully_selected_paths() const;
    /**
     * @brief Gets the prim paths with any selected components.
     *
     * @return The prim paths with any selected components.
     */
    const PXR_NS::SdfPathVector& get_selected_paths() const;
    /**
     * @brief Adds the specified prim paths to the selection list.
     *
     * @param paths The prim paths to add to the selection list.
     */
    void add_prims(const PXR_NS::SdfPathVector& paths);
    /**
     * @brief Removes the specified prim paths from the selection list.
     *
     * @param paths The prim paths to remove from the selection list.
     */
    void remove_prims(const PXR_NS::SdfPathVector& paths);

    /**
     * @brief Adds the points to the selection list using the point indices.
     *
     * @param path The path to the prim which contains the components to be added to the selection list.
     * @param indices The indices of the components to be added to the selection list.
     */
    template <class TCollection>
    void add_points(const PXR_NS::SdfPath& path, const TCollection& indices)
    {
        add_subprims(path, IntervalVector<IndexType>::from_collection(indices), &SelectionData::add_point_indices);
    }

    /**
     * @brief Adds the edges to the selection list using their indices.
     *
     * @param path The path to the prim containing the components to be added to the selection list.
     * @param indices The indices of the components to be added to the selection list.
     */
    template <class TCollection>
    void add_edges(const PXR_NS::SdfPath& path, const TCollection& indices)
    {
        add_subprims(path, IntervalVector<IndexType>::from_collection(indices), &SelectionData::add_edge_indices);
    }

    /**
     * @brief Adds the elements to the selection list using the element indices.
     *
     * @param path The path to the prim containing the components to be added to the selection list.
     * @param indices The indices of the components to be added to the selection list.
     */
    template <class TCollection>
    void add_elements(const PXR_NS::SdfPath& path, const TCollection& indices)
    {
        add_subprims(path, IntervalVector<IndexType>::from_collection(indices), &SelectionData::add_element_indices);
    }

    /**
     * @brief Adds the instances to the selection list using a collection of instance indices.
     *
     * @param path The path to the prim containing the components to be added to the selection list.
     * @param indices A collection containing the indices of the components to be added to the selection list.
     */
    template <class TCollection>
    void add_instances(const PXR_NS::SdfPath& path, const TCollection& indices)
    {
        add_subprims(path, IntervalVector<IndexType>::from_collection(indices), &SelectionData::add_instance_indices);
    }
    /**
     * @brief Adds the specified prim properties to the selection list.
     *
     * @param path The path to the prim containing the properties to be added to the selection list.
     * @param properties The properties to be added to the selection list.
     */
    void add_properties(const PXR_NS::SdfPath& path, const std::unordered_set<PXR_NS::TfToken, PXR_NS::TfHash>& properties)
    {
        add_subprims(path, properties, &SelectionData::add_properties);
    }

    /**
     * @brief Adds the specified set of prim properties to the selection list.
     *
     * @param path The path to the prim containing the properties to be added to the selection list.
     * @param properties The properties to be added to the selection list.
     */
    void add_properties(const PXR_NS::SdfPath& path, const PXR_NS::VtTokenArray& properties)
    {
        std::unordered_set<PXR_NS::TfToken, PXR_NS::TfHash> props_set;
        props_set.insert(properties.begin(), properties.end());
        add_subprims(path, std::move(props_set), &SelectionData::add_properties);
    }

    /**
     * @brief Removes the points from the selection list using the point indices.
     *
     * @param path The path to the prim containing the components to be removed from the selection list.
     * @param indices The indices of the components to be removed from the selection list.
     */
    template <class TCollection>
    void remove_points(const PXR_NS::SdfPath& path, const TCollection& indices)
    {
        remove_subprims(path, IntervalVector<IndexType>::from_collection(indices), &SelectionData::remove_point_indices);
    }

    /**
     * @brief Removes the edges from the selection list using the edge indices.
     *
     * @param path The path to the prim containing the components to be removed from the selection list.
     * @param indices The components to be removed from the selection list.
     */
    template <class TCollection>
    void remove_edges(const PXR_NS::SdfPath& path, const TCollection& indices)
    {
        remove_subprims(path, IntervalVector<IndexType>::from_collection(indices), &SelectionData::remove_edge_indices);
    }
    /**
     * @brief Removes the elements from the selection list using the element indices.
     *
     * @param path The path to the prim containing the components to be removed from the selection list.
     * @param indices The components to be removed from the selection list.
     */
    template <class TCollection>
    void remove_elements(const PXR_NS::SdfPath& path, const TCollection& indices)
    {
        remove_subprims(path, IntervalVector<IndexType>::from_collection(indices), &SelectionData::remove_element_indices);
    }
    /**
     * @brief Removes the instances from the selection list using the instance indices.
     *
     * @param path The path to the prim containing the components to be removed from the selection list.
     * @param indices The indices of the components to be removed from the selection list.
     */
    template <class TCollection>
    void remove_instances(const PXR_NS::SdfPath& path, const TCollection& indices)
    {
        remove_subprims(path, IntervalVector<IndexType>::from_collection(indices), &SelectionData::remove_instance_indices);
    }

    /**
     * @brief Removes the specified prim properties from the selection list.
     *
     * @param path The path to the prim containing the properties to be removed from the selection list.
     * @param properties The properties to be removed from the selection list.
     */
    void remove_properties(const PXR_NS::SdfPath& path, const std::unordered_set<PXR_NS::TfToken, PXR_NS::TfHash>& properties)
    {
        remove_subprims(path, properties, &SelectionData::remove_properties);
    }
    /**
     * @brief Removes the specified set of prim properties from the selection list.
     *
     * @param path The path to the prim containing the properties to be removed from the selection list.
     * @param properties A set of properties to be removed from the selection list.
     */
    void remove_properties(const PXR_NS::SdfPath& path, const PXR_NS::VtTokenArray& properties)
    {
        std::unordered_set<PXR_NS::TfToken, PXR_NS::TfHash> props_set;
        props_set.insert(properties.begin(), properties.end());
        remove_subprims(path, std::move(props_set), &SelectionData::remove_properties);
    }

    /**
     * @brief Sets full selection for the specified prim.
     *
     * @param path The path to the prim for which to set the full selection.
     * @param full_selection The full selection state.
     *
     * @note Changing the full selection state does not affect
     * any of the currently selected components.
     *
     * If the specified prim hasn't any selected components, on changing
     * the full selection state it is set to true or false, which accordingly
     * adds or removes the prim path from the selection list.
     */
    void set_full_selection(const PXR_NS::SdfPath& path, bool full_selection);

    /**
     * @brief Merges the specified selection list with the current using a selection mask.
     *
     * @param selection_list The selection list to be merged.
     * @param merge_mask The mask which defines the component types to be merged.
     */
    void merge(const SelectionList& selection_list, SelectionMask merge_mask = SelectionFlags::ALL);
    /**
     * @brief Subtracts the selected components from the selection list using a selection mask.
     *
     * @param selection_list The selection list containing selection state that needs to be subtracted.
     * @param merge_mask The mask, using which the component types need to be subtracted.
     */
    void difference(const SelectionList& selection_list, SelectionMask merge_mask = SelectionFlags::ALL);
    /**
     * @brief Updates the selection list replacing its contents with
     * the values from the specified selection list.
     *
     * @param selection_list The selection list containing selection state that needs to be updated.
     * @param mask The mask, using which the component types need to be updated.
     */
    void update(const SelectionList& selection_list, SelectionMask mask = SelectionFlags::ALL);
    /**
     * @brief Extracts the components to a new selection list using a selection mask.
     *
     * @param mask The type of components to be extracted.
     */
    SelectionList extract(SelectionMask mask = SelectionFlags::ALL) const;
    /**
     * @brief Extracts the components to a new selection list
     * by the specified paths using a selection mask.
     *
     * @param paths The paths from which to extract the selection data
     * @param mask The type of components to be returned.
     */
    SelectionList extract(const PXR_NS::SdfPathVector& paths, SelectionMask mask = SelectionFlags::ALL) const;
    /**
     * @brief Returns the number of the prims with full selection.
     *
     */
    size_t fully_selected_paths_size() const;
    /**
     * @brief Returns the number of prims with any selection.
     *
     */
    size_t size() const;
    /**
     * @brief Checks whether the selection list is empty.
     *
     */
    bool empty() const;
    /**
     * @brief Checks whether the selection list contains
     * the specified prim.
     *
     * @param path The path to the prim to be checked.
     */
    bool contains(const PXR_NS::SdfPath& path) const;

    /**
     * @brief Adds the specified paths to the selection
     * list setting the full selection state for them.
     *
     * @param new_selection The paths to add to the selection list.
     */
    void set_selected_paths(const PXR_NS::SdfPathVector& new_selection);
    /**
     * @brief Clears the selection list.
     *
     */
    void clear();
    /**
     * @brief Gets the selection data of the specified prim.
     *
     * @param prim_path The prim path at which to retrieve the selection data.
     */
    const SelectionData& get_selection_data(const PXR_NS::SdfPath& prim_path) const;
    /**
     * @brief Sets the specified selection data for the prim path.
     *
     * @param prim_path The prim path at which to set the selection data.
     * @param selection_data The selection data to set.
     */
    void set_selection_data(const PXR_NS::SdfPath& prim_path, const SelectionData& selection_data);
    /**
     * @brief Sets the specified selection data for the prim path.
     *
     * @param path The prim path at which to set the selection data.
     * @param selection_data The selection data to set.
     * Takes ownership over the selection data.
     * The later using is unsafe.
     *
     */
    void set_selection_data(const PXR_NS::SdfPath& path, SelectionData&& selection_data);
    /**
     * @brief Checks whether the selection lists are equal.
     *
     * @param other The selection list to compare with.
     */
    bool equals(const SelectionList& other) const;
    /**
     * @brief The equality operator for the selection list.
     *
     * @param other The selection list to compare with.
     * @return true if the specified selection list and
     * the current selection list are equal, false otherwise.
     */
    bool operator==(const SelectionList& other) const;
    /**
     * @brief The inequality operator for the selection list.
     *
     * @param other The selection list to compare with.
     * @return true if the specified selection list and
     * the current selection list are inequal, false otherwise.
     */
    bool operator!=(const SelectionList& other) const;

private:
    void try_detach();
    void mark_selected_paths_dirty();

    template <class TSrcCollection, class TInserter>
    void add_subprims(const PXR_NS::SdfPath& path, TSrcCollection&& new_indices, TInserter inserter)
    {
        if (new_indices.empty())
            return;
        try_detach();

        auto iter = m_data->m_prim_selection_map.find(path);
        if (iter != m_data->m_prim_selection_map.end())
        {
            (iter->second.*inserter)(std::forward<TSrcCollection>(new_indices));
        }
        else if (!new_indices.empty())
        {
            SelectionData data(false);
            (data.*inserter)(std::forward<TSrcCollection>(new_indices));
            m_data->m_prim_selection_map.emplace(path, std::move(data));
        }
    }

    template <class TSrcCollection, class TEraser>
    void remove_subprims(const PXR_NS::SdfPath& path, TSrcCollection&& indices, TEraser eraser)
    {
        if (indices.empty())
            return;
        try_detach();

        auto iter = m_data->m_prim_selection_map.find(path);
        if (iter == m_data->m_prim_selection_map.end())
            return;

        (iter->second.*eraser)(std::forward<TSrcCollection>(indices));

        if (iter->second.empty())
        {
            m_data->m_prim_selection_map.erase(iter);
        }
    }

    struct SelectionListData
    {
        SelectionMap m_prim_selection_map;
        PXR_NS::SdfPathVector m_selected_paths;
    };

    std::shared_ptr<SelectionListData> m_data;
};

// Temporary solution because currently add_points cannot accept sorted containers.
template <>
inline void SelectionList::add_points<std::set<int>>(const PXR_NS::SdfPath& path, const std::set<int>& indices)
{
    add_subprims(path, IntervalVector<IndexType>::from_sorted_collection(indices), &SelectionData::add_point_indices);
}

// Temporary solution because currently add_edges cannot accept sorted containers.
template <>
inline void SelectionList::add_edges<std::set<int>>(const PXR_NS::SdfPath& path, const std::set<int>& indices)
{
    add_subprims(path, IntervalVector<IndexType>::from_sorted_collection(indices), &SelectionData::add_edge_indices);
}

// Temporary solution because currently add_elements cannot accept sorted containers.
template <>
inline void SelectionList::add_elements<std::set<int>>(const PXR_NS::SdfPath& path, const std::set<int>& indices)
{
    add_subprims(path, IntervalVector<IndexType>::from_sorted_collection(indices), &SelectionData::add_element_indices);
}

OPENDCC_NAMESPACE_CLOSE
