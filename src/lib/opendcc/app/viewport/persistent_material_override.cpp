// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/persistent_material_override.h"

#include "opendcc/app/viewport/prim_material_override.h"

OPENDCC_NAMESPACE_OPEN

PersistentMaterialOverride::PersistentMaterialOverride()
{
    m_override = std::make_shared<PrimMaterialOverride>();
}

PersistentMaterialOverride& PersistentMaterialOverride::instance()
{
    static PersistentMaterialOverride instance;
    return instance;
}

std::shared_ptr<PrimMaterialOverride> PersistentMaterialOverride::get_override()
{
    return m_override;
}

OPENDCC_NAMESPACE_CLOSE