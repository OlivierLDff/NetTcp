#ifndef __NETTCP_SOCKET_HPP__
#define __NETTCP_SOCKET_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/AbstractSocket.hpp>

class QThread;

// C++ Header
#include <memory>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace Net {
namespace Tcp {

class SocketWorker;

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class SocketImpl : public QObject
{
    Q_OBJECT
    // ──────── CONSTRUCTOR ────────
public:
    SocketImpl(QObject* parent = nullptr) : QObject(parent) {}

    // ──────── COMMUNICATION ────────
Q_SIGNALS:
    void startWorker();
    void stopWorker();    
};

class Socket : public AbstractSocket
{
    Q_OBJECT
    NETTCP_REGISTER_TO_QML(Socket);

    // ──────── CONSTRUCTOR ────────
public:
    Socket(QObject* parent = nullptr);
    ~Socket();

    // ──────── WORKER ────────
protected:
    std::unique_ptr<SocketImpl> _impl;
    std::unique_ptr<SocketWorker> _worker;
    std::unique_ptr<QThread> _workerThread;

    // ──────── C++ API ────────
public Q_SLOTS:
    bool start() override;
    bool start(quintptr socketDescriptor) override;
    bool start(const QString& host, const quint16 port) override;
    bool stop() override;

private:
    void killWorker();

    // ──────── CUSTOM WORKER API ────────
protected:
    virtual std::unique_ptr<class SocketWorker> createWorker();
};

}
}

#endif
