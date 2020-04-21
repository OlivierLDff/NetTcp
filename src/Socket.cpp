// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/Socket.hpp>
#include <Net/Tcp/SocketWorker.hpp>
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

Socket::Socket(QObject* parent): AbstractSocket(parent)
{
}

Socket::~Socket()
{
    killWorker();
}

bool Socket::start()
{
    if (!AbstractSocket::start())
        return false;

    if (socketDescriptor())
    {
        LOG_INFO("Start tcp socket via socketDescriptor {}", signed(socketDescriptor()));
    }
    else
    {
        LOG_INFO("Start tcp socket to {}:{}", qPrintable(peerAddress()), signed(peerPort()));
    }

    Q_ASSERT(_worker.get() == nullptr);
    Q_ASSERT(_workerThread.get() == nullptr);

    _worker = createWorker();

    if(useWorkerThread())
    {
        _workerThread = std::make_unique<QThread>();


        if (objectName().size())
            _workerThread->setObjectName(objectName() + " Worker");
        else
        {
            if (socketDescriptor())
                _workerThread->setObjectName("Socket Worker sd" + QString::number(socketDescriptor()));
            else
                _workerThread->setObjectName("Socket Worker " + peerAddress() + ":" + QString::number(peerPort()));
        }
        _worker->moveToThread(_workerThread.get());
    }

    if (socketDescriptor())
    {
        _worker->_socketDescriptor = socketDescriptor();
    }
    else
    {
        _worker->_watchdogPeriod = watchdogPeriod();
        _worker->_address = peerAddress();
        _worker->_port = peerPort();
    }

    connect(_worker.get(), &SocketWorker::startSuccess, this, &Socket::onStartSuccess);
    connect(_worker.get(), &SocketWorker::startFailed, this, &Socket::onStartFail);
    connect(_worker.get(), &SocketWorker::connectionChanged, this, &Socket::setConnected);
    connect(_worker.get(), &SocketWorker::socketError, this, &Socket::socketError);
    connect(_worker.get(), &SocketWorker::bytesReceived, this, &Socket::onBytesReceived);
    connect(_worker.get(), &SocketWorker::bytesSent, this, &Socket::onBytesSent);
    connect(this, &Socket::startWorker, _worker.get(), &SocketWorker::onStart);
    connect(this, &Socket::stopWorker, _worker.get(), &SocketWorker::onStop);
    connect(this, &Socket::watchdogPeriodChanged, _worker.get(), &SocketWorker::onWatchdogPeriodChanged);

    if(_workerThread)
        _workerThread->start();

    LOG_INFO("Start worker");
    Q_EMIT startWorker();

    return true;
}

bool Socket::start(quintptr socketDescriptor)
{
    return AbstractSocket::start(socketDescriptor);
}

bool Socket::start(const QString& host, const quint16 port)
{
    return AbstractSocket::start(host, port);
}

bool Socket::stop()
{
    AbstractSocket::stop();

    LOG_INFO("Stop worker");
    Q_EMIT stopWorker();
    killWorker();

    return true;
}

void Socket::killWorker()
{
    if (_workerThread)
    {
        LOG_INFO("Kill Worker thread and worker");
        _workerThread->exit();
        _workerThread->wait();
        _workerThread = nullptr;
        _worker = nullptr;
    }
    else if (_worker)
    {
        LOG_INFO("Kill Worker");
        const auto workerPtr = _worker.release();
        disconnect(workerPtr, nullptr, this, nullptr);
        disconnect(this, nullptr, workerPtr, nullptr);
        workerPtr->deleteLater();
    }
}

void Socket::onStartSuccess(const QString& peerAddress, const quint16 peerPort, const QString& localAddress,
    const quint16 localPort)
{
    if (socketDescriptor())
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
