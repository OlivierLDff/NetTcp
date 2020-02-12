#ifndef __NETTCP_ABSTRACT_SOCKET_HPP__
#define __NETTCP_ABSTRACT_SOCKET_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/ISocket.hpp>

// Dependencies Header

// Qt Header

// C++ Header
//
// C Header

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace Net {
namespace Tcp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETTCP_API_ AbstractSocket : public ISocket
{
    Q_OBJECT
    NETTCP_REGISTER_TO_QML(AbstractSocket)
    // ──────── CONSTRUCTOR ────────
public:
    AbstractSocket(QObject* parent = nullptr);

    // ──────── ATTRIBUTE ────────
public:
    bool setAddress(const QString& value) override;
    bool setPort(const quint16& value) override;
    bool setUseWorkerThread(const bool& value) override;

    // ──────── C++ API ────────
public Q_SLOTS:
    bool start() override;
    bool start(quintptr socketDescriptor) override;
    bool start(const QString& host, const quint16 port) override;
    bool stop() override;
    bool restart() override;

    void clearRxCounter() override;
    void clearTxCounter() override;
    void clearCounters() override;
};

}
}

#endif