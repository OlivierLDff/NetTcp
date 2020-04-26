#ifndef __NETTCP_MYSOCKETWORKER_HPP__
#define __NETTCP_MYSOCKETWORKER_HPP__

#include <Net/Tcp/SocketWorker.hpp>

class MySocketWorker : public net::tcp::SocketWorker
{
    Q_OBJECT
public:
    MySocketWorker(QObject* parent = nullptr) :
        net::tcp::SocketWorker(parent) {};

private:
    bool waitingForData = false;
    std::uint8_t buffer[128] = {};
    std::uint8_t bufferLength = 0;
    std::uint8_t expectedSize = 0;

protected Q_SLOTS:
    void onConnected() override final;
    void onDataAvailable() override final;

private:
    void readHeader();

public Q_SLOTS:
    void onSendString(const QString& s);
Q_SIGNALS:
    void stringAvailable(const QString& s);
};

#endif
