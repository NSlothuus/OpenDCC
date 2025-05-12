// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/core/undo/router.h"
#include "opendcc/app/core/undo/state_delegate.h"

#include <pxr/base/tf/instantiateSingleton.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

using namespace commands;

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UndoStackNotice, TfType::Bases<TfNotice>>();
}

PXR_NAMESPACE_CLOSE_SCOPE

std::unique_ptr<UndoInverse> UndoRouter::take_inversions()
{
    return std::make_unique<UndoInverse>(std::move(instance().m_inversion));
}

bool UndoRouter::is_muted()
{
    return instance().m_mute_depth > 0;
}

UndoInverse& UndoRouter::get_inversions()
{
    return instance().m_inversion;
}

uint32_t UndoRouter::get_depth()
{
    return instance().m_depth;
}

void UndoRouter::add_inverse(std::shared_ptr<Edit> inverse)
{
    UsdEditsUndoBlock undo_block;
    instance().m_inversion.add(inverse);
}

UndoRouter& UndoRouter::instance()
{
    static UndoRouter router;
    return router;
}

OPENDCC_NAMESPACE_CLOSE
