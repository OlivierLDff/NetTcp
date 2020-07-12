// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/Server.hpp>
#include <Net/Tcp/ServerWorker.hpp>
#include <Net/Tcp/Socket.hpp>
#include <Net/Tcp/Logger.hpp>

// Qt Headers
#include <QTimer>
#include <QTcpSocket>

// ───── DECLARATION ─────

using namespace net::tcp;

// clang-format off
#ifdef NDEBUG
#define LOG_DEV_DEBUG(str, ...) do {} while (0)
#else
#define LOG_DEV_DEBUG(str, ...) Logger::SERVER->debug("[{}] " str, (void*) (this), ##__VA_ARGS__)
#endif

#ifdef NDEBUG
#define LOG_DEV_INFO(str, ...) do {} while (0)
#else
#define LOG_DEV_INFO(str, ...) Logger::SERVER->info("[{}] " str, (void*) (this), ##__VA_ARGS__)
#endif

#ifdef NDEBUG
#define LOG_DEV_WARN(str, ...) do {} while (0)
#else
#define LOG_DEV_WARN(str, ...) Logger::SERVER->warn("[{}] " str, (void*) (this), ##__VA_ARGS__)
#endif

#ifdef NDEBUG
#define LOG_DEV_ERR(str, ...) do {} while (0)
#else
#define LOG_DEV_ERR(str, ...) Logger::SERVER->error("[{}] " str, (void*) (this), ##__VA_ARGS__)
#endif

#define LOG_DEBUG(str, ...)   Logger::SERVER->debug("[{}] " str, (void*) (this), ##__VA_ARGS__)
#define LOG_INFO(str, ...)    Logger::SERVER->info( "[{}] " str, (void*) (this), ##__VA_ARGS__)
#define LOG_WARN(str, ...)    Logger::SERVER->warn( "[{}] " str, (void*) (this), ##__VA_ARGS__)
#define LOG_ERR(str, ...)     Logger::SERVER->error("[{}] " str, (void*) (this), ##__VA_ARGS__)
// clang-format on

// ───── CLASS ─────

Server::Server(QObject* parent) :
    IServer(parent, {"address", "port", "peerAddress", "peerPort"}),
    _worker(std::make_unique<ServerWorker>())
{
    connect(_worker.get(), &ServerWorker::newIncomingConnection, this,
        [this](qintptr handle)
        {
            if(!handle)
            {
                LOG_ERR("Incoming connection with invalid handle. This "
                        "connection is discarded.");
                return;
            }

            LOG_INFO("Incoming new connection detected");

            if(!canAcceptNewClient())
            {
                auto* socket = new QTcpSocket(this);
                socket->setSocketDescriptor(handle);
                const auto peerAddress = socket->peerAddress().toString();
                const auto peerPort = socket->peerPort();
                LOG_INFO("Refuse connection of client {}:{}",
                    peerAddress.toStdString(), peerPort);
                Q_EMIT clientRefused(peerAddress, peerPort);
                socket->close();
                socket->deleteLater();
            }

            auto socket = newSocket(this);
            socket->setUseWorkerThread(useWorkerThread());

            connect(socket, &Socket::startFailed, this,
                [this, socket]()
                {
                    LOG_ERR("Client Start fail : disconnect");
                    disconnect(socket, nullptr, this, nullptr);
                    socket->deleteLater();
                });
            connect(socket, &Socket::startSuccess, this,
                [this, socket](const QString& address, const quint16 port)
                {
                    LOG_INFO("Client successful started {}:{}",
                        qPrintable(address), signed(port));
                    append(socket);
                });

            const bool success = socket->start(handle);
            if(!success)
            {
                LOG_ERR("Fail to handle new socket from handle {}", handle);
                disconnect(socket, nullptr, this, nullptr);
                socket->deleteLater();
            }
        });

    connect(_worker.get(), &ServerWorker::acceptError, this,
        [this](int error)
        {
            // todo : Use our own enum exposed to qml
            Q_EMIT acceptError(error, _worker->errorString());
        });

    onInserted(
        [this](const InsertedCallbackArgs& socket)
        {
            Q_EMIT newClient(socket->peerAddress(), socket->peerPort());
            LOG_INFO("Client {}:{} connected",
                socket->peerAddress().toStdString(),
                uint16_t(socket->peerPort()));
            connect(socket, &Socket::isConnectedChanged, socket,
                [this, socket](bool connected)
                {
                    if(!connected)
                    {
                        LOG_INFO("Client {}:{} disconnected",
                            socket->peerAddress().toStdString(),
                            socket->peerPort());
                        disconnect(socket, nullptr, this, nullptr);
                        disconnect(this, nullptr, socket, nullptr);
                        remove(socket);
                    }
                });
        });

    onRemoved([this](const RemovedCallbackArgs& socket)
        { Q_EMIT clientLost(socket->peerAddress(), socket->peerPort()); });
}

// Defined here to avoid #include <QTimer> and <ServerWorker>
Server::~Server() = default;

bool Server::setWatchdogPeriod(const quint64& value)
{
    if(IServer::setWatchdogPeriod(value))
    {
        if(_watchdog && _watchdog->isActive())
            startWatchdog();
        return true;
    }
    return false;
}

bool Server::setAddress(const QString& value)
{
    if(IServer::setAddress(value))
    {
        restart();
        return true;
    }
    return false;
}

bool Server::setPort(const quint16& value)
{
    if(IServer::setPort(value))
    {
        restart();
        return true;
    }
    return false;
}

bool Server::setUseWorkerThread(const bool& value)
{
    if(IServer::setUseWorkerThread(value))
    {
        restart();
        return true;
    }
    return false;
}

bool Server::tryStart()
{
    stopWatchdog();
    if(!startWorker())
    {
        LOG_ERR("Fail to start worker, start watchdog to retry in {} ms. "
                "Reason : {}",
            static_cast<std::uint64_t>(watchdogPeriod()),
            _worker->errorString().toStdString());
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
    for(const auto client: *this)
    {
        LOG_INFO("Destroy client {}:{}", client->peerAddress().toStdString(),
            client->peerPort());
        disconnect(client, nullptr, this, nullptr);
        client->stop();
    }

    // Close the server
    if(_worker->isListening())
        _worker->close();

    return true;
}

void Server::startWatchdog()
{
    if(!_watchdog)
    {
        _watchdog = std::make_unique<QTimer>();
        connect(
            _watchdog.get(), &QTimer::timeout, this,
            [this]()
            {
                // Watchdog shouldn't be started is worker is listening with success
                Q_ASSERT(!_worker->isListening());
                // Try to restart the server, or start watchdog
                tryStart();
            },
            Qt::QueuedConnection);
        _watchdog->setTimerType(Qt::VeryCoarseTimer);
        LOG_INFO("Try restart server in {} ms", watchdogPeriod());
    }
    else
    {
        LOG_INFO(
            "Try restart server remaining {} ms", _watchdog->remainingTime());
    }
    _watchdog->start(watchdogPeriod());
}

void Server::stopWatchdog() { _watchdog = nullptr; }

Socket* Server::newSocket(QObject* parent) { return new Socket(parent); }

bool Server::start()
{
    if(isRunning())
    {
        LOG_DEV_ERR("Fail to start tcp server that is already running");
        return false;
    }
    LOG_INFO("Start Server on {}:{}", address().toStdString(), port());

    setRunning(true);
    tryStart();
    return true;
}

bool Server::start(const quint16 port)
{
    setPort(port);
    return start();
}

bool Server::start(const QString& address, const quint16 port)
{
    setAddress(address);
    return start(port);
}

bool Server::stop()
{
    setListening(false);
    setRunning(false);

    stopWatchdog();
    stopWorker();

    return true;
}

bool Server::restart()
{
    if(isRunning())
    {
        LOG_INFO("Restart");
        stop();
        return start();
    }
    return false;
}

Socket* Server::getSocket(const QString& address, const quint16 port) const
{
    for(const auto socket: *this)
    {
        if(address == socket->peerAddress() && port == socket->peerPort())
            return socket;
    }
    return nullptr;
}

QList<Socket*> Server::getSockets(const QString& address) const
{
    QList<Socket*> l;
    for(const auto socket: *this)
    {
        if(address == socket->peerAddress())
            l.append(socket);
    }
    return l;
}

void Server::disconnectFrom(const QString& address, const quint16 port)
{
    remove(getSocket(address, port));
}

void Server::disconnectFrom(const QString& address)
{
    remove(getSockets(address));
}

bool Server::canAcceptNewClient() const { return count() < maxClientCount(); }
