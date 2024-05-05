// MIT License
//
// Copyright(c) 2020 Olivier Le Doeuff
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <MyServer.hpp>
#include <MySocket.hpp>

#include <gtest/gtest.h>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>
#include <QtCore/QTimer>
#include <QtCore/QObject>
#include <QtCore/QDebug>

class ServerTests : public ::testing::Test
{
public:
    MyServer server;
    MySocket client;

    QTimer timer;

    void echoTest(quint16 port)
    {
        QSignalSpy connectedSpy(&client, &MySocket::isConnectedChanged);
        client.start("127.0.0.1", port);
        server.start("127.0.0.1", port);
        if(!client.isConnected())
        {
            ASSERT_TRUE(connectedSpy.wait());
        }

        QSignalSpy serverStringAvailable(&server, &MyServer::stringReceived);
        QSignalSpy clientStringAvailable(&client, &MySocket::stringReceived);
        client.sendString("My String");
        ASSERT_TRUE(clientStringAvailable.wait());
        {
            const auto s = clientStringAvailable.takeFirst().at(0).toString();
            ASSERT_EQ(s, QString("My String"));
        }
        {
            const auto s = serverStringAvailable.takeFirst().at(0).toString();
            ASSERT_EQ(s, QString("My String"));
        }
    }

    int connectedCount = 0;
    bool clientSendError = false;
    bool serverSendError = false;

    void runFuzzClientDisconnection(quint16 port)
    {
        server.sendError = serverSendError;

        QSignalSpy newClientSpy(&server, &MyServer::objectInserted);
        client.setWatchdogPeriod(10);
        // Send Echo counter every seconds
        QObject::connect(&timer, &QTimer::timeout,
            [this]()
            {
                if(client.isConnected())
                {
                    if(clientSendError)
                    {
                        qDebug() << "Send Error";
                        Q_EMIT client.sendErrorString();
                    }
                    else
                    {
                        Q_EMIT client.sendString("Hello World");
                    }
                }
            });
        QObject::connect(&client, &MySocket::isConnectedChanged,
            [this](bool value)
            {
                if(value)
                {
                    ++connectedCount;
                    qDebug() << "Client Connected on port " << client.localPort();
                }
            });
        server.onInserted([this](net::tcp::Socket* s)
            { qDebug() << "New Client connected " << s->peerAddress() << ":" << s->peerPort(); });

        // server.start(port) can be called to listen from every interfaces
        server.start("127.0.0.1", port);

        client.setWatchdogPeriod(10);
        client.start("127.0.0.1", port);

        timer.start(10);

        QTest::qWait(10000);
        timer.stop();
        // Give time to finish last connection
        QTest::qWait(1000);
        qInfo() << "Connected Count : " << connectedCount;
        ASSERT_GT(connectedCount, 40);
        ASSERT_EQ(newClientSpy.count(), connectedCount);
    }
};

TEST_F(ServerTests, echoTestMonoThread)
{
    server.setUseWorkerThread(false);
    client.setUseWorkerThread(false);
    echoTest(30000);
}

TEST_F(ServerTests, echoTestWorkerThreadClient)
{
    server.setUseWorkerThread(false);
    client.setUseWorkerThread(true);
    echoTest(30001);
}

TEST_F(ServerTests, echoTestWorkerThreadServer)
{
    server.setUseWorkerThread(true);
    client.setUseWorkerThread(false);
    echoTest(30002);
}

TEST_F(ServerTests, echoTestWorkerThreadClientServer)
{
    server.setUseWorkerThread(true);
    client.setUseWorkerThread(true);
    echoTest(30003);
}

TEST_F(ServerTests, echoTestWorkerThreadClientServerNoDelay)
{
    server.setUseWorkerThread(true);
    client.setUseWorkerThread(true);
    server.setNoDelay(true);
    client.setNoDelay(true);
    echoTest(30009);
}

TEST_F(ServerTests, echoTestWorkerThreadClientServerDelay)
{
    server.setUseWorkerThread(true);
    client.setUseWorkerThread(true);
    server.setNoDelay(false);
    client.setNoDelay(false);
    echoTest(30010);
}

TEST_F(ServerTests, fuzzDisconnectionClientServer)
{
    clientSendError = true;
    serverSendError = false;
    server.setUseWorkerThread(false);
    client.setUseWorkerThread(false);
    runFuzzClientDisconnection(30004);
}

TEST_F(ServerTests, fuzzDisconnectionClientServerWorkerThreadClient)
{
    clientSendError = true;
    serverSendError = false;
    server.setUseWorkerThread(true);
    client.setUseWorkerThread(false);
    runFuzzClientDisconnection(30004);
}

TEST_F(ServerTests, fuzzDisconnectionClientServerWorkerThreadServer)
{
    clientSendError = true;
    serverSendError = false;
    server.setUseWorkerThread(false);
    client.setUseWorkerThread(true);
    runFuzzClientDisconnection(30004);
}

TEST_F(ServerTests, fuzzDisconnectionClientServerWorkerThreadClientServer)
{
    clientSendError = true;
    serverSendError = false;
    server.setUseWorkerThread(true);
    client.setUseWorkerThread(true);
    runFuzzClientDisconnection(30004);
}

TEST_F(ServerTests, fuzzDisconnectionServerClient)
{
    clientSendError = false;
    serverSendError = true;
    server.setUseWorkerThread(false);
    client.setUseWorkerThread(false);
    runFuzzClientDisconnection(30004);
}

TEST_F(ServerTests, fuzzDisconnectionServerClientWorkerThreadClient)
{
    clientSendError = false;
    serverSendError = true;
    server.setUseWorkerThread(true);
    client.setUseWorkerThread(false);
    runFuzzClientDisconnection(30004);
}

TEST_F(ServerTests, fuzzDisconnectionServerClientWorkerThreadServer)
{
    clientSendError = false;
    serverSendError = true;
    server.setUseWorkerThread(false);
    client.setUseWorkerThread(true);
    runFuzzClientDisconnection(30004);
}

TEST_F(ServerTests, fuzzDisconnectionServerClientWorkerThreadClientServer)
{
    clientSendError = false;
    serverSendError = true;
    server.setUseWorkerThread(true);
    client.setUseWorkerThread(true);
    runFuzzClientDisconnection(30004);
}
