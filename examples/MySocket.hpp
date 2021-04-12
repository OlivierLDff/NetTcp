#ifndef __NETTCP_MYSOCKET_HPP__
#define __NETTCP_MYSOCKET_HPP__

#include <MySocketWorker.hpp>
#include <Net/Tcp/Socket.hpp>

class MySocket : public net::tcp::Socket
{
    Q_OBJECT
public:
    MySocket(QObject* parent = nullptr) : net::tcp::Socket(parent) {};

protected:
    net::tcp::SocketWorker* createWorker() override;

Q_SIGNALS:
    void sendString(const QString& s);
    void sendErrorString();
    void stringReceived(const QString& s);
};
#endif
