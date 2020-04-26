#ifndef __NETTCP_SOCKET_HPP__
#define __NETTCP_SOCKET_HPP__

// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/ISocket.hpp>

// Stl Headers
#include <memory>

// ───── DECLARATION ─────

class QThread;

namespace net {
namespace tcp {

class SocketWorker;

// ───── CLASS ─────

class Socket : public ISocket
{
    Q_OBJECT
    NETTCP_REGISTER_TO_QML(Socket);

    // ──────── CONSTRUCTOR ────────
public:
    Socket(QObject* parent = nullptr);
    ~Socket();

    // ──────── ATTRIBUTE ────────
public:
    bool setPeerAddress(const QString& value) override;
    bool setPeerPort(const quint16& value) override;
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

    // ──────── WORKER ────────
private:
    std::unique_ptr<SocketWorker> _worker;
    std::unique_ptr<QThread> _workerThread;

private Q_SLOTS:
    void killWorker();
    void onStartSuccess(const QString& peerAddress, const quint16 peerPort,
        const QString& localAddress, const quint16 localPort);
    void onStartFail();
    void onBytesReceived(const uint64_t count);
    void onBytesSent(const uint64_t count);

Q_SIGNALS:
    void startWorker();
    void stopWorker();

    // ──────── CUSTOM WORKER API ────────
protected:
    virtual std::unique_ptr<class SocketWorker> createWorker();
};

}
}

#endif
