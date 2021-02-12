#ifndef __NETTCP_SERVER_WORKER_HPP__
#define __NETTCP_SERVER_WORKER_HPP__

// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/Export.hpp>

// Qt Headers
#include <QtNetwork/QTcpServer>

// ───── DECLARATION ─────

namespace net {
namespace tcp {

// ───── CLASS ─────

class NETTCP_API_ ServerWorker : public QTcpServer
{
    Q_OBJECT
    // ──────── CONSTRUCTOR ────────
public:
    ServerWorker(QObject* parent = nullptr);

    // ──────── QTCPSERVER OVERRIDE ────────
protected:
    void incomingConnection(qintptr handle) override;

Q_SIGNALS:
    void newIncomingConnection(qintptr handle);
};

}
}

#endif
