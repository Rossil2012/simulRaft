#ifndef SERVER_HH
#define SERVER_HH

#include "Package.hh"
#include "Settings.hh"

#include <QVector>
#include <QQueue>
#include <QTimer>
#include <QThread>
#include <QApplication>
#include <QDebug>

/* Class:       Server
 * Function:    Simulate a Server's Behaviors */
class Server : public QObject
{
    /** Data Structure
     * On All Servers:
     * currentTerm:             Last Term this Server has Seen
     * voteFor:                 The Voted Server in this Timeout Span
     * log[]:                   Storing the Log Entries
     * commitIndex:             Highest Index of Entries Known to be Comitted
     * lastApplied:             Highest Index of Entries Applied to the State Machine
     *
     * On Leaders:
     * nextIndex[]:             For each Server, the Index of the Next Log Entry to Send to that Server
     * matchIndex[]:            For each Server, the Highest Index of Log Entries Known have been Replicated to that Server
     *
     * Assistant Varibles
     * ID:                      The Identity Number
     * leaderID:                The Identity Number of the Leader
     * clusterSize:             The number of all the Servers in the cluster
     * status:                  The status of the server in the cluster (leader/candidate/follower)
     * electionTimeout:         Handle the Election Timeout
     * heartbeatTimeout:        Handle the Heartbeat Timeout
     * crashTimeout:            Simulate Crashes
     * recoverTimeout:          Simulate Recovery from Crashes
     *
     ** Procedures
     * receivePackage:          Distribute the Package to Specific Function According to the status
     * leaderReceivePackage:    Process the Received Package as a Leader
     * candidateReceivePackage: Process the Received Package as a Candidate
     * followerReceivePackage:  Process the Received Package as a Follower
     * leaderStepDown:          Leader Convert to Follower
     * candidateStepDown:       Candidate Convert to Follower
     * candidateExalted:        Candidate Convert to Leader
     * followerExalted:         Follower Convert to Candidate
     * generateHeartbeat:       Generate an Empty AppendEntries RPC and Return it
     * generateARPC:            Generate an AppendEntries RPC and Return it
     * generateARPC_R:          Generate a Response to the AppendEntries RPC and Return it
     * generateRRPC:            Generate an RequestVote RPC and Return it
     * generateRRPC_R:          Generate a Response to the RequestVote RPC and Return it
     * generateIRPC:            Generate an InstallSnapshot RPC and Return it
     * generateIRPC_R:          Generate a Response to the InstallSnapshot RPC and Return it
     * ElectionTimeout:         For Follower to followerExalted(), for Candidate to Enter a New Term
     * HeartbeatTimeout:        Generate Heartbeat or ARPC to all the Servers and Update the commitIndex
     * CrashTimeout:            Simulate a Crash
     * RecoverTimeout:          Simulate a Recovery from Crash


*/

    Q_OBJECT
    /* Variables */
private:
    uint64_t currentTerm;
    uint32_t voteFor;
    QVector<Entry> log;
    uint64_t commitIndex;
    uint64_t lastApplied;

    QVector<uint64_t> nextIndex;
    QVector<uint64_t> matchIndex;

    uint32_t ID;
    uint32_t leaderID;
    uint32_t clusterSize;
    uint32_t voteForMe;
    bool isCrashed;
    enum {LEADER, CANDIDATE, FOLLOWER} status;
    QTimer *electionTimeout;
    QTimer *heartbeatTimeout;
    QTimer *crashTimeout;
    QTimer *recoverTimeout;

    QThread newThread;


    /* Procedures */
private:
    void leaderReceivePackage(const uniformRPC &package);
    void candidateReceivePackage(const uniformRPC &package);
    void followerReceivePackage(const uniformRPC &package);
    void leaderStepDown();
    void candidateStepDown();
    void candidateExalted();
    void followerExalted();
    uniformRPC generateHeartbeat();
    uniformRPC generateARPC(uint64_t startIndex, int entriesLength);
    uniformRPC generateARPC_R(bool accepted, uint64_t lastGrantedIndex);
    uniformRPC generateRRPC();
    uniformRPC generateRRPC_R(bool accepted);
    uniformRPC generateIRPC();
    uniformRPC generateIRPC_R(bool accepted);

public:
    explicit Server(uint32_t id, uint32_t serverNum);
    virtual ~Server() override;
    void prepare();
    void stop();
    uint32_t getID();

private slots:
    void ElectionTimeout();
    void HeartbeatTimeout();
    void CrashTimeout();
    void RecoverTimeout();
    void initialize();
    void writeLog(Entry *entries, int entriesLength);
    void writeLog(char *command);

public slots:
    void receivePackage(const uniformRPC &package);
    void run();

signals:
    void sendCrash(uint32_t id);
    void sendRecover(uint32_t id);
    void sendPackage(const uniformRPC &package, uint32_t receiverID);
    void broadcast(const uniformRPC &package);
    void claimLeader(uint32_t id);
    void reportCommit(uint64_t commitIndex);
};

#endif // SERVER_HH
