#include "Cluster.hh"

Cluster::Cluster(uint32_t clusterSize)
    : size(clusterSize), net(clusterSize), packageSum(0), packageLoss(0), commitIndex(0), leaderID(0)
{
    connect(&net, &simulNet::send, this, &Cluster::packageDistribution);
    connect(&net, &simulNet::returnPackageInfo, this, &Cluster::getPackageInfo);
    connect(this, &Cluster::sendPackage, &net, &simulNet::sendPackage);

    servers = new Server *[clusterSize + 1];
    for (uint32_t i = 0; i <= clusterSize; ++i)
    {
        servers[i] = new Server(i, clusterSize);
        connect(servers[int(i)], &Server::sendPackage, &net, &simulNet::sendPackage);
        connect(servers[int(i)], &Server::broadcast, &net, &simulNet::broadcast);
        connect(servers[int(i)], &Server::sendCrash, this, &Cluster::receiveCrash);
        connect(servers[int(i)], &Server::sendRecover, this, &Cluster::receiveRecover);
        connect(servers[int(i)], &Server::claimLeader, this, &Cluster::leaderFound);
        connect(servers[int(i)], &Server::reportCommit, this, &Cluster::receiveCommitIndex);
    }
}

Cluster::~Cluster()
{
    for (uint32_t i = 0; i <= size; ++i)
    {
        delete servers[i];
    }
    delete servers;

    stop();
}

void Cluster::run()
{
    initialize();
}

void Cluster::stop()
{
    newThread.quit();
    newThread.wait();
}

void Cluster::getPackageInfo(uint32_t packageSum, uint32_t packageLoss, const QVector<packagePair> &failedServers)
{
    if (packageLoss)
    {
        for (auto i = failedServers.begin(); i != failedServers.end(); ++i)
        {
            emit sendPackage(i->package, i->receiverID);
        }
    }

    this->packageSum += packageSum;
    this->packageLoss += packageLoss;
}

void Cluster::receiveCrash(uint32_t id)
{
    crashedServer.append(id);
}

void Cluster::receiveRecover(uint32_t id)
{
    crashedServer.removeAll(id);
}

void Cluster::packageDistribution(const uniformRPC &package, uint32_t receiverID)
{
    connect(this, &Cluster::send, servers[int(receiverID)], &Server::receivePackage);
    emit send(package);
    disconnect(this, &Cluster::send, servers[int(receiverID)], &Server::receivePackage);
}

void Cluster::leaderFound(uint32_t id)
{
    leaderID = id;
}

void Cluster::receiveCommitIndex(uint64_t commitIndex)
{
    this->commitIndex = commitIndex;
}

void Cluster::writeEntries(const char *command)
{
    uniformRPC u_lrpc;
    u_lrpc.type = uniformRPC::LRPC;
    u_lrpc.package.lrpc.command = new char[strlen(command) + 1];
    strcpy(u_lrpc.package.lrpc.command, command);

    emit packageDistribution(u_lrpc, uint32_t(Settings::boundInt(1, int(size))));
}

void Cluster::initialize()
{
    //net.run();

    for (uint32_t i = 1; i <= size; ++i)
    {
        servers[i]->prepare();
    }

    for (uint32_t i = 1; i <= size; ++i)
    {
        connect(this, &Cluster::sendRun, servers[i], &Server::run);
        emit sendRun();
        disconnect(this, &Cluster::sendRun, servers[i], &Server::run);
    }
}
