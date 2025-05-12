/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include <pxr/imaging/hd/filteringSceneIndex.h>
#include <pxr/imaging/hd/sceneIndexPlugin.h>

OPENDCC_NAMESPACE_OPEN

class PruneSceneIndexPlugin : public PXR_NS::HdSceneIndexPlugin
{
public:
    PXR_NS::HdSceneIndexBaseRefPtr _AppendSceneIndex(const PXR_NS::HdSceneIndexBaseRefPtr &input_scene,
                                                     const PXR_NS::HdContainerDataSourceHandle &input_args) override;
};

OPENDCC_NAMESPACE_CLOSE
