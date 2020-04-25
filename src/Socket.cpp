// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/Socket.hpp>
#include <Net/Tcp/SocketWorker.hpp>
#include <Net/Tcp/Logger.hpp>

// Qt Headers
#include <QThread>

// ───── DECLARATION ─────

using namespace Net::Tcp;

// clang-format off

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

// clang-format on

// ───── CLASS ─────

Socket::Socket(QObject* parent) : ISocket(parent) {}

Socket::~Socket() { killWorker(); }

bool Socket::setPeerAddress(const QString& value)
{
    if(ISocket::setPeerAddress(value))
    {
        if(!socketDescriptor())
        {
            LOG_INFO("Peer Address is {}. Restart the worker for update.",
                value.toStdString());
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

bool Socket::setPeerPort(const quint16& value)
{
    if(ISocket::setPeerPort(value))
    {
        if(!socketDescriptor())
        {
            LOG_INFO("Peer Port is {}. Restart the worker for update.",
                static_cast<std::uint16_t>(value));
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

bool Socket::setUseWorkerThread(const bool& value)
{
    if(ISocket::setUseWorkerThread(value))
    {
        LOG_INFO("Restart worker because {}",
            value ? "it use it's own thread now" :
                    "it's not using it's own thread anymore");
        restart();
        return true;
    }
    return false;
}

bool Socket::start()
{
    if(isRunning())
    {
        LOG_DEV_WARN("Can't start socket that is already running");
        return false;
    }

    setRunning(true);

    if(socketDescriptor())
    {
        LOG_INFO("Start tcp socket via socketDescriptor {}",
            signed(socketDescriptor()));
    }
    else
    {
        LOG_INFO("Start tcp socket to {}:{}", qPrintable(peerAddress()),
            signed(peerPort()));
    }

    Q_ASSERT(_worker.get() == nullptr);
    Q_ASSERT(_workerThread.get() == nullptr);

    _worker = createWorker();

    if(useWorkerThread())
    {
        _workerThread = std::make_unique<QThread>();

        if(objectName().size())
            _workerThread->setObjectName(objectName() + " Worker");
        else
        {
            if(socketDescriptor())
                _workerThread->setObjectName(
                    "Socket Worker sd" + QString::number(socketDescriptor()));
            else
                _workerThread->setObjectName("Socket Worker " + peerAddress() +
                                             ":" + QString::number(peerPort()));
        }
        _worker->moveToThread(_workerThread.get());
    }

    if(socketDescriptor())
    {
        _worker->_socketDescriptor = socketDescriptor();
    }
    else
    {
        _worker->_watchdogPeriod = watchdogPeriod();
        _worker->_address = peerAddress();
        _worker->_port = peerPort();
    }

    connect(_worker.get(), &SocketWorker::startSuccess, this,
        &Socket::onStartSuccess);
    connect(
        _worker.get(), &SocketWorker::startFailed, this, &Socket::onStartFail);
    connect(_worker.get(), &SocketWorker::connectionChanged, this,
        &Socket::setConnected);
    connect(
        _worker.get(), &SocketWorker::socketError, this, &Socket::socketError);
    connect(_worker.get(), &SocketWorker::bytesReceived, this,
        &Socket::onBytesReceived);
    connect(
        _worker.get(), &SocketWorker::bytesSent, this, &Socket::onBytesSent);
    connect(this, &Socket::startWorker, _worker.get(), &SocketWorker::onStart);
    connect(this, &Socket::stopWorker, _worker.get(), &SocketWorker::onStop);
    connect(this, &Socket::watchdogPeriodChanged, _worker.get(),
        &SocketWorker::onWatchdogPeriodChanged);

    if(_workerThread)
        _workerThread->start();

    LOG_INFO("Start worker");
    Q_EMIT startWorker();

    return true;
}

bool Socket::start(quintptr socketDescriptor)
{
    setSocketDescriptor(socketDescriptor);
    return start();
}

bool Socket::start(const QString& host, const quint16 port)
{
    setPeerAddress(host);
    setPeerPort(port);
    return start();
}

bool Socket::stop()
{
    LOG_INFO("Stop worker");

    resetConnected();
    resetRunning();
    resetTxBytesPerSeconds();
    resetRxBytesPerSeconds();

    Q_EMIT stopWorker();
    killWorker();

    return true;
}

bool Socket::restart()
{
    if(isRunning())
    {
        LOG_INFO("Restart");
        stop();
        return start();
    }
    return false;
}

void Socket::clearRxCounter()
{
    LOG_INFO("Clear Rx Counter");
    resetRxBytesPerSeconds();
    resetRxBytesTotal();
}

void Socket::clearTxCounter()
{
    LOG_INFO("Clear Tx Counter");
    resetTxBytesPerSeconds();
    resetTxBytesTotal();
}

void Socket::clearCounters()
{
    clearRxCounter();
    clearTxCounter();
}

void Socket::killWorker()
{
    if(_workerThread)
    {
        LOG_INFO("Kill Worker thread and worker");
        _workerThread->exit();
        _workerThread->wait();
        _workerThread = nullptr;
        _worker = nullptr;
    }
    else if(_worker)
    {
        LOG_INFO("Kill Worker");
        const auto workerPtr = _worker.release();
        disconnect(workerPtr, nullptr, this, nullptr);
        disconnect(this, nullptr, workerPtr, nullptr);
        workerPtr->deleteLater();
    }
}

void Socket::onStartSuccess(const QString& peerAddress, const quint16 peerPort,
    const QString& localAddress, const quint16 localPort)
{
    if(socketDescriptor())
    {
        setPeerAddress(peerAddress);
        setPeerPort(peerPort);
    }
    setLocalAddress(localAddress);
    setLocalPort(localPort);
    Q_EMIT startSuccess(peerAddress, peerPort);
}

void Socket::onStartFail()
{
    resetConnected();
    Q_EMIT startFailed();
}

void Socket::onBytesReceived(const uint64_t count)
{
    setRxBytesPerSeconds(count);
    setRxBytesTotal(rxBytesTotal() + count);
}

void Socket::onBytesSent(const uint64_t count)
{
    setTxBytesPerSeconds(count);
    setTxBytesTotal(txBytesTotal() + count);
}

std::unique_ptr<SocketWorker> Socket::createWorker()
{
    return std::make_unique<SocketWorker>();
}
