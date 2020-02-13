// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/SocketWorker.hpp>

// Dependencies Header

// Qt Header
#include <QLoggingCategory>
#include <QHostAddress>
#include <QTimer>

// STL Header

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

Q_LOGGING_CATEGORY(SOCKET_WORKER_LOG_CAT, "net.tcp.socket.worker")

using namespace Net::Tcp;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

SocketWorker::SocketWorker(QObject* parent): QObject(parent)
{
}

SocketWorker::~SocketWorker() = default;

void SocketWorker::onStart()
{
    if(_socketDescriptor)
        qCDebug(SOCKET_WORKER_LOG_CAT, "onStart tcp socket worker %d", signed(_socketDescriptor));
    else
        qCDebug(SOCKET_WORKER_LOG_CAT, "onStart tcp socket worker %s:%d", qPrintable(_address), signed(_port));
    Q_ASSERT(!_socket);
    _socket = std::make_unique<QTcpSocket>(this);
    if (_socketDescriptor)
    {
        const auto result = _socket->setSocketDescriptor(_socketDescriptor);
        if (!result)
        {
            _socket = nullptr;
            qCDebug(SOCKET_WORKER_LOG_CAT, "Error : Failed to set socket descriptor");
            Q_EMIT startFailed();
            return;
        }
    }
    else
    {
        _socket->connectToHost(_address, _port);
    }
    _isRunning = true;

    connect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this,
            &SocketWorker::onSocketError);
    connect(_socket.get(), &QTcpSocket::stateChanged, this, &SocketWorker::onSocketStateChanged);
    connect(_socket.get(), &QTcpSocket::connected, this, &SocketWorker::onConnected);
    connect(_socket.get(), &QTcpSocket::disconnected, this, &SocketWorker::onDisconnected);
    connect(_socket.get(), &QTcpSocket::readyRead, this, &SocketWorker::onDataAvailable);

    if (_socket->state() == QAbstractSocket::ConnectedState)
        onConnected();

    _socket->setSocketOption(QAbstractSocket::LowDelayOption, true);
}

void SocketWorker::onStop()
{
    qCDebug(SOCKET_WORKER_LOG_CAT, "onStop tcp socket worker");
    _isRunning = false;
    stopWatchdog();
    stopBytesCounter();
    closeSocket();
}

void SocketWorker::closeSocket()
{
    if (_socket)
    {
        disconnect(this, nullptr, _socket.get(), nullptr);
        disconnect(_socket.get(), nullptr, this, nullptr);
        _socket->close();
        // todo: do something here to handle delete later with shared_ptr or unique_ptr instead of raw pointer
        _socket->deleteLater();
        _socket.release();
    }
}

size_t SocketWorker::write(const uint8_t* buffer, const size_t length)
{
    if (!_socket)
        return false;
    if (!_socket->isValid())
        return false;

    const auto bytesWritten = _socket->write(reinterpret_cast<const char*>(buffer), length);
    if (bytesWritten != length)
    {
        qCDebug(SOCKET_WORKER_LOG_CAT, "Error : only wrote %d/%d", int(bytesWritten), int(length));
        closeAndRestart();
    }
    _txBytesCounter += bytesWritten;
    return bytesWritten;
}

size_t SocketWorker::write(const char* buffer, const size_t length)
{
    return write(reinterpret_cast<const uint8_t*>(buffer), length);
}

void SocketWorker::onSocketError(const QAbstractSocket::SocketError e)
{
    if (_socket)
        qCDebug(SOCKET_WORKER_LOG_CAT, "Error : Socket Error (%d) : %s", int(e), qPrintable(_socket->errorString()));
    else
        qCDebug(SOCKET_WORKER_LOG_CAT, "Error : Socket Error (%d)", int(e));
    Q_EMIT socketError(int(e), _socket ? _socket->errorString() : QStringLiteral("Invalid Socket"));
}

void SocketWorker::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    qCDebug(SOCKET_WORKER_LOG_CAT, "New Socket state : %d", socketState);
    if (socketState == QAbstractSocket::UnconnectedState)
    {
        onDisconnected();
    }
}

void SocketWorker::onConnected()
{
    stopWatchdog();
    qCDebug(SOCKET_WORKER_LOG_CAT, "Socket is connected to %s:%d", qPrintable(_socket->peerAddress().toString()), _socket->peerPort());
    Q_EMIT connectionChanged(true);
    Q_EMIT startSuccess(_socket ? _socket->peerAddress().toString() : "", _socket ? _socket->peerPort() : 0, _socket ? _socket->localAddress().toString() : "", _socket ? _socket->localPort() : 0);

    startBytesCounter();
}

void SocketWorker::onDisconnected()
{
    qCDebug(SOCKET_WORKER_LOG_CAT, "Socket disconnected from %s:%d", qPrintable(_socket->peerAddress().toString()), _socket->peerPort());
    Q_EMIT connectionChanged(false);
    if (!_socketDescriptor)
        closeAndRestart();

    stopBytesCounter();
}

void SocketWorker::onDataAvailable()
{
    qCDebug(SOCKET_WORKER_LOG_CAT, "Warning : you need to override onDataAvailable");
}

size_t SocketWorker::bytesAvailable() const
{
    return _socket ? _socket->bytesAvailable() : 0;
}

size_t SocketWorker::read(uint8_t* data, size_t maxLen)
{
    return read(reinterpret_cast<char*>(data), maxLen);
}

size_t SocketWorker::read(char* data, size_t maxLen)
{
    const auto byteRead = _socket ? _socket->read(data, maxLen) : 0;
    _rxBytesCounter += byteRead;
    return byteRead;
}

void SocketWorker::onWatchdogPeriodChanged(quint64 period)
{
    _watchdogPeriod = period;
    // Restart the watchdog with new period
    if (_watchdog && _watchdog->isActive())
        closeAndRestart();
}

void SocketWorker::onWatchdogTimeout()
{
    // Try to restart the server, or start watchdog
    onStart();
}

void SocketWorker::closeAndRestart()
{
    qCDebug(SOCKET_WORKER_LOG_CAT, "Close and restart socket later Call");
    if (!_isRunning)
        return;

    // Don't restart again
    if (_watchdog && (_watchdog->remainingTime() > 0) && _watchdog->remainingTime() < int(_watchdogPeriod))
        return;

    closeSocket();
    if (!_watchdog)
    {
        _watchdog = std::make_unique<QTimer>();

        connect(_watchdog.get(), &QTimer::timeout, this, &SocketWorker::onWatchdogTimeout);
        _watchdog->setTimerType(Qt::VeryCoarseTimer);
        _watchdog->setSingleShot(true);
    }
    _watchdog->start(_watchdogPeriod);
    qCDebug(SOCKET_WORKER_LOG_CAT, "Start worker watchdog to attempt reconnection in %d ms", signed(_watchdogPeriod));
}

void SocketWorker::stopWatchdog()
{
    _watchdog = nullptr;
}

void SocketWorker::startBytesCounter()
{
    Q_ASSERT(_bytesCounterTimer.get() == nullptr);

    _bytesCounterTimer = std::make_unique<QTimer>();
    _bytesCounterTimer->setSingleShot(false);
    _bytesCounterTimer->setInterval(1000);
    connect(_bytesCounterTimer.get(), &QTimer::timeout, this, &SocketWorker::updateDataCounter);
    _bytesCounterTimer->start();
}

void SocketWorker::stopBytesCounter()
{
    Q_EMIT bytesReceived(_rxBytesCounter);
    Q_EMIT bytesSent(_txBytesCounter);

    if (_rxBytesCounter)
    {
        _rxBytesCounter = 0;
        Q_EMIT bytesReceived(0);
    }
    if (_txBytesCounter)
    {
        _txBytesCounter = 0;
        Q_EMIT bytesSent(0);
    }

    _bytesCounterTimer = nullptr;
}

void SocketWorker::updateDataCounter()
{
    Q_EMIT bytesReceived(_rxBytesCounter);
    Q_EMIT bytesSent(_txBytesCounter);
    _rxBytesCounter = 0;
    _txBytesCounter = 0;
}
