/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "api.h"
#include <memory>
#include <unordered_set>
#include "pxr/usd/usd/prim.h"
#include "pxr/base/gf/transform.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "opendcc/usd_editor/bullet_physics/engine.h"

class btCollisionShape;
class btTransform;

OPENDCC_NAMESPACE_OPEN

bool BULLET_PHYSICS_API is_supported_type(const PXR_NS::UsdPrim& type);
std::unique_ptr<btCollisionShape> BULLET_PHYSICS_API create_collision_shape(const PXR_NS::UsdPrim& prim, BulletPhysicsEngine::BodyType type,
                                                                            BulletPhysicsEngine::MeshApproximationType mesh_approximation_type,
                                                                            PXR_NS::UsdTimeCode time_code);
void BULLET_PHYSICS_API reset_pivots(const PXR_NS::SdfPathVector& paths);
PXR_NS::SdfPathVector BULLET_PHYSICS_API get_supported_prims_paths_recursively(const PXR_NS::UsdStageRefPtr& stage,
                                                                               const PXR_NS::SdfPathVector& paths);
void BULLET_PHYSICS_API usd_transform_to_bullet(const PXR_NS::GfTransform& from, btTransform& to);
void BULLET_PHYSICS_API update_children(PXR_NS::UsdPrim prim, btCollisionShape* shape,
                                        const std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash>& components);

OPENDCC_NAMESPACE_CLOSE
