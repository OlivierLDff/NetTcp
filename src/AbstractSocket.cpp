// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application Header
#include <Net/Tcp/AbstractSocket.hpp>

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

AbstractSocket::AbstractSocket(QObject* parent): ISocket(parent)
{
}

bool AbstractSocket::setAddress(const QString& value)
{
    if (ISocket::setAddress(value))
    {
        restart();
        return true;
    }
    return false;
}

bool AbstractSocket::setPort(const quint16& value)
{
    if (ISocket::setPort(value))
    {
        restart();
        return true;
    }
    return false;
}

bool AbstractSocket::setUseWorkerThread(const bool& value)
{
    if (ISocket::setUseWorkerThread(value))
    {
        restart();
        return true;        
    }
    return false;
}

bool AbstractSocket::start()
{
    if (isRunning())
        return false;

    setRunning(true);

    return true;
}

bool AbstractSocket::start(quintptr socketDescriptor)
{
    setSocketDescriptor(socketDescriptor);
    return start();
}

bool AbstractSocket::start(const QString& host, const quint16 port)
{
    setAddress(host);
    setPort(port);
    return start();
}

bool AbstractSocket::stop()
{
    resetConnected();
    resetRunning();
    resetTxBytesPerSeconds();
    resetRxBytesPerSeconds();

    return true;
}

bool AbstractSocket::restart()
{
    if(isRunning())
    {
        stop();
        return start();        
    }
    return false;
}

void AbstractSocket::clearRxCounter()
{
    resetRxBytesPerSeconds();
    resetRxBytesTotal();
}

void AbstractSocket::clearTxCounter()
{
    resetTxBytesPerSeconds();
    resetTxBytesTotal();
}

void AbstractSocket::clearCounters()
{
    clearRxCounter();
    clearTxCounter();
}
