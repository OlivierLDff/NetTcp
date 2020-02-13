// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/Server.hpp>
#include <Net/Tcp/ServerWorker.hpp>
#include <Net/Tcp/Socket.hpp>

// Qt Header
#include <QLoggingCategory>
#include <QTimer>

// STL Header

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

Q_LOGGING_CATEGORY(SERVER_LOG_CAT, "net.tcp.server")

using namespace Net::Tcp;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

Server::Server(QObject* parent): AbstractServer(parent, 
    {
        {"address"},
        {"port"},
        {"peerAddress"},
        {"peerPort"}
    }),
    _worker(std::make_unique<ServerWorker>())
{
    connect(_worker.get(), &ServerWorker::newIncomingConnection, this, &Server::onNewIncomingConnection);
    connect(_worker.get(), &ServerWorker::acceptError, [this](int error)
    {
            Q_EMIT acceptError(error, _worker->errorString());
    });
}

Server::~Server() = default;

bool Server::setWatchdogPeriod(const quint64& value)
{
    if(AbstractServer::setWatchdogPeriod(value))
    {
        if (_watchdog && _watchdog->isActive())
            startWatchdog();
        return true;
    }
    return false;
}

bool Server::setAddress(const QString& value)
{
    if (AbstractServer::setAddress(value))
    {
        restart();
        return true;
    }
    return false;
}

bool Server::setPort(const quint16& value)
{
    if (AbstractServer::setPort(value))
    {
        restart();
        return true;
    }
    return false;
}

bool Server::tryStart()
{
    stopWatchdog();
    if (!startWorker())
    {
        qCDebug(SERVER_LOG_CAT, "Error : Fail to start worker, start watchdog to retry in %llu ms. Reason : %s",
               static_cast<long long unsigned>(watchdogPeriod()), qPrintable(_worker->errorString()));
        startWatchdog();
        return false;
    }
    return true;
}

bool Server::startWorker()
{
    Q_ASSERT(!_worker->isListening());

    // Make sure not client are still in memory
    clear();

    // Start to listen
    const auto result = _worker->listen(QHostAddress(address()), port());
    setListening(result);
    return result;
}

bool Server::stopWorker()
{
    // Destroy every clients
    for (const auto it : *this)
    {
        disconnect(it, nullptr, this, nullptr);
        it->stop();
    }

    // Close the server
    if (_worker->isListening())
        _worker->close();

    return true;
}

void Server::startWatchdog()
{
    if (!_watchdog)
    {
        _watchdog = std::make_unique<QTimer>();
        connect(_watchdog.get(), &QTimer::timeout, this, &Server::onWatchdogTimeout);
        _watchdog->setTimerType(Qt::VeryCoarseTimer);
    }
    _watchdog->start(watchdogPeriod());
}

void Server::stopWatchdog()
{
    _watchdog = nullptr;
}

void Server::onItemInserted(AbstractSocket* item, int row)
{
    Q_EMIT newClient(item->peerAddress(), item->peerPort());
    connect(item, &Socket::isConnectedChanged, [this, item](bool connected)
        {
            if (!connected)
            {
                qCDebug(SERVER_LOG_CAT, "Client disconnected");
                disconnect(item, nullptr, this, nullptr);
                disconnect(this, nullptr, item, nullptr);
                remove(item);
            }
        });
}

void Server::onItemAboutToBeRemoved(AbstractSocket* item, int row)
{
    Q_EMIT clientLost(item->peerAddress(), item->peerPort());
}

void Server::onWatchdogTimeout()
{
    // Watchdog shouldn't be started is worker is listening with success
    Q_ASSERT(!_worker->isListening());
    // Try to restart the server, or start watchdog
    tryStart();
}

AbstractSocket* Server::newTcpSocket(QObject* parent)
{
    return new Socket(this);
}

void Server::onNewIncomingConnection(qintptr handle)
{
    if (!handle)
        return;

    auto socket = newTcpSocket(this);

    qCDebug(SERVER_LOG_CAT, "Connect to new client");
    connect(socket, &Socket::startFailed, [this, socket]()
        {
            qCDebug(SERVER_LOG_CAT, "Client Start fail : disconnect");
            disconnect(socket, nullptr, this, nullptr);
            socket->deleteLater();
        });
    connect(socket, &Socket::startSuccess, [this, socket](const QString& address, const quint16 port)
        {
            qCDebug(SERVER_LOG_CAT, "Client successful started %s:%d", qPrintable(address), signed(port));
            append(socket);
        });

    const bool success = socket->start(handle);
    if(!success)
    {
        qCDebug(SERVER_LOG_CAT, "Error : Fail to handle new socket");
        disconnect(socket, nullptr, this, nullptr);
        socket->deleteLater();        
    }
}

bool Server::start()
{
    if (!AbstractServer::start())
        return false;

    tryStart();

    return true;
}

bool Server::start(const quint16 port)
{
    return AbstractServer::start(port);
}

bool Server::start(const QString& address, const quint16 port)
{
    return AbstractServer::start(address, port);
}

bool Server::stop()
{
    AbstractServer::stop();

    stopWatchdog();
    stopWorker();

    return true;
}
