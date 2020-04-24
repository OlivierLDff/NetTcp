#include <MyServer.hpp>
#include <MySocket.hpp>

Net::Tcp::Socket* MyServer::newSocket(QObject* parent)
{
    const auto socket = new MySocket(parent);
    socket->setUseWorkerThread(multiThreaded);
    connect(socket, &MySocket::stringReceived, this,
        [this, socket](const QString& string)
        {
            Q_EMIT socket->sendString(string);
            Q_EMIT stringReceived(string, socket->peerAddress(), socket->peerPort());
        }
    );
    return socket;
}
