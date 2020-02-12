// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/Utils.hpp>
#include <Net/Tcp/Server.hpp>
#include <Net/Tcp/Socket.hpp>
#include <Net/Tcp/Version.hpp>

// Qt Header
#include <QCoreApplication>
#include <QLoggingCategory>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

Q_LOGGING_CATEGORY(NETTCP_UTILS_LOG_CAT, "net.tcp.utils")

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

static const char* _defaultUri = "NetTcp";
static const char** _uri = &_defaultUri;
static quint8 _major = 1;
static quint8 _minor = 0;

static void NetTcp_registerTypes()
{
    qCDebug(NETTCP_UTILS_LOG_CAT, "Register NetTcp v%s", qPrintable(Net::Tcp::Version::version().readable()));

    qCDebug(NETTCP_UTILS_LOG_CAT, "Register Singleton %s.Version %d.%d to QML", *_uri, _major, _minor);
    Net::Tcp::Version::registerSingleton(*_uri, _major, _minor);

    qCDebug(NETTCP_UTILS_LOG_CAT, "Register %s.AbstractServer %d.%d to QML", *_uri, _major, _minor);
    Net::Tcp::AbstractServer::registerToQml(*_uri, _major, _minor);

    qCDebug(NETTCP_UTILS_LOG_CAT, "Register %s.Server %d.%d to QML", *_uri, _major, _minor);
    Net::Tcp::Server::registerToQml(*_uri, _major, _minor);

    qCDebug(NETTCP_UTILS_LOG_CAT, "Register %s.AbstractSocket %d.%d to QML", *_uri, _major, _minor);
    Net::Tcp::AbstractSocket::registerToQml(*_uri, _major, _minor);

    qCDebug(NETTCP_UTILS_LOG_CAT, "Register %s.Socket %d.%d to QML", *_uri, _major, _minor);
    Net::Tcp::Socket::registerToQml(*_uri, _major, _minor);
}

static void NetTcp_registerTypes(const char* uri, const quint8 major, const quint8 minor)
{
    if(uri)
        _uri = &uri;
    _major = major;
    _minor = minor;
    NetTcp_registerTypes();
}

void NetTcp_loadResources()
{
    qCDebug(NETTCP_UTILS_LOG_CAT, "Load NetTcp.qrc v%s", qPrintable(Net::Tcp::Version::version().readable()));
    //Q_INIT_RESOURCE(NetTcp);
    // todo : uncomment when qml debug will be done
}

#ifndef NETTCP_STATIC
Q_COREAPP_STARTUP_FUNCTION(NetTcp_registerTypes)
Q_COREAPP_STARTUP_FUNCTION(NetTcp_loadResources)
#endif

using namespace Net::Tcp;

void Utils::registerTypes(const char* uri, const quint8 major, const quint8 minor)
{
    ::NetTcp_registerTypes(uri, major, minor);
}

void Utils::loadResources()
{
    ::NetTcp_loadResources();
}
