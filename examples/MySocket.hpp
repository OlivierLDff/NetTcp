#ifndef __NETTCP_MYSOCKET_HPP__
#define __NETTCP_MYSOCKET_HPP__

#include <MySocketWorker.hpp>
#include <Net/Tcp/Socket.hpp>

class MySocket : public Net::Tcp::Socket
{
    Q_OBJECT
public:
    MySocket(QObject* parent = nullptr) : Net::Tcp::Socket(parent) {};

protected:
    std::unique_ptr<Net::Tcp::SocketWorker> createWorker() override;

Q_SIGNALS:
    void sendString(const QString& s);
    void stringReceived(const QString& s);
};
#endif
