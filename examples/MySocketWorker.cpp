#include <MySocketWorker.hpp>

void MySocketWorker::onConnected()
{
    Net::Tcp::SocketWorker::onConnected();
    waitingForData = false;
    bufferLength = 0;
}

void MySocketWorker::onDataAvailable()
{
    // Read header if not done
    if(!waitingForData)
        readHeader();

    // Otherwise read maximum number of bytes expected
    while(waitingForData && bytesAvailable())
    {
        // Read maximum data until the whole packet have been read.
        const auto bytesRead =
            read(buffer + bufferLength, expectedSize - bufferLength);
        bufferLength += std::uint8_t(bytesRead);

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

void MySocketWorker::readHeader()
{
    // Only read if something is available
    if(!bytesAvailable())
        return;

    // Only read if in correct state
    if(waitingForData)
        return;

    // Read 1 byte
    if(!read(&expectedSize, 1))
        return closeAndRestart();

    // Check header is valid
    if(expectedSize == 0 || expectedSize >= 128)
        return closeAndRestart();

    // Go to next state waiting for data
    waitingForData = true;
}

void MySocketWorker::onSendString(const QString& s)
{
    const auto data = s.toStdString();
    // Max packet size is 128
    if(data.length() >= 128)
        return;

    quint8 size = uint8_t(data.length() + 1);
    // Write header
    if(!write(&size, 1))
        return closeAndRestart();
    // Write data
    if(!write(data.c_str(), size))
        return closeAndRestart();
}
