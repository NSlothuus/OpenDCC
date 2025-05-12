/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/app/core/selection_list.h"

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/base/gf/vec2i.h>
#include <pxr/usd/usdGeom/mesh.h>

#include <tuple>
#include <memory>
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN

class HalfEdge;
using HalfEdgePtr = std::shared_ptr<HalfEdge>;
class OPENDCC_API HalfEdge
{
public:
    HalfEdge();
    ~HalfEdge();

    static HalfEdgePtr from_mesh(const PXR_NS::UsdGeomMesh& mesh, PXR_NS::UsdTimeCode time);

    SelectionList edge_loop_selection(const PXR_NS::GfVec2i& begin);
    SelectionList grow_selection(const SelectionList& current);
    SelectionList decrease_selection(const SelectionList& current);
    SelectionList topology_selection(const SelectionList& current);

private:
    class HalfEdgeImpl;
    using HalfEdgeImplPtr = std::unique_ptr<HalfEdgeImpl>;
    HalfEdgeImplPtr m_impl;
};

class OPENDCC_API HalfEdgeCache
{
public:
    HalfEdgeCache();
    ~HalfEdgeCache();

    HalfEdgePtr get_half_edge(const PXR_NS::UsdPrim& prim, PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default());
    bool contains(const PXR_NS::UsdPrim& prim, PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default());

    void clear_at_time(const PXR_NS::UsdPrim& prim, PXR_NS::UsdTimeCode time);
    void clear_timesamples(const PXR_NS::UsdPrim& prim);
    void clear();

private:
    void update(const PXR_NS::UsdPrim& prim, PXR_NS::UsdTimeCode time);

    using MeshSamples = std::unordered_map<PXR_NS::UsdTimeCode, HalfEdgePtr, PXR_NS::TfHash>;
    using PrimCache = std::unordered_map<PXR_NS::UsdPrim, MeshSamples, PXR_NS::TfHash>;
    PrimCache m_cache;
};

OPENDCC_NAMESPACE_CLOSE
