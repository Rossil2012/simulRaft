#include "simulNet.hh"

simulNet::simulNet(uint32_t serverNum)
    : clusterSize(serverNum)
{
}

simulNet::~simulNet()
{
    stop();
}

void simulNet::timerEvent(QTimerEvent *event)
{
    auto ite = packageQuene.find(event->timerId());
    if (ite != packageQuene.end())
    {
        killTimer(ite.key());
        emit send(ite.value().first, ite.value().second);
        packageQuene.remove(ite.key());
    }

    return QObject::timerEvent(event);
}

void simulNet::run()
{
    moveToThread(&newThread);
    newThread.start();
}

void simulNet::stop()
{
    newThread.quit();
    newThread.wait();
}

bool simulNet::sendPackage(const uniformRPC &package, uint32_t receiverID)
{
    if (Settings::trueORfalse(Settings::packageLossRate))
    {
        QVector<packagePair> failedServers;
        failedServers.append(Settings::makePair(package, receiverID));
        emit returnPackageInfo(1, 1, failedServers);
        return false;
    }
    else
    {
        int timerID = startTimer(abs(int(Settings::normalRandom_95(Settings::broadcastDelay_low, Settings::broadcastDelay_high))));
        packageQuene.insert(timerID, std::make_pair(package, receiverID));
        emit returnPackageInfo(1, 0, QVector<packagePair>());
        return true;
    }
}

bool simulNet::broadcast(const uniformRPC &package)
{

    uint32_t from = dynamic_cast<Server *>(sender())->getID();

    bool success = true;
    uint32_t sum = 0;
    uint32_t loss = 0;
    QVector<packagePair> failedServers;
    for (uint32_t i = 1; i <= clusterSize; ++i)
    {
        if (i == from)
        {
            continue;
        }

        ++sum;
        if (!sendPackage(package, i))
        {
            success = false;
            failedServers.append(Settings::makePair(package, i));
            ++loss;
        }
    }

    emit returnPackageInfo(sum, loss, failedServers);

    return success;
}
