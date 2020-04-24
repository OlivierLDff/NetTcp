// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/AbstractServer.hpp>
#include <Net/Tcp/Logger.hpp>

// ───── DECLARATION ─────

using namespace Net::Tcp;

#ifdef NDEBUG
# define LOG_DEV_DEBUG(str, ...) { do {} while (0); }
#else
# define LOG_DEV_DEBUG(str, ...) Logger::SERVER->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_INFO(str, ...)  { do {} while (0); }
#else
# define LOG_DEV_INFO(str, ...)  Logger::SERVER->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_WARN(str, ...)  { do {} while (0); }
#else
# define LOG_DEV_WARN(str, ...)  Logger::SERVER->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_ERR(str, ...)   { do {} while (0); }
#else
# define LOG_DEV_ERR(str, ...)   Logger::SERVER->error( "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#define LOG_DEBUG(str, ...)      Logger::SERVER->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__);
#define LOG_INFO(str, ...)       Logger::SERVER->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#define LOG_WARN(str, ...)       Logger::SERVER->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#define LOG_ERR(str, ...)        Logger::SERVER->error( "[{}] " str, (void*)(this), ## __VA_ARGS__);

// ───── CLASS ─────

bool AbstractServer::start()
{
    if (isRunning())
    {
        LOG_DEV_ERR("Fail to start tcp server that is already running");
        return false;
    }
    LOG_INFO("Start Server");

    setRunning(true);
    return true;
}

bool AbstractServer::start(const quint16 port)
{
    setPort(port);
    return start();
}

bool AbstractServer::start(const QString& address, const quint16 port)
{
    setAddress(address);
    return start(port);
}

bool AbstractServer::stop()
{
    setListening(false);
    setRunning(false);

    return true;
}

bool AbstractServer::restart()
{
    if(isRunning())
    {
        stop();
        return start();
    }
    return false;
}

Socket* AbstractServer::getSocket(const QString& address, const quint16 port)
{
    for(const auto socket : *this)
    {
        if (address == socket->peerAddress() &&
            port == socket->peerPort())
            return socket;
    }
    return nullptr;
}

QList<Socket*> AbstractServer::getSockets(const QString& address)
{
    QList<Socket*> l;
    for (const auto socket : *this)
    {
        if (address == socket->peerAddress())
            l.append(socket);
    }
    return l;
}

void AbstractServer::disconnectFrom(const QString& address, const quint16 port)
{
    remove(getSocket(address, port));
}
