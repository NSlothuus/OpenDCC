/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/export.h"
#include "opendcc/app/core/api.h"

#include <functional>
#include <vector>
#include <memory>

OPENDCC_NAMESPACE_OPEN

namespace commands
{
    class Edit
    {
    public:
        Edit() = default;
        virtual ~Edit() = default;
        virtual bool operator()() = 0;
        virtual bool merge_with(const Edit* other) = 0;
        virtual size_t get_edit_type_id() const = 0;
    };

    template <class EditType>
    size_t get_edit_type_id()
    {
        static bool type_id;
        return (size_t)(&type_id);
    }

    class OPENDCC_API UndoInverse
    {
    private:
        std::vector<std::shared_ptr<Edit>> m_inversions;

        void invert_impl();

    public:
        UndoInverse() = default;
        UndoInverse(const UndoInverse& inverse) = delete;
        UndoInverse(UndoInverse&& inverse) noexcept;
        UndoInverse& operator=(const UndoInverse&) = delete;
        UndoInverse& operator=(UndoInverse&& inverse) noexcept;
        void add(std::shared_ptr<Edit> inversion);
        void invert();
        void move_inversions(UndoInverse& inverse);
        size_t size() const;
        void clear() noexcept;
    };
}

OPENDCC_NAMESPACE_CLOSE
