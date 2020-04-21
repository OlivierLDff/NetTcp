// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/AbstractSocket.hpp>
#include <Net/Tcp/Logger.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace Net::Tcp;

#ifdef NDEBUG
# define LOG_DEV_DEBUG(str, ...) { do {} while (0); }
#else
# define LOG_DEV_DEBUG(str, ...) Logger::SOCKET->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_INFO(str, ...)  { do {} while (0); }
#else
# define LOG_DEV_INFO(str, ...)  Logger::SOCKET->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_WARN(str, ...)  { do {} while (0); }
#else
# define LOG_DEV_WARN(str, ...)  Logger::SOCKET->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_ERR(str, ...)   { do {} while (0); }
#else
# define LOG_DEV_ERR(str, ...)   Logger::SOCKET->error( "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#define LOG_DEBUG(str, ...)      Logger::SOCKET->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__);
#define LOG_INFO(str, ...)       Logger::SOCKET->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#define LOG_WARN(str, ...)       Logger::SOCKET->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#define LOG_ERR(str, ...)        Logger::SOCKET->error( "[{}] " str, (void*)(this), ## __VA_ARGS__);

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

AbstractSocket::AbstractSocket(QObject* parent): ISocket(parent)
{
}

bool AbstractSocket::setPeerAddress(const QString& value)
{
    if (ISocket::setPeerAddress(value))
    {
        if (!socketDescriptor())
        {
            LOG_INFO("Peer Address is {}. Restart the worker for update.", value.toStdString());
            restart();
        }
        else
        {
            LOG_INFO("Peer Address is {}", value.toStdString());
        }
        return true;
    }
    return false;
}

bool AbstractSocket::setPeerPort(const quint16& value)
{
    if (ISocket::setPeerPort(value))
    {
        if (!socketDescriptor())
        {
            LOG_INFO("Peer Port is {}. Restart the worker for update.", static_cast<std::uint16_t>(value));
            restart();
        }
        else
        {
            LOG_INFO("Peer Port is {}", static_cast<std::uint16_t>(value));
        }
        return true;
    }
    return false;
}

bool AbstractSocket::setUseWorkerThread(const bool& value)
{
    if (ISocket::setUseWorkerThread(value))
    {
        LOG_INFO("Restart worker because {}", value ? "it use it's own thread now" : "it's not using it's own thread anymore");
        restart();
        return true;
    }
    return false;
}

bool AbstractSocket::start()
{
    if (isRunning())
        return false;

    setRunning(true);

    return true;
}

bool AbstractSocket::start(quintptr socketDescriptor)
{
    setSocketDescriptor(socketDescriptor);
    return start();
}

bool AbstractSocket::start(const QString& host, const quint16 port)
{
    setPeerAddress(host);
    setPeerPort(port);
    return start();
}

bool AbstractSocket::stop()
{
    resetConnected();
    resetRunning();
    resetTxBytesPerSeconds();
    resetRxBytesPerSeconds();

    return true;
}

bool AbstractSocket::restart()
{
    if(isRunning())
    {
        stop();
        return start();
    }
    return false;
}

void AbstractSocket::clearRxCounter()
{
    LOG_INFO("Clear Rx Counter");
    resetRxBytesPerSeconds();
    resetRxBytesTotal();
}

void AbstractSocket::clearTxCounter()
{
    LOG_INFO("Clear Tx Counter");
    resetTxBytesPerSeconds();
    resetTxBytesTotal();
}

void AbstractSocket::clearCounters()
{
    clearRxCounter();
    clearTxCounter();
}
