#ifndef __NETTCP_SOCKET_WORKER_HPP__
#define __NETTCP_SOCKET_WORKER_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/Export.hpp>

// Dependencies Header

// Qt Header
#include <QObject>
#include <QTcpSocket>
class QTimer;

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace Net {
namespace Tcp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETTCP_API_ SocketWorker : public QObject
{
    Q_OBJECT
    // ──────── CONSTRUCTOR ────────
public:
    SocketWorker(QObject* parent = nullptr);
    virtual ~SocketWorker();

    // ──────── ATTRIBUTES ────────
private:
    bool _isRunning = false;
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
    size_t write(const uint8_t* buffer, const size_t length);
    size_t write(const char* buffer, const size_t length);

    // ──────── READ API ────────
public Q_SLOTS:
    virtual void onDataAvailable();
public:
    size_t bytesAvailable() const;
    size_t read(uint8_t* data, size_t maxLen);
    size_t read(char* data, size_t maxLen);

    // ──────── COMMUNICATION TO SOCKET ────────
protected Q_SLOTS:
    void onSocketError(const QAbstractSocket::SocketError e);
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    virtual void onConnected();
    virtual void onDisconnected();

Q_SIGNALS:
    void startSuccess(const QString& s, const quint16 port);
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
