// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/base/logging/logger.h"
#include "opendcc/base/logging/default_logging_delegate.h"
#include <cstdarg>

OPENDCC_NAMESPACE_OPEN

OPENDCC_INITIALIZE_LIBRARY_LOG_CHANNEL("Logger");

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::log_impl(const MessageContext& context, const std::string& message)
{
    if (context.channel.empty())
    {
        OPENDCC_WARN("Logging channel of message '{}' is null.", message);
        return;
    }

    auto& logger = instance();
    Lock lock(logger.m_mutex);
    if (context.level < logger.m_log_level)
    {
        return;
    }
    for (auto delegate : logger.m_delegates)
    {
        delegate->log(context, message);
    }
}

LogLevel Logger::get_log_level()
{
    auto& logger = instance();
    Lock lock(logger.m_mutex);
    return logger.m_log_level;
}

Logger::Logger()
    : m_default_delegate { new DefaultLoggingDelegate }
{
    m_delegates.push_back(m_default_delegate);
}

void Logger::add_logging_delegate(LoggingDelegate* delegate)
{
    auto& logger = instance();
    Lock lock(logger.m_mutex);
    for (const auto entry : logger.m_delegates)
    {
        if (entry == delegate)
        {
            OPENDCC_WARN("LoggingDelegate ({}) already added.", static_cast<void*>(delegate));
            return;
        }
    }
    logger.m_delegates.push_back(delegate);
}

void Logger::set_log_level(LogLevel level)
{
    auto& logger = instance();
    Lock lock(logger.m_mutex);
    logger.m_log_level = level;
}

void Logger::remove_logging_delegate(LoggingDelegate* delegate)
{
    auto& logger = instance();
    Lock lock(logger.m_mutex);
    auto& delegates = logger.m_delegates;
    auto it = std::find(delegates.begin(), delegates.end(), delegate);
    if (it != delegates.end())
    {
        delegates.erase(it);
    }
}

DefaultLoggingDelegate* Logger::get_default_logging_delegate()
{
    return instance().m_default_delegate;
}

Logger::~Logger()
{
    m_delegates.clear();
    delete m_default_delegate;
}

bool fail_assert(const MessageContext& ctx, const char* condition)
{
    Logger::log(ctx, "Failed assert '{}' in function '{}'.", condition, ctx.function);
#if __cplusplus >= 201703L || _MSVC_LANG >= 201703L
    if (is_debugged())
    {
        trap_debugger();
    }
#endif

    abort();
}

OPENDCC_NAMESPACE_CLOSE
