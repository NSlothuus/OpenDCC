/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"

#include <memory>

OPENDCC_NAMESPACE_OPEN

class PrimMaterialOverride;

class OPENDCC_API PersistentMaterialOverride
{
private:
    PersistentMaterialOverride();

    PersistentMaterialOverride(const PersistentMaterialOverride&) = delete;
    PersistentMaterialOverride(PersistentMaterialOverride&&) = delete;
    PersistentMaterialOverride& operator=(const PersistentMaterialOverride&) = delete;
    PersistentMaterialOverride& operator=(PersistentMaterialOverride&&) = delete;

public:
    static PersistentMaterialOverride& instance();

    std::shared_ptr<PrimMaterialOverride> get_override();

private:
    std::shared_ptr<PrimMaterialOverride> m_override;
};

OPENDCC_NAMESPACE_CLOSE
