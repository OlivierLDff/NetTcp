#ifndef __NETTCP_MYSERVER_HPP__
#define __NETTCP_MYSERVER_HPP__

#include <MySocket.hpp>
#include <Net/Tcp/Server.hpp>

class MyServer : public Net::Tcp::Server
{
    Q_OBJECT
public:
    bool multiThreaded = false;
protected:
    Net::Tcp::AbstractSocket* newTcpSocket(QObject* parent) override;

Q_SIGNALS:
    void stringReceived(QString string, QString address, quint16 port);
};

#endif
