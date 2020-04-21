// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/SocketWorker.hpp>
#include <Net/Tcp/Logger.hpp>

// Qt Header
#include <QHostAddress>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace Net::Tcp;

#ifdef NDEBUG
# define LOG_DEV_DEBUG(str, ...) { do {} while (0); }
#else
# define LOG_DEV_DEBUG(str, ...) Logger::SOCKET_WORKER->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_INFO(str, ...)  { do {} while (0); }
#else
# define LOG_DEV_INFO(str, ...)  Logger::SOCKET_WORKER->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_WARN(str, ...)  { do {} while (0); }
#else
# define LOG_DEV_WARN(str, ...)  Logger::SOCKET_WORKER->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#ifdef NDEBUG
# define LOG_DEV_ERR(str, ...)   { do {} while (0); }
#else
# define LOG_DEV_ERR(str, ...)   Logger::SOCKET_WORKER->error( "[{}] " str, (void*)(this), ## __VA_ARGS__);
#endif

#define LOG_DEBUG(str, ...)      Logger::SOCKET_WORKER->debug( "[{}] " str, (void*)(this), ## __VA_ARGS__);
#define LOG_INFO(str, ...)       Logger::SOCKET_WORKER->info(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#define LOG_WARN(str, ...)       Logger::SOCKET_WORKER->warn(  "[{}] " str, (void*)(this), ## __VA_ARGS__);
#define LOG_ERR(str, ...)        Logger::SOCKET_WORKER->error( "[{}] " str, (void*)(this), ## __VA_ARGS__);

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
    {
        LOG_DEV_INFO("Start worker {}", std::uintptr_t(_socketDescriptor));
    }
    else
    {
        LOG_DEV_INFO("Start worker {}:{}", qPrintable(_address), signed(_port));
    }
    Q_ASSERT(!_socket);
    _socket = std::make_unique<QTcpSocket>(this);
    if (_socketDescriptor)
    {
        const auto result = _socket->setSocketDescriptor(_socketDescriptor);
        if (!result)
        {
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

    connect(_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this,
            &SocketWorker::onSocketError);
    connect(_socket.get(), &QTcpSocket::stateChanged, this, &SocketWorker::onSocketStateChanged);
    connect(_socket.get(), &QTcpSocket::connected, this, &SocketWorker::onConnected);
    connect(_socket.get(), &QTcpSocket::disconnected, this, &SocketWorker::onDisconnected);
    connect(_socket.get(), &QTcpSocket::readyRead, this, &SocketWorker::onDataAvailable);

    if (_socket->state() == QAbstractSocket::ConnectedState)
        onConnected();

    // todo : maybe this should be exposed as a property on Net::Tcp::Socket object
    _socket->setSocketOption(QAbstractSocket::LowDelayOption, true);
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
    if (_socket)
    {
        if(_socketDescriptor)
        {
            LOG_DEV_INFO("Close socket worker {}", _socketDescriptor);
        }
        else
        {
            LOG_DEV_INFO("Close socket worker {}:{}", qPrintable(_address), signed(_port));
        }
        disconnect(this, nullptr, _socket.get(), nullptr);
        disconnect(_socket.get(), nullptr, this, nullptr);
        _socket->close();
        _socket.release()->deleteLater();
    }
}

size_t SocketWorker::write(const uint8_t* buffer, const size_t length)
{
    if (!_socket)
    {
        LOG_DEV_ERR("Fail to write to null socket");
        return false;
    }
    if (!_socket->isValid())
    {
        LOG_DEV_ERR("Fail to write to invalid socket");
        return false;
    }

    const auto bytesWritten = _socket->write(reinterpret_cast<const char*>(buffer), length);
    if (bytesWritten != length)
    {
        LOG_ERR("Fail to write to socket, only wrote {}/{}", int(bytesWritten), int(length));
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
    // todo : use our own error type
    if (_socket)
    {
        LOG_ERR("Socket Error ({}) : {}", int(e), qPrintable(_socket->errorString()));
    }
    else
    {
        LOG_ERR("Socket Error ({})", int(e));
    }
    Q_EMIT socketError(int(e), _socket ? _socket->errorString() : QStringLiteral("Invalid Socket"));
}

void SocketWorker::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    LOG_INFO("New Socket state : {}", socketState);
    if (socketState == QAbstractSocket::UnconnectedState)
    {
        onDisconnected();
    }
}

void SocketWorker::onConnected()
{
    stopWatchdog();
    LOG_INFO("Socket is connected to {}:{}", qPrintable(_socket->peerAddress().toString()), _socket->peerPort());
    Q_EMIT connectionChanged(true);
    Q_EMIT startSuccess(_socket ? _socket->peerAddress().toString() : "", _socket ? _socket->peerPort() : 0, _socket ? _socket->localAddress().toString() : "", _socket ? _socket->localPort() : 0);

    startBytesCounter();
}

void SocketWorker::onDisconnected()
{
    LOG_INFO("Socket disconnected from {}:{}", qPrintable(_socket->peerAddress().toString()), _socket->peerPort());
    Q_EMIT connectionChanged(false);
    if (!_socketDescriptor)
        closeAndRestart();

    stopBytesCounter();
}

void SocketWorker::onDataAvailable()
{
    LOG_DEV_WARN("You need to override onDataAvailable");
}

bool SocketWorker::isConnected() const
{
    return _socket && _socket->state() == QAbstractSocket::ConnectedState;
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
    if (!_isRunning)
        return;

    LOG_INFO("Close and try to restart socket in {} ms", _watchdogPeriod);

    // Don't restart again
    if (_watchdog && (_watchdog->remainingTime() > 0) && _watchdog->remainingTime() < int(_watchdogPeriod))
    {
        LOG_INFO("Socket Restart timer is already running. Remaining time before restart : {} ms", _watchdog->remainingTime());
        return;
    }

    closeSocket();
    if (!_watchdog)
    {
        LOG_DEV_DEBUG("Allocate watchdog");
        _watchdog = std::make_unique<QTimer>();

        connect(_watchdog.get(), &QTimer::timeout, this, &SocketWorker::onWatchdogTimeout);
        _watchdog->setTimerType(Qt::VeryCoarseTimer);
        _watchdog->setSingleShot(true);
    }
    _watchdog->start(_watchdogPeriod);
    LOG_INFO("Start Watchdog to attempt reconnection in {} ms", signed(_watchdogPeriod));
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
