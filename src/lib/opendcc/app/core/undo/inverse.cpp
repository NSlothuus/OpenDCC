// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/opendcc.h"
#include "opendcc/app/core/undo/inverse.h"
#include "opendcc/app/core/undo/block.h"
#include "opendcc/app/core/undo/router.h"
#include <pxr/usd/sdf/changeBlock.h>

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE
using namespace commands;

void UndoInverse::invert_impl()
{
    SdfChangeBlock change_block;
    for (auto it = m_inversions.rbegin(); it != m_inversions.rend(); ++it)
    {
        (*(*it))();
    }
    m_inversions.clear();
}

commands::UndoInverse::UndoInverse(UndoInverse&& inverse) noexcept
    : m_inversions(std::move(inverse.m_inversions))
{
}

UndoInverse& commands::UndoInverse::operator=(UndoInverse&& inverse) noexcept
{
    if (&inverse == this)
    {
        return *this;
    }

    m_inversions = std::move(inverse.m_inversions);
    return *this;
}

void UndoInverse::add(std::shared_ptr<Edit> inversion)
{
    if (!m_inversions.empty())
    {
        if (m_inversions.back()->get_edit_type_id() == inversion->get_edit_type_id() && m_inversions.back()->merge_with(inversion.get()))
            return;
    }
    m_inversions.push_back(inversion);
}

void UndoInverse::invert()
{
    if (UndoRouter::get_depth() != 0)
    {
        TF_CODING_ERROR("Inversion during open edit block may result in corrupted undo "
                        "stack.");
    }

    UsdEditsUndoBlock edit_block;
    invert_impl();

    move_inversions(UndoRouter::get_inversions());
}

void UndoInverse::move_inversions(UndoInverse& inverse)
{
    for (int i = 0; i < inverse.m_inversions.size(); i++)
    {
        add(inverse.m_inversions[i]);
    }
    inverse.clear();
}

size_t UndoInverse::size() const
{
    return m_inversions.size();
}

void UndoInverse::clear() noexcept
{
    m_inversions.clear();
}
OPENDCC_NAMESPACE_CLOSE
