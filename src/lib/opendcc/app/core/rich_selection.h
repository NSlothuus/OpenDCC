/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/selection_list.h"
#include "opendcc/app/core/point_cloud_bvh.h"
#include <tbb/concurrent_unordered_map.h>
#include "opendcc/app/core/session.h"

OPENDCC_NAMESPACE_OPEN
/**
 * @brief The RichSelection class is a SelectionList extension
 * which provides additional features for the soft selection.
 * Besides the selected components (vertces, faces, edges), it contains
 * the corresponding values of their weight.
 *
 * Moreover, this class allows to iterate over the containing data as an unordered map
 * where the key is a prim path and the value is a weight map.
 *
 */
class OPENDCC_API RichSelection
{
public:
    /**
     * @brief A weight map which uses prim's id as a key and weight as a value.
     *
     */
    using WeightMap = tbb::concurrent_unordered_map<SelectionList::IndexType, float>; // std::unordered_map<SelectionList::IndexType, float>;
    /**
     * Assigns weight for the specified vertex.
     * This function receives vertex distance and returns a weight for it.
     *
     */
    using WeightFn = std::function<float(float dist)>;
    /**
     * @brief A color assignment function which receives
     * the vertex weight value and returns an RGB color to assign to this vertex.
     *
     */
    using ColorFn = std::function<PXR_NS::GfVec3f(float weight)>;
    using iterator = std::unordered_map<PXR_NS::SdfPath, WeightMap, PXR_NS::SdfPath::Hash>::iterator;
    using const_iterator = std::unordered_map<PXR_NS::SdfPath, WeightMap, PXR_NS::SdfPath::Hash>::const_iterator;

    RichSelection();
    ~RichSelection() = default;

    /**
     * @brief Constructs a new Rich Selection object with the specified functions of weight and color assignment.
     *
     * @param weight_fn A weight assignment function.
     * @param color_fn A color assignment function.
     */
    RichSelection(WeightFn weight_fn, ColorFn color_fn = ColorFn());
    /**
     * @brief Gets the weights of the specified prim.
     *
     * @param prim_path Reference to the prim from which to get the weights.
     * @return An unordered weight map reference.
     */
    const WeightMap& get_weights(const PXR_NS::SdfPath& prim_path) const;
    /**
     * @brief The start iterator for the containing collection.
     *
     */
    iterator begin();
    /**
     * @brief The end iterator for the containing collection.
     *
     */
    iterator end();
    /**
     * @brief The const start iterator for the containing collection.
     *
     */
    const_iterator begin() const;
    /**
     * @brief The const end iterator for the containing collection.
     *
     */
    const_iterator end() const;
    /**
     * @brief Clears all the containing data.
     *
     */
    void clear();

    /**
     * @brief Sets the specified prims to the SelectionList, then evaluates their weights.
     *
     * The information of the previous SelectionList will be replaced.
     *
     * @param selection The list of prims to add to the SelectionList.
     *
     * @note The information about the prims and their topology is received from the current stage.
     */
    void set_soft_selection(const SelectionList& selection);
    /**
     * @brief Returns an RGB color for the specified weight.
     *
     * @param weight A weight value to return the RGB color for.
     */
    PXR_NS::GfVec3f get_soft_selection_color(float weight) const;
    /**
     * @brief Gets the containing raw SelectionList without any weight data.
     *
     */
    const SelectionList& get_selection_list() const { return m_data->m_selection_list; }
    /**
     * @brief Checks whether the object has color data.
     *
     * @return true if the color data is presented, false if not found.
     */
    bool has_color_data() const;
    /**
     * @brief Re-evaluates the weight data of the stored SelectionList.
     *
     * @note The information about the prims and their topology is received from the current stage.
     *
     */
    void update();

private:
    struct RichSelectionData
    {
        WeightFn m_weight_fn;
        ColorFn m_color_fn;
        SelectionList m_selection_list;
        std::shared_ptr<PointCloudBVH> m_bvh;
        Session::StageChangedCallbackHandle m_callback_handle;
        ~RichSelectionData();

        std::unordered_map<PXR_NS::SdfPath, WeightMap, PXR_NS::SdfPath::Hash> m_per_prim_weights;
    };
    std::shared_ptr<RichSelectionData> m_data;
};

OPENDCC_NAMESPACE_CLOSE
