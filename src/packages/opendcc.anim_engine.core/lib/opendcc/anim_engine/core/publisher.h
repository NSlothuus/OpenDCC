/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/base/vendor/eventpp/eventdispatcher.h"
#include "opendcc/base/vendor/eventpp/utilities/scopedremover.h"

template <typename EventType, typename CallbackType>
class Publisher
{
    using Dispatcher_ = typename eventpp::EventDispatcher<EventType, CallbackType>;
    using Handle_ = typename Dispatcher_::Handle;

public:
    class Handle
    {
    public:
        Handle() {}
        Handle(std::shared_ptr<Dispatcher_>& dispatcher, const EventType& event_type, Handle_& h)
            : m_dispatcher(dispatcher)
            , m_handle(h)
            , m_type(event_type)
        {
        }
        ~Handle() { unsubscribe(); }
        Handle(const Handle&) = delete;
        void unsubscribe()
        {
            if (std::shared_ptr<Dispatcher_> ptr = m_dispatcher.lock())
            {
                ptr->removeListener(m_type, m_handle);
                m_dispatcher.reset();
            }
        }
        Handle(Handle&& other) noexcept
        {
            m_dispatcher = other.m_dispatcher;
            other.m_dispatcher.reset();
        }
        void operator=(const Handle&) = delete;
        const Handle& operator=(Handle&& other) noexcept
        {
            if (this == &other)
                return *this;

            unsubscribe();
            m_handle = other.m_handle;
            m_type = other.m_type;
            m_dispatcher = other.m_dispatcher;
            other.m_dispatcher.reset();
            return *this;
        }

    private:
        Handle_ m_handle;
        EventType m_type;
        std::weak_ptr<Dispatcher_> m_dispatcher;
    };

    Handle subscribe(const EventType& event_type, std::function<CallbackType> callback)
    {
        auto handle = m_dispatcher->appendListener(event_type, callback);
        return Handle(m_dispatcher, event_type, handle);
    }

    Publisher()
        : m_dispatcher(std::make_shared<Dispatcher_>())
    {
    }

protected:
    template <typename... A>
    void dispatch(const EventType& event_type, A... args)
    {
        m_dispatcher->dispatch(event_type, args...);
    }

private:
    std::shared_ptr<Dispatcher_> m_dispatcher;
};

template <typename BaseType>
class KeyType
{
    BaseType m_value;

public:
    explicit KeyType(const BaseType& value)
        : m_value(value)
    {
    }
    bool operator<(const KeyType<BaseType>& other) const { return this->m_value < other.m_value; }
};