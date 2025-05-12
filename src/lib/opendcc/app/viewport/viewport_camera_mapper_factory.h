/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <pxr/base/tf/token.h>
#include "opendcc/app/viewport/viewport_camera_mapper.h"
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API ViewportCameraMapperFactory
{
public:
    static void register_camera_mapper(const PXR_NS::TfToken& name, std::function<ViewportCameraMapperPtr()> mapper_creator);
    static void unregister_camera_mapper(const PXR_NS::TfToken& name);

    static ViewportCameraMapperPtr create_camera_mapper(const PXR_NS::TfToken& name);

private:
    static ViewportCameraMapperFactory& get_instance();
    ViewportCameraMapperFactory() = default;
    ViewportCameraMapperFactory(const ViewportCameraMapperFactory&) = delete;
    ViewportCameraMapperFactory(ViewportCameraMapperFactory&&) = delete;
    ViewportCameraMapperFactory& operator=(const ViewportCameraMapperFactory&) = delete;
    ViewportCameraMapperFactory& operator=(ViewportCameraMapperFactory&&) = delete;

    std::unordered_map<PXR_NS::TfToken, std::function<ViewportCameraMapperPtr()>, PXR_NS::TfToken::HashFunctor> m_registry;
};

OPENDCC_NAMESPACE_CLOSE