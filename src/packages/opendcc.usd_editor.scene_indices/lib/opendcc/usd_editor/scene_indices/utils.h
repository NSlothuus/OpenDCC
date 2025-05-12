/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include <pxr/imaging/hd/sceneIndex.h>
#if PXR_VERSION >= 2408
#include <pxr/imaging/hd/sceneIndexPrimView.h>
#endif

OPENDCC_NAMESPACE_OPEN

#if PXR_VERSION < 2408
class PrimView
{
public:
    class const_iterator
    {
    public:
        const const_iterator &operator++()
        {
            if (m_skip_descendants)
            {
                m_skip_descendants = false;
            }
            else
            {
                if (auto children = m_scene->GetChildPrimPaths(**this); !children.empty())
                {
                    m_stack.push_back({ std::move(children), 0 });
                    return *this;
                }
            }

            while (!m_stack.empty())
            {
                auto &entry = m_stack.back();
                ++entry.index;
                if (entry.index < entry.paths.size())
                {
                    break;
                }
                m_stack.pop_back();
            }

            return *this;
        }
        const PXR_NS::SdfPath &operator*() const
        {
            const auto &entry = m_stack.back();
            return entry.paths[entry.index];
        }
        bool operator==(const const_iterator &other) const { return m_stack == other.m_stack; }
        bool operator!=(const const_iterator &other) const { return !(*this == other); }

        void SkipDescendants() { m_skip_descendants = true; }

    private:
        friend class PrimView;
        struct StackEntry
        {
            std::vector<PXR_NS::SdfPath> paths;
            size_t index = 0;
            bool operator==(const StackEntry &other) const { return paths == other.paths && index == other.index; }
            bool operator!=(const StackEntry &other) const { return !(*this == other); }
        };
        const_iterator(const PXR_NS::HdSceneIndexBaseRefPtr &input_scene, const PXR_NS::SdfPath &root)
            : m_scene(input_scene)
            , m_stack { { { root }, 0 } }
            , m_skip_descendants { false }
        {
        }
        const_iterator(const PXR_NS::HdSceneIndexBaseRefPtr &input_scene)
            : m_scene(input_scene)
            , m_skip_descendants { false }
        {
        }

        PXR_NS::HdSceneIndexBaseRefPtr m_scene;
        std::vector<StackEntry> m_stack;
        bool m_skip_descendants = false;
    };

    PrimView(const PXR_NS::HdSceneIndexBaseRefPtr &input_scene)
        : PrimView(input_scene, PXR_NS::SdfPath::AbsoluteRootPath())
    {
    }
    PrimView(const PXR_NS::HdSceneIndexBaseRefPtr &input_scene, const PXR_NS::SdfPath &subroot)
        : m_begin { input_scene, subroot }
        , m_end { input_scene }
    {
    }

    const const_iterator &begin() const { return m_begin; }
    const const_iterator &end() const { return m_end; }

private:
    const const_iterator m_begin;
    const const_iterator m_end;
};
#else
using PrimView = PXR_NS::HdSceneIndexPrimView;
#endif

OPENDCC_NAMESPACE_CLOSE
