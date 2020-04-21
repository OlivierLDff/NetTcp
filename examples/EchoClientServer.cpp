
// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Dependencies
#include <Net/Tcp/NetTcp.hpp>

// Qt
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QTimer>
#include <QCoreApplication>
#include <QNetworkInterface>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

Q_LOGGING_CATEGORY(APP_LOG_CAT, "app")
Q_LOGGING_CATEGORY(APP_SERVER_LOG_CAT, "server")
Q_LOGGING_CATEGORY(APP_CLIENT_LOG_CAT, "client")

class MySocketWorker : public Net::Tcp::SocketWorker
{
    Q_OBJECT
public:
    MySocketWorker(QObject* parent = nullptr) : Net::Tcp::SocketWorker(parent) {}

private:
    bool waitingForData = false;
    uint8_t buffer[128] = {};
    uint8_t bufferLength = 0;
    uint8_t expectedSize = 0;

protected Q_SLOTS:
    void onConnected() override final
    {
        Net::Tcp::SocketWorker::onConnected();
        waitingForData = false;
        bufferLength = 0;
    }

    void readHeader()
    {
        // Only read if something is available
        if (!bytesAvailable())
            return;

        // Only read if in correct state
        if (waitingForData)
            return;

        // Read 1 byte
        if (!read(&expectedSize, 1))
            return closeAndRestart();

        // Check header is valid
        if (expectedSize == 0 || expectedSize >= 128)
            return closeAndRestart();

        // Go to next state waiting for data
        waitingForData = true;
    }

    void onDataAvailable() override
    {
        // Read header if not done
        if (!waitingForData)
            readHeader();

        // Otherwise read maximum number of bytes expected
        while(waitingForData && bytesAvailable())
        {
            // Read maximum data until the whole packet have been read.
            const auto bytesRead = read(buffer + bufferLength, expectedSize - bufferLength);
            bufferLength += uint8_t(bytesRead);

            // Emit the received string when read is complete
            if(bufferLength == expectedSize)
            {
                QString s(reinterpret_cast<char*>(buffer));
                Q_EMIT stringAvailable(s);
                waitingForData = false;
                bufferLength = 0;
                readHeader();
            }
        }
    }

public Q_SLOTS:
    void onSendString(const QString& s)
    {
        const auto data = s.toStdString();
        // Max packet size is 128
        if (data.length() >= 128)
            return;

        quint8 size = uint8_t(data.length() + 1);
        // Write header
        if (!write(&size, 1))
            return closeAndRestart();
        // Write data
        if (!write(data.c_str(), size))
            return closeAndRestart();
    }
Q_SIGNALS:
    void stringAvailable(const QString& s);
};

class MySocket : public Net::Tcp::Socket
{
    Q_OBJECT
public:
    MySocket(QObject* parent = nullptr) : Net::Tcp::Socket(parent) {}

protected:
    std::unique_ptr<Net::Tcp::SocketWorker> createWorker() override
    {
        auto worker = std::make_unique<MySocketWorker>();

        // Send string to worker
        connect(this, &MySocket::sendString, worker.get(), &MySocketWorker::onSendString);

        // Receive string from worker
        connect(worker.get(), &MySocketWorker::stringAvailable, this, &MySocket::stringReceived);

        return std::move(worker);
    }

Q_SIGNALS:
    void sendString(const QString& s);
    void stringReceived(const QString& s);
};

class MyServer : public Net::Tcp::Server
{
    Q_OBJECT
public:
    bool multiThreaded = false;
protected:
    Net::Tcp::AbstractSocket* newTcpSocket(QObject* parent) override
    {
        const auto s = new MySocket(parent);
        s->setUseWorkerThread(multiThreaded);
        connect(s, &MySocket::stringReceived, [this, s](const QString& string)
            {
                qCInfo(APP_SERVER_LOG_CAT, "RX \"%s\" from client %s:%d", qPrintable(string), qPrintable(s->peerAddress()), signed(s->peerPort()));
                Q_EMIT s->sendString(string);
            });
        return s;
    }
};

#include "EchoClientServer.moc"

class App
{
public:
    int counter = 0;

    uint16_t port = 9999;
    QString ip = QStringLiteral("127.0.0.1");

    MyServer server;
    MySocket client;

    bool multiThreaded = false;

    QTimer timer;

public:
    void start()
    {
        qCInfo(APP_LOG_CAT, "Init application");

        server.multiThreaded = multiThreaded;

        // Send Echo counter every seconds
        QObject::connect(&timer, &QTimer::timeout, [this]()
            {
                Q_EMIT client.sendString("Echo " + QString::number(counter++));
            });
        // Print the message that echoed from server socket
        QObject::connect(&client, &MySocket::stringReceived, [this](const QString value)
            {
                qCInfo(APP_CLIENT_LOG_CAT, "Rx \"%s\" server client %s:%d", qPrintable(value), qPrintable(client.peerAddress()), signed(client.peerPort()));
            });

        QObject::connect(&server, &Net::Tcp::Server::isRunningChanged, [](bool value)
            {
                qCInfo(APP_SERVER_LOG_CAT, "isRunning : %d", signed(value));
            });
        QObject::connect(&server, &Net::Tcp::Server::isListeningChanged, [](bool value)
            {
                qCInfo(APP_SERVER_LOG_CAT, "isBounded : %d", signed(value));
            });
        QObject::connect(&client, &Net::Tcp::Socket::isRunningChanged, [](bool value)
            {
                qCInfo(APP_CLIENT_LOG_CAT, "isRunning : %d", signed(value));
            });
        QObject::connect(&client, &Net::Tcp::Socket::isConnectedChanged, [](bool value)
            {
                qCInfo(APP_CLIENT_LOG_CAT, "isConnected : %d", signed(value));
            });
        QObject::connect(&server, &Net::Tcp::Server::acceptError, [](int value, const QString& error)
            {
                qCInfo(APP_SERVER_LOG_CAT, "error : %s", qPrintable(error));
            });
        QObject::connect(&client, &Net::Tcp::Socket::socketError, [](int value, const QString& error)
            {
                qCInfo(APP_CLIENT_LOG_CAT, "error : %s", qPrintable(error));
            });
        QObject::connect(&server, &Net::Tcp::Server::newClient, [](const QString& address, const quint16 port)
            {
                qCInfo(APP_SERVER_LOG_CAT, "New Client %s:%d", qPrintable(address), signed(port));
            });
        QObject::connect(&server, &Net::Tcp::Server::clientLost, [](const QString& address, const quint16 port)
            {
                qCInfo(APP_SERVER_LOG_CAT, "Client Disconnected %s:%d", qPrintable(address), signed(port));
            });
        QObject::connect(&client, &Net::Tcp::Socket::txBytesTotalChanged, [](quint64 total)
            {
                qCInfo(APP_CLIENT_LOG_CAT, "Send bytes %llu", (long long unsigned)(total));
            });

        qCInfo(APP_SERVER_LOG_CAT, "Start server on address %s:%d", qPrintable(ip), signed(port));
        // server.start(port) can be called to listen from every interfaces
        server.start(ip, port);

        client.setUseWorkerThread(multiThreaded);
        qCInfo(APP_CLIENT_LOG_CAT, "Start client to connect to address %s, to port %d", qPrintable(ip), signed(port));
        client.start(ip, port);

        qCInfo(APP_LOG_CAT, "Start application");
        timer.start(1000);
    }
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // ────────── COMMAND PARSER ──────────────────────────────────────

    QCommandLineParser parser;
    parser.setApplicationDescription("Echo Client Server");
    parser.addHelpOption();

    QCommandLineOption multiThreadOption(QStringList() << "t",
        QCoreApplication::translate("main", "Make the worker live in a different thread. Default false"));
    parser.addOption(multiThreadOption);

    QCommandLineOption portOption(QStringList() << "s" << "src",
        QCoreApplication::translate("main", "Port for rx packet. Default \"9999\"."),
        QCoreApplication::translate("main", "port"));
    portOption.setDefaultValue("9999");
    parser.addOption(portOption);

    QCommandLineOption ipOption(QStringList() << "i" << "ip",
        QCoreApplication::translate("main", "Ip address of multicast group. Default \"127.0.0.1\""),
        QCoreApplication::translate("main", "ip"));
    ipOption.setDefaultValue(QStringLiteral("127.0.0.1"));
    parser.addOption(ipOption);

    // Process the actual command line arguments given by the user
    parser.process(app);

    // ────────── APPLICATION ──────────────────────────────────────

    // Register types for to use SharedDatagram in signals
    Net::Tcp::Utils::registerTypes();

    // Create the app and start it
    App echo;
    bool ok;
    const auto port = parser.value(portOption).toInt(&ok);
    if (ok)
        echo.port = port;
    const auto ip = parser.value(ipOption);
    if (!ip.isEmpty())
        echo.ip = ip;
    echo.multiThreaded = parser.isSet(multiThreadOption);

    echo.start();

    // Start event loop
    return QCoreApplication::exec();
}
