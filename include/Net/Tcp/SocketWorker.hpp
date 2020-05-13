#ifndef __NETTCP_SOCKET_WORKER_HPP__
#define __NETTCP_SOCKET_WORKER_HPP__

// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/Export.hpp>

// Qt Headers
#include <QObject>
#include <QTcpSocket>

// STL Header
#include <memory>

// ───── DECLARATION ─────

class QTimer;

namespace net {
namespace tcp {

// ───── CLASS ─────

class NETTCP_API_ SocketWorker : public QObject
{
    Q_OBJECT
    // ──────── CONSTRUCTOR ────────
public:
    SocketWorker(QObject* parent = nullptr);
    ~SocketWorker();

    // ──────── ATTRIBUTES ────────
private:
    bool _isRunning = false;
    // Use to avoid multiple call to onConnected/onDisconnected
    bool _isConnected = false;
    // Use to avoid nested call of closeSocket()
    bool _pendingClosing = false;
    quintptr _socketDescriptor = 0;
    QString _address;
    quint16 _port = 0;
    std::unique_ptr<QTcpSocket> _socket = nullptr;

    // ──────── CONTROL FROM SOCKET API ────────
public Q_SLOTS:
    void onStart();
    void onStop();
    void closeSocket();
    void closeAndRestart();

    // ──────── WRITE API ────────
public:
    std::size_t write(const std::uint8_t* buffer, const std::size_t length);
    std::size_t write(const char* buffer, const std::size_t length);

    // ──────── READ API ────────
protected Q_SLOTS:
    bool isConnected() const;
    virtual void onDataAvailable();

public:
    std::size_t bytesAvailable() const;
    std::size_t read(std::uint8_t* data, std::size_t maxLen);
    std::size_t read(char* data, std::size_t maxLen);

    // ──────── COMMUNICATION TO SOCKET ────────
protected Q_SLOTS:
    void onSocketError(const QAbstractSocket::SocketError e);
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    virtual void onConnected();
    virtual void onDisconnected();

Q_SIGNALS:
    void startSuccess(const QString& peerAddress, const quint16 peerPort,
        const QString& localAddress, const quint16 localPort);
    void startFailed();
    void connectionChanged(bool connected);
    void socketError(int error, QString description);

    // ──────── WATCHDOG ────────
private Q_SLOTS:
    void onWatchdogPeriodChanged(quint64 period);
    void onWatchdogTimeout();
    void stopWatchdog();

private:
    uint64_t _watchdogPeriod = 1000;
    std::unique_ptr<QTimer> _watchdog;

    // ──────── STATISTICS ────────
private:
    quint64 _rxBytesCounter = 0;
    quint64 _txBytesCounter = 0;
    std::unique_ptr<QTimer> _bytesCounterTimer;

protected:
    void startBytesCounter();
    virtual void stopBytesCounter();

protected Q_SLOTS:
    virtual void updateDataCounter();

Q_SIGNALS:
    void bytesReceived(const quint64 rx);
    void bytesSent(const quint64 tx);

    // ──────── FRIENDS ────────
private:
    friend class Socket;
};

}
}

#endif
