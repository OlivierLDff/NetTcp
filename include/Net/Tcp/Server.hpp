#ifndef __NETTCP_SERVER_HPP__
#define __NETTCP_SERVER_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/AbstractServer.hpp>

class QTimer;

// C++ Header
#include <memory>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace Net {
namespace Tcp {

class ServerWorker;

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

class NETTCP_API_ Server : public AbstractServer
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
    bool start() override;
    bool start(const quint16 port) override;
    bool start(const QString& address, const quint16 port) override;
    bool stop() override;

    // ──────── CUSTOM SOCKET API ────────
protected Q_SLOTS:
    virtual class AbstractSocket* newTcpSocket(QObject* parent);

    // ──────── PRIVATE ────────
private:
    /** Try to start the server. If fail, start a watchdog timer that will retry later */
    bool tryStart();
    bool startWorker();
    bool stopWorker();
    void startWatchdog();
    void stopWatchdog();

protected:
    void onItemInserted(AbstractSocket* item, int row) override;
    void onItemAboutToBeRemoved(AbstractSocket* item, int row) override;
private:
    std::unique_ptr<ServerWorker> _worker;
    std::unique_ptr<QTimer> _watchdog;

private Q_SLOTS:
    void onWatchdogTimeout();
    void onNewIncomingConnection(qintptr handle);
};

}
}

#endif
