// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/SocketWorker.hpp>
#include <Net/Tcp/Logger.hpp>

// Qt Headers
#include <QtCore/QTimer>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>

// ───── DECLARATION ─────

using namespace net::tcp;

// clang-format off
#ifdef NDEBUG
# define LOG_DEV_DEBUG(str, ...) do {} while (0)
#else
# define LOG_DEV_DEBUG(str, ...) Logger::SOCKET_WORKER->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_INFO(str, ...)  do {} while (0)
#else
# define LOG_DEV_INFO(str, ...)  Logger::SOCKET_WORKER->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_WARN(str, ...)  do {} while (0)
#else
# define LOG_DEV_WARN(str, ...)  Logger::SOCKET_WORKER->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__)
#endif

#ifdef NDEBUG
# define LOG_DEV_ERR(str, ...)   do {} while (0)
#else
# define LOG_DEV_ERR(str, ...)   Logger::SOCKET_WORKER->error( "[{}] " str, (void*)(this), ## __VA_ARGS__)
#endif

#define LOG_DEBUG(str, ...)      Logger::SOCKET_WORKER->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__)
#define LOG_INFO(str, ...)       Logger::SOCKET_WORKER->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__)
#define LOG_WARN(str, ...)       Logger::SOCKET_WORKER->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__)
#define LOG_ERR(str, ...)        Logger::SOCKET_WORKER->error( "[{}] " str, (void*)(this), ## __VA_ARGS__)
// clang-format on

// ───── CLASS ─────

SocketWorker::SocketWorker(QObject* parent) : QObject(parent) {}

SocketWorker::~SocketWorker() = default;

void SocketWorker::onStart()
{
    if(_socketDescriptor)
        LOG_DEV_INFO("Start worker {}", std::uintptr_t(_socketDescriptor));
    else
        LOG_DEV_INFO("Start worker {}:{}", qPrintable(_address), signed(_port));

    Q_ASSERT(!_socket);
    _socket = new QTcpSocket(this);
    _socket->setObjectName("socket");
    if(_socketDescriptor)
    {
        const auto result = _socket->setSocketDescriptor(_socketDescriptor);
        if(!result)
        {
            _socket->deleteLater();
            _socket = nullptr;
            LOG_ERR("Fail to set socket descriptor. Can't start the socket.");
            Q_EMIT startFailed();
            return;
        }
    }
    else
    {
        _socket->connectToHost(_address, _port);
    }

    _isRunning = true;

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(_socket QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this,
        &SocketWorker::onSocketError);
#else
    connect(_socket, &QAbstractSocket::errorOccurred, this, &SocketWorker::onSocketError);
#endif
    connect(_socket, &QTcpSocket::stateChanged, this, &SocketWorker::onSocketStateChanged);
    connect(_socket, &QTcpSocket::connected, this, &SocketWorker::onConnected);
    connect(_socket, &QTcpSocket::disconnected, this, &SocketWorker::onDisconnected);
    connect(_socket, &QTcpSocket::readyRead, this, &SocketWorker::onDataAvailable);

    if(_socket->state() == QAbstractSocket::ConnectedState)
        onConnected();

    applyNoDelayOption();
}

void SocketWorker::onStop()
{
    LOG_INFO("Stop Worker");
    _isRunning = false;
    stopWatchdog();
    stopBytesCounter();
    closeSocket();
}

void SocketWorker::closeSocket()
{
    if(!_socket)
        return;

    // Avoid nested call (with onDisconnected)
    if(_pendingClosing)
        return;
    _pendingClosing = true;

    if(_socketDescriptor)
        LOG_DEV_INFO("Close socket worker {}", _socketDescriptor);
    else
        LOG_DEV_INFO("Close socket worker {}:{}", qPrintable(_address), signed(_port));

    Q_ASSERT(_socket);
    disconnect(this, nullptr, _socket, nullptr);
    disconnect(_socket, nullptr, this, nullptr);
    onDisconnected();
    _socket->close();

    LOG_DEV_INFO("Delete later closed socket {}", static_cast<void*>(_socket));
    // very important to deleteLater here, because this function is often call from DirectConnect slot connected to socket
    _socket->deleteLater();
    _socket = nullptr;

    _pendingClosing = false;
}

std::size_t SocketWorker::write(const std::uint8_t* buffer, const std::size_t length)
{
    if(!_socket)
    {
        LOG_DEV_WARN("Fail to write to null socket. Check it with isConnected()");
        return false;
    }
    if(!_socket->isValid())
    {
        LOG_DEV_ERR("Fail to write to invalid socket");
        return false;
    }

    std::size_t bytesWritten = 0;
    std::size_t bytesLeft = length;

    while(bytesWritten != length)
    {
        const auto currentBytesWritten =
            _socket->write(reinterpret_cast<const char*>(buffer) + bytesWritten, bytesLeft - bytesWritten);

        if(currentBytesWritten < 0)
        {
            LOG_ERR("Fail to write to socket");
            closeAndRestart();
            return 0;
        }

        bytesWritten += std::size_t(currentBytesWritten);
        bytesLeft -= std::size_t(currentBytesWritten);
    }

    _txBytesCounter += length;
    return bytesWritten;
}

std::size_t SocketWorker::write(const char* buffer, const std::size_t length)
{
    return write(reinterpret_cast<const std::uint8_t*>(buffer), length);
}

void SocketWorker::onSocketError(const QAbstractSocket::SocketError e)
{
    // todo : use our own error type
    if(_socket)
    {
        LOG_ERR("Socket Error ({}) : {}", int(e), qPrintable(_socket->errorString()));
    }
    else
    {
        LOG_ERR("Socket Error ({})", int(e));
    }
    Q_EMIT socketError(int(e), _socket ? _socket->errorString() : QStringLiteral("Invalid Socket"));

    if(e == QAbstractSocket::SocketError::ConnectionRefusedError)
    {
        closeAndRestart();
    }
}

void SocketWorker::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    LOG_INFO("New Socket state : {}", (int)socketState);
    if(socketState == QAbstractSocket::UnconnectedState)
    {
        onDisconnected();
    }
}

void SocketWorker::onConnected()
{
    // Avoid nested call with connections
    if(_isConnected)
        return;
    _isConnected = true;

    Q_ASSERT(_socket);
    stopWatchdog();
    LOG_INFO("Socket is connected to {}:{}", qPrintable(_socket->peerAddress().toString()), _socket->peerPort());
    Q_EMIT startSuccess(_socket ? _socket->peerAddress().toString() : "", _socket ? _socket->peerPort() : 0,
        _socket ? _socket->localAddress().toString() : "", _socket ? _socket->localPort() : 0);
    Q_EMIT connectionChanged(true);

    startBytesCounter();
}

void SocketWorker::onDisconnected()
{
    // Avoid nested call with connections
    if(!_isConnected)
        return;
    _isConnected = false;

    if(_socket)
    {
        LOG_INFO("Socket disconnected from {}:{}", qPrintable(_socket->peerAddress().toString()), _socket->peerPort());
    }
    Q_EMIT connectionChanged(false);
    if(!_socketDescriptor)
        closeAndRestart();

    stopBytesCounter();
}

bool SocketWorker::isConnected() const { return _socket && _socket->state() == QAbstractSocket::ConnectedState; }

void SocketWorker::onDataAvailable() { LOG_DEV_WARN("You need to override onDataAvailable"); }

std::size_t SocketWorker::bytesAvailable() const { return _socket ? _socket->bytesAvailable() : 0; }

std::size_t SocketWorker::read(std::uint8_t* data, std::size_t maxLen)
{
    return read(reinterpret_cast<char*>(data), maxLen);
}

std::size_t SocketWorker::read(char* data, std::size_t maxLen)
{
    if(!_socket)
        LOG_DEV_WARN("Don't call read when socket is null. Check with isConnected().");

    const auto byteRead = _socket ? _socket->read(data, maxLen) : 0;
    _rxBytesCounter += byteRead;
    return byteRead;
}

void SocketWorker::onWatchdogPeriodChanged(quint64 period)
{
    _watchdogPeriod = period;
    // Restart the watchdog with new period
    if(_watchdog && _watchdog->isActive())
        closeAndRestart();
}

void SocketWorker::onWatchdogTimeout()
{
    // Try to restart the server, or start watchdog
    onStart();
}

void SocketWorker::closeAndRestart()
{
    if(!_isRunning)
        return;

    LOG_INFO("Close and try to restart socket in {} ms", _watchdogPeriod);

    // Don't restart again
    if(_watchdog && (_watchdog->remainingTime() > 0) && _watchdog->remainingTime() <= int(_watchdogPeriod))
    {
        LOG_INFO("Socket Restart timer is already running. Remaining time "
                 "before restart : {} ms",
            _watchdog->remainingTime());
        return;
    }

    closeSocket();

    // Can't restart a socket descriptor based socket
    // Just need to wait for the parent to kill it
    if(_socketDescriptor)
    {
        LOG_DEV_DEBUG("Don't start watchdog for worker, because it have been "
                      "created with a socketDescriptor. Please kill this socket worker from owner");
        return;
    }

    if(!_watchdog)
    {
        _watchdog = new QTimer(this);
        _watchdog->setObjectName("watchdog");
        LOG_DEV_DEBUG("Allocate watchdog {}", static_cast<void*>(_watchdog));

        connect(_watchdog, &QTimer::timeout, this, &SocketWorker::onWatchdogTimeout);
        _watchdog->setTimerType(Qt::VeryCoarseTimer);
        _watchdog->setSingleShot(true);
    }
    _watchdog->start(_watchdogPeriod);
    LOG_INFO("Start Watchdog to attempt reconnection in {} ms", signed(_watchdogPeriod));
}

void SocketWorker::setNoDelay(bool value)
{
    if(value != _noDelay)
    {
        _noDelay = value;
        applyNoDelayOption();
    }
}

void SocketWorker::applyNoDelayOption() const
{
    if(_socket)
    {
        _socket->setSocketOption(QAbstractSocket::LowDelayOption, _noDelay);
    }
}

void SocketWorker::stopWatchdog()
{
    if(_watchdog)
    {
        disconnect(_watchdog, &QTimer::timeout, this, nullptr);
        _watchdog->deleteLater();
        _watchdog = nullptr;
    }
}

void SocketWorker::startBytesCounter()
{
    if(_bytesCounterTimer)
    {
        LOG_DEV_ERR("_bytesCounterTimer is valid at startBytesCounter called");
        stopBytesCounter();
    }
    // Should only be called if _bytesCounterTimer is nullptr
    Q_ASSERT(!_bytesCounterTimer);

    _bytesCounterTimer = new QTimer(this);
    _bytesCounterTimer->setObjectName("bytesCounter");
    _bytesCounterTimer->setSingleShot(false);
    _bytesCounterTimer->setInterval(1000);
    connect(_bytesCounterTimer, &QTimer::timeout, this, &SocketWorker::updateDataCounter);
    _bytesCounterTimer->start();
}

void SocketWorker::stopBytesCounter()
{
    Q_EMIT bytesReceived(_rxBytesCounter);
    Q_EMIT bytesSent(_txBytesCounter);

    if(_rxBytesCounter)
    {
        _rxBytesCounter = 0;
        Q_EMIT bytesReceived(0);
    }
    if(_txBytesCounter)
    {
        _txBytesCounter = 0;
        Q_EMIT bytesSent(0);
    }

    if(_bytesCounterTimer)
    {
        disconnect(_bytesCounterTimer, &QTimer::timeout, this, nullptr);
        _bytesCounterTimer->deleteLater();
        _bytesCounterTimer = nullptr;
    }
}

void SocketWorker::updateDataCounter()
{
    Q_EMIT bytesReceived(_rxBytesCounter);
    Q_EMIT bytesSent(_txBytesCounter);
    _rxBytesCounter = 0;
    _txBytesCounter = 0;
}
