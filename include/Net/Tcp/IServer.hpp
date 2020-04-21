#ifndef __NETTCP_ISERVER_HPP__
#define __NETTCP_ISERVER_HPP__

// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/AbstractSocket.hpp>

// Dependencies Headers
#include <ObjectListModel.hpp>

// ───── DECLARATION ─────

namespace Net {
namespace Tcp {

// ───── CLASS ─────

class NETTCP_API_ IServer : public Olm::ObjectList<AbstractSocket>
{
    Q_OBJECT
    // ──────── CONSTRUCTOR ────────
public:
    IServer(QObject* parent = nullptr,
        const QList<QByteArray> & exposedRoles = {},
        const QByteArray & displayRole = {},
        const QByteArray& uidRole = {}) :
    Olm::ObjectList<AbstractSocket>(parent, exposedRoles, displayRole, uidRole) {}

    // ──────── ATTRIBUTES ────────
protected:
    NETTCP_PROPERTY_RO          (bool, isRunning, Running);
    NETTCP_PROPERTY_RO          (bool, isListening, Listening);
    NETTCP_PROPERTY_D           (quint64, watchdogPeriod, WatchdogPeriod, 1000);
    NETTCP_PROPERTY             (QString, address, Address);
    NETTCP_PROPERTY             (quint16, port, Port);
    NETTCP_PROPERTY             (bool, useWorkerThread, UseWorkerThread);

    // ──────── C++ API ────────
public Q_SLOTS:
    virtual bool start() = 0;
    virtual bool start(const quint16 port) = 0;
    virtual bool start(const QString& address, const quint16 port) = 0;
    virtual bool stop() = 0;
    virtual bool restart() = 0;

    virtual AbstractSocket* getSocket(const QString& address, const quint16 port) = 0;
    virtual QList<AbstractSocket*> getSocket(const QString& address) = 0;
    virtual void disconnectFrom(const QString& address, const quint16 port) = 0;

    // ──────── SIGNALS ────────
Q_SIGNALS:
    void acceptError(int error, const QString description);
    void newClient(const QString& address, const quint16 port);
    void clientLost(const QString& address, const quint16 port);
};

}
}

#endif
