#include <MyServer.hpp>
#include <MySocket.hpp>

net::tcp::Socket* MyServer::newSocket(QObject* parent)
{
    const auto socket = new MySocket(parent);
    socket->setUseWorkerThread(multiThreaded);
    connect(socket, &MySocket::stringReceived, this,
        [this, socket](const QString& string)
        {
            if(sendError)
                Q_EMIT socket->sendErrorString();
            else
                Q_EMIT socket->sendString(string);
            Q_EMIT stringReceived(
                string, socket->peerAddress(), socket->peerPort());
        });
    return socket;
}
