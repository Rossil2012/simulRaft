#ifndef SIMULNET_HH
#define SIMULNET_HH

#include "Package.hh"
#include "Settings.hh"
#include "Server.hh"

#include <QThread>
#include <QTimer>
#include <QMap>
#include <QEvent>
#include <QDebug>


/* Class:       simulNet
 * Function:    A Simulation of the Actual Network */
class simulNet : public QObject
{
    /** Variables:
     * packageQuene:        Store the Package and Destination with Corresponding timerID
     *
     ** Procedure:
     * timerEvent:          Send the Package when the Network Delay Timeouts
     * sendPackage:         Send the Package to One Specific Server in a Random Time
     * broadcast:           Send the Package to All the Servers, each in a Random Time  */
    Q_OBJECT
private:
    QMap<int, std::pair<uniformRPC, uint32_t>> packageQuene;
    uint32_t clusterSize;

    QThread newThread;
public:
    explicit simulNet(uint32_t serverNum);
    virtual void timerEvent(QTimerEvent *event) override;
    virtual ~simulNet() override;

public slots:
    void run();
    void stop();
    bool sendPackage(const uniformRPC &package, uint32_t receiverID);
    bool broadcast(const uniformRPC &package);
signals:
    void send(const uniformRPC &package, uint32_t receiverID);
    void returnPackageInfo(uint32_t packageSum, uint32_t packageLoss, const QVector<packagePair> &failedServers);
};

#endif // SIMULNET_HH
