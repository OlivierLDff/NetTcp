#ifndef __NETTCP_MYSERVER_HPP__
#define __NETTCP_MYSERVER_HPP__

#include <Net/Tcp/Server.hpp>

class MyServer : public net::tcp::Server
{
    Q_OBJECT
public:
    bool multiThreaded = false;

protected:
    net::tcp::Socket* newSocket(QObject* parent) override;

Q_SIGNALS:
    void stringReceived(QString string, QString address, quint16 port);
};

#endif
