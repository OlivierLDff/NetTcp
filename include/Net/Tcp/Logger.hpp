#ifndef __NETTCP_LOGGER_HPP__
#define __NETTCP_LOGGER_HPP__

// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/Export.hpp>

// Dependencies Headers
#include <spdlog/spdlog.h>
#include <spdlog/sinks/sink.h>

// Stl Headers
#include <memory>
#include <set>

// ───── DECLARATION ─────

namespace Net {
namespace Tcp {

// ───── CLASS ─────

/**
 * Define static logger that library use.
 * You need to install sink on them
 */
class NETTCP_API_ Logger
{
    // ─────── TYPES ─────────
public:
    using Log       = spdlog::logger;
    using LogPtr    = std::shared_ptr<Log>;
    using LogList   = std::set<LogPtr>;
    using Sink      = spdlog::sinks::sink;
    using SinkPtr   = std::shared_ptr<Sink>;

    // ─────── LOGGERS NAME ─────────
public:
    // Kore
    static const char* const SERVER_NAME;
    static const LogPtr SERVER;
    static const char* const SOCKET_NAME;
    static const LogPtr SOCKET;
    static const char* const SOCKET_WORKER_NAME;
    static const LogPtr SOCKET_WORKER;
    static const char* const UTILS_NAME;
    static const LogPtr UTILS;

    // ─────── LIST OF ALL LOGGERS ─────────
public:
    // Loggers
    static const LogList LOGGERS;

    // ─────── API ─────────
public:
    static void registerSink(const SinkPtr& sink);
    static void unRegisterSink(const SinkPtr& sink);
};

}
}

#endif