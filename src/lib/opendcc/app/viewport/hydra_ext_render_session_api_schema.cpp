// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/hydra_ext_render_session_api_schema.h"
#include "hydra_render_session_api/tokens.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

HdTokenDataSourceHandle HydraExtRenderSessionAPISchema::get_render_delegate() const
{
    return _GetTypedDataSource<HdTokenDataSource>(UsdHydraExtTokens->render_delegate);
}

/*static*/
HydraExtRenderSessionAPISchema HydraExtRenderSessionAPISchema::get_from_parent(const HdContainerDataSourceHandle &from_parent_container)
{
    return HydraExtRenderSessionAPISchema(from_parent_container ? HdContainerDataSource::Cast(from_parent_container->Get(get_schema_token()))
                                                                : nullptr);
}

/*static*/
const TfToken &HydraExtRenderSessionAPISchema::get_schema_token()
{
    return UsdHydraExtTokens->HydraRenderSessionAPI;
}

/*static*/
const HdDataSourceLocator &HydraExtRenderSessionAPISchema::get_default_locator()
{
    static const HdDataSourceLocator locator(get_schema_token());
    return locator;
}

OPENDCC_NAMESPACE_CLOSE
