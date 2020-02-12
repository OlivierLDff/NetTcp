// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/AbstractServer.hpp>

// Dependencies Header

// Qt Header
#include <QLoggingCategory>

// STL Header

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

Q_LOGGING_CATEGORY(ABSTRACT_SERVER_LOG_CAT, "net.tcp.abstractServer")

using namespace Net::Tcp;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

bool AbstractServer::start()
{
    if (isRunning())
    {
        qCDebug(ABSTRACT_SERVER_LOG_CAT, "Error : Fail to start tcp server that is already running");
        return false;
    }
    qCDebug(ABSTRACT_SERVER_LOG_CAT, "Start tcp server");

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

AbstractSocket* AbstractServer::getSocket(const QString& address, const quint16 port)
{
    for(const auto it : *this)
    {
        if (address == it->address() &&
            port == it->port())
            return it;
    }
    return nullptr;
}

QList<AbstractSocket*> AbstractServer::getSocket(const QString& address)
{
    QList<AbstractSocket*> l;
    for (const auto it : *this)
    {
        if (address == it->address())
            l.append(it);
    }
    return l;
}

void AbstractServer::disconnectFrom(const QString& address, const quint16 port)
{
    remove(getSocket(address, port));
}
