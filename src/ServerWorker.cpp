// ───── INCLUDE ─────

// Library Headers
#include <Net/Tcp/ServerWorker.hpp>

// ───── DECLARATION ─────

using namespace Net::Tcp;

// ───── CLASS ─────

ServerWorker::ServerWorker(QObject* parent) : QTcpServer(parent) {}

void ServerWorker::incomingConnection(qintptr handle)
{
    Q_EMIT newIncomingConnection(handle);
}
