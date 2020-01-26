#ifndef SERVER_HH
#define SERVER_HH

#include "Package.hh"

#include <QVector>
#include <QTimer>

/* Class:       Server
 * Function:    Simulate the Server's Behaviors */
class Server
{
    /** Data Structure **/
private:
    /* On All Servers:
     * currentTerm:     Last Term this Server has Seen
     * voteFor:         The Voted Server in this Timeout Span
     * log[]:           Storing the Log Entries
     * commitIndex:     Highest Index of Entries Known to be Comitted
     * lastApplied:     Highest Index of Entries Applied to the State Machine */
    unsigned long long currentTerm;
    unsigned int voteFor;
    QVector<Entry> log;
    unsigned long long commitIndex;
    unsigned long long lastApplied;

    /* On Leaders:
     * nextIndex[]:     For each Server, the Index of the Next Log Entry to Send to that Server
     * matchIndex[]:    For each Server, the Highest Index of Log Entries Known have been Replicated to that Server */
    QVector<unsigned long long> nextIndex;
    QVector<unsigned long long> matchIndex;

    /* Assistant Varibles
     * ID:                      The Identity Number
     * clusterSize:             The number of all the Servers in the cluster
     * status:                  The status of the server in the cluster (leader/candidate/follower)
     * timeout & timeoutSpan:   Handle the Election Timeout */
    unsigned int ID;
    unsigned int clusterSize;
    enum {LEADER, CANDIDATE, FOLLOWER} status;
    QTimer timeout;
    unsigned int timeoutSpan;

    /** Procedure **/
private:
    /* On Leader:
     * AppendEntries RPC */
    void AppendEntries();

    /* On Candidate:
     * RequestVote RPC */
    void RequestVote();

    /* Assistant Procedures */



};

#endif // SERVER_HH
