#ifndef __NETTCP_ISOCKET_HPP__
#define __NETTCP_ISOCKET_HPP__

// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/Export.hpp>
#include <Net/Tcp/Property.hpp>

// ───── DECLARATION ─────

namespace net {
namespace tcp {

// ───── CLASS ─────

class NETTCP_API_ ISocket : public QObject
{
    Q_OBJECT
    // ──────── CONSTRUCTOR ────────
public:
    ISocket(QObject* parent = nullptr) : QObject(parent) {}

    // ──────── ATTRIBUTE STATE ────────
protected:
    NETTCP_PROPERTY_RO(bool, isRunning, Running);
    NETTCP_PROPERTY_RO(bool, isConnected, Connected);
    NETTCP_PROPERTY_D(quint64, watchdogPeriod, WatchdogPeriod, 5000);

    // ──────── ATTRIBUTE ────────
protected:
    NETTCP_PROPERTY_RO(QString, localAddress, LocalAddress);
    NETTCP_PROPERTY_RO(quint16, localPort, LocalPort);
    NETTCP_PROPERTY(QString, peerAddress, PeerAddress);
    NETTCP_PROPERTY(quint16, peerPort, PeerPort);
    NETTCP_PROPERTY(quintptr, socketDescriptor, SocketDescriptor);
    NETTCP_PROPERTY(bool, useWorkerThread, UseWorkerThread);
    NETTCP_PROPERTY_D(bool, noDelay, NoDelay, true);

    // ──────── STATUS ────────
protected:
    NETTCP_PROPERTY_RO(quint64, rxBytesPerSeconds, RxBytesPerSeconds);
    NETTCP_PROPERTY_RO(quint64, txBytesPerSeconds, TxBytesPerSeconds);
    NETTCP_PROPERTY_RO(quint64, rxBytesTotal, RxBytesTotal);
    NETTCP_PROPERTY_RO(quint64, txBytesTotal, TxBytesTotal);

    // ──────── C++ API ────────
public Q_SLOTS:
    virtual bool start() = 0;
    virtual bool start(quintptr socketDescriptor) = 0;
    virtual bool start(const QString& host, const quint16 port) = 0;
    virtual bool stop() = 0;
    virtual bool restart() = 0;

    virtual void clearRxCounter() = 0;
    virtual void clearTxCounter() = 0;
    virtual void clearCounters() = 0;

    // ──────── SIGNALS ────────
Q_SIGNALS:
    void socketError(int error, const QString description);
    void startSuccess(const QString& address, const quint16 port);
    void startFailed();
};

}
}

#endif
