#ifndef __NETTCP_SERVER_HPP__
#define __NETTCP_SERVER_HPP__

// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/IServer.hpp>

// ───── DECLARATION ─────

QT_FORWARD_DECLARE_CLASS(QTimer);

namespace net {
namespace tcp {

class ServerWorker;

// ───── CLASS ─────

class NETTCP_API_ Server : public IServer
{
    Q_OBJECT
    NETTCP_REGISTER_TO_QML(Server)

    // ──────── CONSTRUCTOR ────────
public:
    Server(QObject* parent = nullptr);
    ~Server();

    // ──────── ATTRIBUTES ────────
public:
    bool setWatchdogPeriod(const quint64& value) override;
    bool setAddress(const QString& value) override;
    bool setPort(const quint16& value) override;
    bool setUseWorkerThread(const bool& value) override;

    // ──────── C++ API ────────
public Q_SLOTS:
    bool start() override final;
    bool start(const quint16 port) override final;
    bool start(const QString& address, const quint16 port) override final;
    bool stop() override final;
    bool restart() override final;
    Socket* getSocket(const QString& address, const quint16 port) const override final;
    QList<Socket*> getSockets(const QString& address) const override final;
    void disconnectFrom(const QString& address, const quint16 port) override final;
    void disconnectFrom(const QString& address) override final;

protected:
    virtual bool canAcceptNewClient() const;

    // ──────── CUSTOM SOCKET API ────────
protected:
    virtual class Socket* newSocket(QObject* parent);

    // ──────── PRIVATE ────────
private:
    /** Try to start the server. If fail, start a watchdog timer that will retry later */
    bool tryStart();
    bool startWorker();
    bool stopWorker();
    void startWatchdog();
    void stopWatchdog();

private:
    ServerWorker* _worker = nullptr;
    QTimer* _watchdog = nullptr;
};

}
}

#endif
