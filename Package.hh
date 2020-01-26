#ifndef PACKAGE_HH
#define PACKAGE_HH

#include <QVector>

/* Class Entry
 * Function:    Packing the Arguments of an Entry */
struct Entry
{

};

/* Class:       AppendEntriesRPC
 * Function:    Packing the Arguments of an AppendEntries RPC */
struct AppendEntriesRPC
{
    /* Arguments:
     * term:            Leader's Term (used for leader to update itself)
     * leaderID:        Leader's ID (usually used to redirect clients to the leader)
     * prevLogIndex:    Index of the Entry Immediately Preceding New Ones (used to check whether the entry can be replicated to the follower)
     * prevLogTerm:     Term of the Entry Immediately Preceding New Ones (used to check whether the entry can be replicated to the follower)
     * entries[]:       Log Entries to store (empty for heartbeat)
     * leaderCommit:    Leader's commitIndex    */
    unsigned long long term;
    unsigned int leaderID;
    unsigned long long prevLogIndex;
    unsigned long long prevLogTerm;
    QVector<void> entries;                  //??????????????????
    unsigned long long leaderCommit;
};

/* Class:       AppendEntriesRPC_Response
 * Function:    Packing the Arguments of the Response of a AppendEntries RPC */
struct AppendEntriesRPC_Response
{
    /* Arguments:
     * term:        currentTerm of this Server
     * success:     Whether to Accept the AppendEntries RPC (true if the leader is at least up-to-date as this server) */
    unsigned long long term;
    bool success;
};

/* Class:       RequestVoteRPC
 * Function:    Packing the Arguments of a RequestVote RPC */
struct RequestVoteRPC
{
    /* Arguments:
     * term:            Candidate's currentTerm
     * candidateID:     Candidate's ID
     * lastLogIndex:    The Last Entry's Index
     * lastLogTerm:     The Last Entry's Term   */
    unsigned long long term;
    unsigned int candidateID;
    unsigned long long lastLogIndex;
    unsigned long long lastLogTerm;

};

/* Class:       RequestVoteRPC_Response
 * Function:    Packing the Arguments of the Response of a RequestVote RPC */
struct RequestVoteRPC_Response
{
    /* Arguments:
     * term:            currentTerm of this Server (used for candidate to update for itself)
     * voteGranted:     Whether Vote for the Candidate  */
    unsigned long long term;
    bool voteGranted;
};

/* Class:       InstallSnapshotRPC
 * Function:    Packing the Arguments of an InstallSnapshot RPC
 * Bonus for Log Compaction */
struct InstallSnapshotRPC
{

};

/* Class:       InstallSnapshotRPC_Response
 * Function:    Packing the Arguments of the Response of an InstallSnapshot RPC
 * Bonus for Log Compaction */
struct InstallSnapshotRPC_Response
{

};

#endif // PACKAGE_HH
