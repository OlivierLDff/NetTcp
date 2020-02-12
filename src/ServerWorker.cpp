// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/ServerWorker.hpp>

// Dependencies Header

// Qt Header

// STL Header

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace Net::Tcp;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

ServerWorker::ServerWorker(QObject* parent): QTcpServer(parent)
{
}

void ServerWorker::incomingConnection(qintptr handle)
{
    Q_EMIT newIncomingConnection(handle);
}
