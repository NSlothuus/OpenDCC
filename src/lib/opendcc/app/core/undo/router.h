/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/export.h"
#include "opendcc/app/core/undo/inverse.h"
#include "opendcc/app/core/undo/block.h"

#include <pxr/usd/sdf/layer.h>

OPENDCC_NAMESPACE_OPEN

namespace commands
{
    class OPENDCC_API UndoRouter : boost::noncopyable
    {
    private:
        friend class UsdEditsUndoBlock;
        friend class UsdEditsBlock;
        UndoRouter() = default;

        UndoInverse m_inversion;
        int32_t m_depth = 0;
        int32_t m_mute_depth = 0;

        static std::unique_ptr<UndoInverse> take_inversions();

    public:
        static bool is_muted();
        static UndoInverse& get_inversions();

        static uint32_t get_depth();
        static void add_inverse(std::shared_ptr<Edit> inverse);

        static UndoRouter& instance();
    };

    class UndoStackNotice : public PXR_NS::TfNotice
    {
    public:
        explicit UndoStackNotice(std::unique_ptr<UndoInverse>&& inverse)
            : m_inverse(std::move(inverse)) {};

        const std::shared_ptr<UndoInverse>& get_inverse() const { return m_inverse; }

    private:
        std::shared_ptr<UndoInverse> m_inverse;
    };

} // namespace commands
OPENDCC_NAMESPACE_CLOSE
