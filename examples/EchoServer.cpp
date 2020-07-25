
// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Application
#include <MyServer.hpp>

// Dependencies
#include <Net/Tcp/NetTcp.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>
#ifdef _MSC_VER
#    include <spdlog/sinks/msvc_sink.h>
#endif

// Qt
#include <QCommandLineParser>
#include <QCoreApplication>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

std::shared_ptr<spdlog::logger> appLog =
    std::make_shared<spdlog::logger>("app");
std::shared_ptr<spdlog::logger> serverLog =
    std::make_shared<spdlog::logger>("server");

class App
{
public:
    uint16_t port = 9999;
    QString ip = QStringLiteral("127.0.0.1");

    MyServer server;

    bool multiThreaded = false;

public:
    void start()
    {
        appLog->info("Init application");

        server.multiThreaded = multiThreaded;

        QObject::connect(&server, &MyServer::stringReceived,
            [](const QString value, const QString address, const quint16 port)
            {
                serverLog->info("Rx \"{}\" from server {}:{}",
                    qPrintable(value), qPrintable(address), port);
            });

        QObject::connect(&server, &net::tcp::Server::isRunningChanged,
            [](bool value) { serverLog->info("isRunning : {}", value); });
        QObject::connect(&server, &net::tcp::Server::isListeningChanged,
            [](bool value) { serverLog->info("isBounded : {}", value); });
        QObject::connect(&server, &net::tcp::Server::acceptError,
            [](int value, const QString& error)
            { serverLog->error("accept error : {}", error.toStdString()); });
        QObject::connect(&server, &net::tcp::Server::newClient,
            [](const QString& address, const quint16 port) {
                serverLog->info(
                    "New Client {}:{}", qPrintable(address), signed(port));
            });
        QObject::connect(&server, &net::tcp::Server::clientLost,
            [](const QString& address, const quint16 port)
            {
                serverLog->info("Client Disconnected {}:{}",
                    qPrintable(address), signed(port));
            });

        serverLog->info(
            "Start server on address {}:{}", qPrintable(ip), signed(port));
        // server.start(port) can be called to listen from every interfaces
        server.start(ip, port);

        appLog->info("Start application");
    }
};

static void installLoggers()
{
#ifdef _MSC_VER
    const auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    msvcSink->set_level(spdlog::level::debug);
    net::tcp::Logger::registerSink(msvcSink);
    appLog->sinks().emplace_back(msvcSink);
    serverLog->sinks().emplace_back(msvcSink);
#endif

    const auto stdoutSink =
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdoutSink->set_level(spdlog::level::debug);
    net::tcp::Logger::registerSink(stdoutSink);
    appLog->sinks().emplace_back(stdoutSink);
    serverLog->sinks().emplace_back(stdoutSink);
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
        QCoreApplication::translate("main",
            "Make the worker live in a different thread. Default false"));
    parser.addOption(multiThreadOption);

    QCommandLineOption portOption(QStringList() << "s"
                                                << "src",
        QCoreApplication::translate(
            "main", "Port for rx packet. Default \"9999\"."),
        QCoreApplication::translate("main", "port"));
    portOption.setDefaultValue("9999");
    parser.addOption(portOption);

    QCommandLineOption ipOption(QStringList() << "i"
                                              << "ip",
        QCoreApplication::translate(
            "main", "Ip address of multicast group. Default \"127.0.0.1\""),
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
