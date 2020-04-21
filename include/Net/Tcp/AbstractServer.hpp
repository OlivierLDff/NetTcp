#ifndef __NETTCP_ABSTRACT_SERVER_HPP__
#define __NETTCP_ABSTRACT_SERVER_HPP__

// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/IServer.hpp>

// Dependencies
#include <ObjectListModel.hpp>

// ───── DECLARATION ─────

namespace Net {
namespace Tcp {

// ───── CLASS ─────

class NETTCP_API_ AbstractServer : public IServer
{
    Q_OBJECT
    NETTCP_REGISTER_TO_QML(AbstractServer)

    // ──────── CONSTRUCTOR ────────
public:
    AbstractServer(QObject* parent = nullptr,
        const QList<QByteArray> & exposedRoles = {},
        const QByteArray & displayRole = {},
        const QByteArray& uidRole = {}) :
    IServer(parent, exposedRoles, displayRole, uidRole) {}

    // ──────── C++ API ────────
public Q_SLOTS:
    bool start() override;
    bool start(const quint16 port) override;
    bool start(const QString& address, const quint16 port) override;
    bool stop() override;
    bool restart() override;

    AbstractSocket* getSocket(const QString& address, const quint16 port) override;
    QList<AbstractSocket*> getSocket(const QString& address) override;
    void disconnectFrom(const QString& address, const quint16 port) override;
};

}
}

#endif
