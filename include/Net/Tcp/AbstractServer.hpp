#ifndef __NETTCP_ABSTRACT_SERVER_HPP__
#define __NETTCP_ABSTRACT_SERVER_HPP__

// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/IServer.hpp>

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
        const QByteArray & displayRole = {}) :
    IServer(parent, exposedRoles, displayRole) {}

    // ──────── C++ API ────────
public Q_SLOTS:
    bool start() override;
    bool start(const quint16 port) override;
    bool start(const QString& address, const quint16 port) override;
    bool stop() override;
    bool restart() override;

    Socket* getSocket(const QString& address, const quint16 port) override final;
    QList<Socket*> getSockets(const QString& address) override final;
    void disconnectFrom(const QString& address, const quint16 port) override final;
};

}
}

#endif
