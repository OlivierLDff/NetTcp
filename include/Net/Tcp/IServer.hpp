#ifndef __NETTCP_ISERVER_HPP__
#define __NETTCP_ISERVER_HPP__

// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/Socket.hpp>

// Dependencies Headers
#include <QOlm/QOlm.hpp>

// ───── DECLARATION ─────

namespace net {
namespace tcp {

// ───── CLASS ─────

/** Interface Server class that define API and basic property */
class NETTCP_API_ IServer : public qolm::QOlm<Socket>
{
    Q_OBJECT

    // ──────── CONSTRUCTOR ────────
public:
    IServer(QObject* parent = nullptr,
        const QList<QByteArray>& exposedRoles = {},
        const QByteArray& displayRole = {}) :
        QOlm<Socket>(parent, exposedRoles, displayRole)
    {
    }

    // ──────── ATTRIBUTES ────────
protected:
    NETTCP_PROPERTY_RO(bool, isRunning, Running);
    NETTCP_PROPERTY_RO(bool, isListening, Listening);
    NETTCP_PROPERTY_D(quint64, watchdogPeriod, WatchdogPeriod, 1000);
    NETTCP_PROPERTY(QString, address, Address);
    NETTCP_PROPERTY(quint16, port, Port);
    NETTCP_PROPERTY(bool, useWorkerThread, UseWorkerThread);

    // ──────── C++ API ────────
public Q_SLOTS:
    virtual bool start() = 0;
    virtual bool start(const quint16 port) = 0;
    virtual bool start(const QString& address, const quint16 port) = 0;
    virtual bool stop() = 0;
    virtual bool restart() = 0;

    virtual Socket* getSocket(
        const QString& address, const quint16 port) const = 0;
    virtual QList<Socket*> getSockets(const QString& address) const = 0;
    virtual void disconnectFrom(const QString& address, const quint16 port) = 0;
    virtual void disconnectFrom(const QString& address) = 0;

    // ──────── SIGNALS ────────
Q_SIGNALS:
    void acceptError(int error, const QString description);
    void newClient(const QString& address, const quint16 port);
    void clientLost(const QString& address, const quint16 port);
};

}
}

#endif
