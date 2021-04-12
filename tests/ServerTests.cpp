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
#include <QtTest/QSignalSpy>

class ServerTests : public ::testing::Test
{
public:
    MyServer server;
    MySocket client;

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
