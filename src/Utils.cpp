// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Library Headers
#include <Net/Tcp/Utils.hpp>
#include <Net/Tcp/Server.hpp>
#include <Net/Tcp/Socket.hpp>
#include <Net/Tcp/Version.hpp>
#include <Net/Tcp/Logger.hpp>

// Qt Headers
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QAbstractSocket>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

// clang-format off
#ifdef NDEBUG
# define LOG_DEV_DEBUG(str, ...) do {} while (0)
#else
# define LOG_DEV_DEBUG(str, ...) net::tcp::Logger::SOCKET->debug(str, ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_INFO(str, ...)  do {} while (0)
#else
# define LOG_DEV_INFO(str, ...)  net::tcp::Logger::SOCKET->info( str, ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_WARN(str, ...)  do {} while (0)
#else
# define LOG_DEV_WARN(str, ...)  net::tcp::Logger::SOCKET->warn( str, ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_ERR(str, ...)   do {} while (0)
#else
# define LOG_DEV_ERR(str, ...)   net::tcp::Logger::SOCKET->error(str, ## __VA_ARGS__)
#endif

#define LOG_DEBUG(str, ...)      net::tcp::Logger::SOCKET->debug(str, ## __VA_ARGS__)
#define LOG_INFO(str, ...)       net::tcp::Logger::SOCKET->info( str, ## __VA_ARGS__)
#define LOG_WARN(str, ...)       net::tcp::Logger::SOCKET->warn( str, ## __VA_ARGS__)
#define LOG_ERR(str, ...)        net::tcp::Logger::SOCKET->error(str, ## __VA_ARGS__)
// clang-format on

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

static const char* _defaultUri = "NetTcp";
static const char** _uri = &_defaultUri;
static quint8 _major = 1;
static quint8 _minor = 0;

static void NetTcp_registerTypes()
{
    LOG_DEV_INFO("Register NetTcp v{}",
        qPrintable(net::tcp::Version::version().readable()));

    LOG_DEV_INFO(
        "Register Singleton {}.Version {}.{} to QML", *_uri, _major, _minor);
    net::tcp::Version::registerSingleton(*_uri, _major, _minor);

    LOG_DEV_INFO("Register {}.Server {}.{} to QML", *_uri, _major, _minor);
    net::tcp::Server::registerToQml(*_uri, _major, _minor);

    LOG_DEV_INFO("Register {}.Socket {}.{} to QML", *_uri, _major, _minor);
    net::tcp::Socket::registerToQml(*_uri, _major, _minor);

    // Mandatory to listen to socket error since this type isn't registered by Qt
    qRegisterMetaType<QAbstractSocket::SocketState>();
}

static void NetTcp_registerTypes(
    const char* uri, const quint8 major, const quint8 minor)
{
    if(uri)
        _uri = &uri;
    _major = major;
    _minor = minor;
    NetTcp_registerTypes();
}

void NetTcp_loadResources()
{
    LOG_DEV_INFO("Load NetTcp.qrc v{}",
        qPrintable(net::tcp::Version::version().readable()));
    Q_INIT_RESOURCE(NetTcp);
}

#ifndef NETTCP_STATIC
Q_COREAPP_STARTUP_FUNCTION(NetTcp_registerTypes)
Q_COREAPP_STARTUP_FUNCTION(NetTcp_loadResources)
#endif

using namespace net::tcp;

void Utils::registerTypes(
    const char* uri, const quint8 major, const quint8 minor)
{
    ::NetTcp_registerTypes(uri, major, minor);
}

void Utils::loadResources() { ::NetTcp_loadResources(); }
