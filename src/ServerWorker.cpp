// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/ServerWorker.hpp>

// ───── DECLARATION ─────

using namespace net::tcp;

// ───── CLASS ─────

ServerWorker::ServerWorker(QObject* parent) : QTcpServer(parent) {}

void ServerWorker::incomingConnection(qintptr handle)
{
    Q_EMIT newIncomingConnection(handle);
}
