/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>

#include <pxr/base/gf/vec3f.h>

#include "opendcc/app/viewport/iviewport_tool_context.h"
#include "opendcc/app/viewport/viewport_view.h"

class Scope
{
public:
    Scope(std::function<void()> do_it)
        : _do_it(do_it)
    {
    }
    ~Scope() { _do_it(); }

private:
    std::function<void()> _do_it;
};

#define DEBUG_MESSAGE(message) OPENDCC_INFO(message);
#define TIMER_START(timer_name) auto timer_name##_start = tbb::tick_count::now()
#define TIMER_END(timer_name) OPENDCC_INFO(std::string(#timer_name) + " " + std::to_string((tbb::tick_count::now() - timer_name##_start).seconds()));
#define TIMER_SCOPE(timer_name)               \
    auto timer_name = tbb::tick_count::now(); \
    Scope timer_name##_scope(                 \
        [&]() { OPENDCC_INFO(std::string(#timer_name) + " " + std::to_string((tbb::tick_count::now() - timer_name).seconds())); });

#ifdef NO_PARALLEL
#define PARALLEL_FOR(var, begin, end) for (size_t var = begin; var < end; ++var)
#define END_FOR
#else
#include "tbb/tbb.h"
#define PARALLEL_FOR(var, begin_val, end_val) tbb::parallel_for(tbb::blocked_range<size_t>(begin_val, end_val), [&](const tbb::blocked_range<size_t>& tbb_range) \
{ \
	for (size_t var = tbb_range.begin(); var < tbb_range.end(); ++var)

#define END_FOR \
    });

float line_plane_intersection(const PXR_NS::GfVec3f& line_direction, const PXR_NS::GfVec3f& point_on_line, const PXR_NS::GfVec3f& plane_normal,
                              const PXR_NS::GfVec3f& point_on_plane);

void solve_ray_info(const OPENDCC_NAMESPACE::ViewportMouseEvent& mouse_event, const OPENDCC_NAMESPACE::ViewportViewPtr& viewport_view,
                    PXR_NS::GfVec3f& start, PXR_NS::GfVec3f& direction);

#endif
