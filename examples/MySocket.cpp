#include <MySocket.hpp>

net::tcp::SocketWorker* MySocket::createWorker()
{
    auto worker = new MySocketWorker;

    // Send string to worker
    connect(this, &MySocket::sendString, worker, &MySocketWorker::onSendString);
    connect(this, &MySocket::sendErrorString, worker, &MySocketWorker::onSendErrorString);

    // Receive string from worker
    connect(worker, &MySocketWorker::stringAvailable, this, &MySocket::stringReceived);

    return worker;
}
