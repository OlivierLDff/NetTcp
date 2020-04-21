
// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/Logger.hpp>

// ───── DECLARATION ─────

using namespace Net::Tcp;

template<typename... Args>
static Logger::LogPtr makeLog(Args&&... args) { return std::make_shared<Logger::Log>(std::forward<Args>(args)...); }

// Camera
const char* const Logger::SERVER_NAME        = "net.tcp.server";
const Logger::LogPtr Logger::SERVER          = makeLog(SERVER_NAME);
const char* const Logger::SOCKET_NAME        = "net.tcp.socket";
const Logger::LogPtr Logger::SOCKET          = makeLog(SOCKET_NAME);
const char* const Logger::SOCKET_WORKER_NAME = "net.tcp.socket.worker";
const Logger::LogPtr Logger::SOCKET_WORKER   = makeLog(SOCKET_WORKER_NAME);
const char* const Logger::UTILS_NAME         = "net.tcp.utils";
const Logger::LogPtr Logger::UTILS           = makeLog(UTILS_NAME);

const Logger::LogList Logger::LOGGERS =
{
    SERVER,
    SOCKET,
    SOCKET_WORKER,
    UTILS
};

// ───── CLASS ─────

void Logger::registerSink(const SinkPtr& sink)
{
    for(const auto& it : LOGGERS)
        it->sinks().emplace_back(sink);
}

void Logger::unRegisterSink(const SinkPtr& sink)
{
    for (const auto& it : LOGGERS)
    {
        auto& sinks = it->sinks();

        auto sinkIt = sinks.begin();
        while(sinkIt != sinks.end())
        {
            const auto& s = *sinkIt;
            if(s == sink)
            {
                sinks.erase(sinkIt);
                break;
            }
        }
    }
}