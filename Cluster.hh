#ifndef CLUSTER_HH
#define CLUSTER_HH

#include "Server.hh"
#include "simulNet.hh"
#include "Package.hh"

#include <QVector>
#include <QList>
#include <QEventLoop>

/* Class:       Server
 * Function:    Simulate a Raft Cluster's Behaviors */
class Cluster : public QObject
{
    Q_OBJECT
private:
    uint32_t size;
    Server **servers;
    simulNet net;

    QThread newThread;
public:
    explicit Cluster(uint32_t clusterSize);
    virtual ~Cluster() override;
    void run();
    void stop();

    uint64_t packageSum;
    uint64_t packageLoss;
    uint64_t commitIndex;
    uint32_t leaderID;
    QVector<uint32_t> crashedServer;

public slots:
    void getPackageInfo(uint32_t packageSum, uint32_t packageLoss, const QVector<packagePair> &failedServers);
    void receiveCrash(uint32_t id);
    void receiveRecover(uint32_t id);
    void packageDistribution(const uniformRPC &package, uint32_t receiverID);
    void leaderFound(uint32_t id);
    void receiveCommitIndex(uint64_t commitIndex);
    void writeEntries(const char *command);
    void initialize();
signals:
    bool sendPackage(const uniformRPC &package, uint32_t receiverID);
    void send(const uniformRPC &package);
    void sendRun();
};

#endif // CLUSTER_HH
