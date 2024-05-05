
// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

#include <MySocket.hpp>

// Dependencies
#include <Net/Tcp/NetTcp.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>
#ifdef _MSC_VER
#    include <spdlog/sinks/msvc_sink.h>
#endif

// Qt
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTimer>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

std::shared_ptr<spdlog::logger> appLog = std::make_shared<spdlog::logger>("app");
std::shared_ptr<spdlog::logger> clientLog = std::make_shared<spdlog::logger>("client");

class App
{
public:
    int counter = 0;

    uint16_t port = 9999;
    QString ip = QStringLiteral("127.0.0.1");
    MySocket client;

    bool multiThreaded = false;

    QTimer timer;

public:
    void start()
    {
        appLog->info("Init application");

        // Send Echo counter every seconds
        QObject::connect(&timer, &QTimer::timeout,
            [this]()
            {
                if(client.isConnected())
                {
                    Q_EMIT client.sendString("Echo " + QString::number(counter++));
                }
            });
        // Print the message that echoed from server socket
        QObject::connect(&client, &MySocket::stringReceived,
            [this](const QString value)
            {
                clientLog->info("Rx \"{}\" from server {}:{}", qPrintable(value), qPrintable(client.peerAddress()),
                    int(client.peerPort()));
            });
        QObject::connect(&client, &net::tcp::Socket::isRunningChanged,
            [](bool value) { clientLog->info("isRunning : {}", value); });
        QObject::connect(&client, &net::tcp::Socket::isConnectedChanged,
            [this](bool value)
            {
                clientLog->info("isConnected : {}", value);
                // Reset counter at connection/disconnection
                counter = 0;
            });
        QObject::connect(&client, &net::tcp::Socket::socketError,
            [](int value, const QString& error) { clientLog->error("socket error : {}", error.toStdString()); });
        QObject::connect(&client, &net::tcp::Socket::txBytesTotalChanged,
            [](quint64 total) { clientLog->info("Sent bytes : {}", total); });

        client.setUseWorkerThread(multiThreaded);
        clientLog->info("Start client to connect to address {}, on port {}", qPrintable(ip), int(port));
        client.start(ip, port);

        appLog->info("Start application");
        timer.start(1000);
    }
};

static void installLoggers()
{
#ifdef _MSC_VER
    const auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    msvcSink->set_level(spdlog::level::debug);
    net::tcp::Logger::registerSink(msvcSink);
    appLog->sinks().emplace_back(msvcSink);
    clientLog->sinks().emplace_back(msvcSink);
#endif

    const auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdoutSink->set_level(spdlog::level::debug);
    net::tcp::Logger::registerSink(stdoutSink);
    appLog->sinks().emplace_back(stdoutSink);
    clientLog->sinks().emplace_back(stdoutSink);
}

int main(int argc, char* argv[])
{
    installLoggers();

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
    net::tcp::registerQmlTypes();

    // Create the app and start it
    App echo;
    bool ok;
    const auto port = parser.value(portOption).toInt(&ok);
    if(ok)
        echo.port = port;
    const auto ip = parser.value(ipOption);
    if(!ip.isEmpty())
        echo.ip = ip;
    echo.multiThreaded = parser.isSet(multiThreadOption);

    echo.start();

    // Start event loop
    return QCoreApplication::exec();
}
