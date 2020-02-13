// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/Socket.hpp>
#include <Net/Tcp/SocketWorker.hpp>

// Dependencies Header

// Qt Header
#include <QLoggingCategory>
#include <QThread>

// STL Header

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

Q_LOGGING_CATEGORY(SOCKET_LOG_CAT, "net.tcp.socket")

using namespace Net::Tcp;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

Socket::Socket(QObject* parent): AbstractSocket(parent), _impl(std::make_unique<SocketImpl>(this))
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
        qCDebug(SOCKET_LOG_CAT, "Start tcp socket via socketDescriptor %d", signed(socketDescriptor()));
    else
        qCDebug(SOCKET_LOG_CAT, "Start tcp socket to %s : %d", qPrintable(peerAddress()), signed(peerPort()));

    Q_ASSERT(_worker.get() == nullptr);
    Q_ASSERT(_workerThread.get() == nullptr);

    _worker = createWorker();

    if(useWorkerThread())
    {
        _workerThread = std::make_unique<QThread>();


        if (objectName().size())
            _workerThread->setObjectName(objectName() + " Worker");
        else
            _workerThread->setObjectName("Socket Worker");
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

    connect(_worker.get(), &SocketWorker::startSuccess, [this](const QString& peerAddress, const quint16 peerPort, const QString& localAddress, const quint16 localPort)
    {
        if(socketDescriptor())
        {
            setPeerAddress(peerAddress);
            setPeerPort(peerPort);            
        }
        setLocalAddress(localAddress);
        setLocalPort(localPort);
         Q_EMIT startSuccess(peerAddress, peerPort);
    });
    connect(_worker.get(), &SocketWorker::startFailed, [this]()
    {
        setConnected(false);
    });
    connect(_worker.get(), &SocketWorker::startFailed, this, &Socket::startFailed);
    connect(_worker.get(), &SocketWorker::connectionChanged, this, &Socket::setConnected);
    connect(_worker.get(), &SocketWorker::socketError, this, &Socket::socketError);
    connect(_worker.get(), &SocketWorker::bytesReceived, [this](const quint64 count)
        {
            setRxBytesPerSeconds(count);
            setRxBytesTotal(rxBytesTotal() + count);
        });

    connect(_worker.get(), &SocketWorker::bytesSent, [this](const quint64 count)
        {
            setTxBytesPerSeconds(count);
            setTxBytesTotal(txBytesTotal() + count);
        });

    connect(_impl.get(), &SocketImpl::startWorker, _worker.get(), &SocketWorker::onStart);
    connect(_impl.get(), &SocketImpl::stopWorker, _worker.get(), &SocketWorker::onStop);

    connect(this, &Socket::watchdogPeriodChanged, _worker.get(), &SocketWorker::onWatchdogPeriodChanged);

    if(_workerThread)
        _workerThread->start();

    Q_EMIT _impl->startWorker();

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

    Q_EMIT _impl->stopWorker();
    killWorker();

    return true;
}

void Socket::killWorker()
{
    SocketWorker* workerPtr = nullptr;
    if (_workerThread)
    {
        _workerThread->exit();
        _workerThread->wait();
        _workerThread = nullptr;
        _worker = nullptr;
    }
    else if (_worker)
    {
        workerPtr = _worker.release();
        disconnect(workerPtr, nullptr, this, nullptr);
        disconnect(this, nullptr, workerPtr, nullptr);
        disconnect(_impl.get(), nullptr, workerPtr, nullptr);
        workerPtr->deleteLater();
    }
}

std::unique_ptr<SocketWorker> Socket::createWorker()
{
    return std::make_unique<SocketWorker>();
}
