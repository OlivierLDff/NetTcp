#include <MySocket.hpp>

std::unique_ptr<net::tcp::SocketWorker> MySocket::createWorker()
{
    auto worker = std::make_unique<MySocketWorker>();

    // Send string to worker
    connect(this, &MySocket::sendString, worker.get(),
        &MySocketWorker::onSendString);

    // Receive string from worker
    connect(worker.get(), &MySocketWorker::stringAvailable, this,
        &MySocket::stringReceived);

    return std::move(worker);
}
