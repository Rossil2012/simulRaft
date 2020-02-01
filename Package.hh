#ifndef PACKAGE_HH
#define PACKAGE_HH

typedef char byte;

#include <QVector>
#include <QMetaType>

/* Class Entry
 * Function:    Packing the Arguments of an Entry */
struct Entry
{
    /* Arguments
     * term:        The Leader's Term when this Entry is Created
     * command:     The Command Stored by this Entry    */
    uint64_t term;
    char *command;
};

/* Class:       AppendEntriesRPC
 * Function:    Packing the Arguments of an AppendEntries RPC */
struct AppendEntriesRPC
{
    /* Arguments
     * term:            Leader's Term (used for leader to update itself)
     * leaderID:        Leader's ID (usually used to redirect clients to the leader)
     * prevLogIndex:    Index of the Entry Immediately Preceding New Ones (used to check whether the entry can be replicated to the follower)
     * prevLogTerm:     Term of the Entry Immediately Preceding New Ones (used to check whether the entry can be replicated to the follower)
     * entries[]:       Log Entries to store (empty for heartbeat)
     * leaderCommit:    Leader's commitIndex    */
    uint64_t term;
    uint32_t leaderID;
    uint64_t prevLogIndex;
    uint64_t prevLogTerm;
    Entry *entries;
    int entriesLength;
    uint64_t leaderCommit;
};

/* Class:       AppendEntriesRPC_Response
 * Function:    Packing the Arguments of the Response of a AppendEntries RPC */
struct AppendEntriesRPC_Response
{
    /** Arguments
     * term:        currentTerm of this Server
     * success:     Whether to Accept the AppendEntries RPC (true if the leader is at least up-to-date as this server)
     *
     ** Assistant Arguments
     * responsorID:          The Identity Number of Responser
     * lastGrantedIndex:     The Index of the Last Granted Log  */
    uint64_t term;
    bool success;

    uint32_t responsorID;
    uint64_t lastGrantedIndex;
};

/* Class:       RequestVoteRPC
 * Function:    Packing the Arguments of a RequestVote RPC */
struct RequestVoteRPC
{
    /* Arguments
     * term:            Candidate's currentTerm
     * candidateID:     Candidate's ID
     * lastLogIndex:    The Last Entry's Index
     * lastLogTerm:     The Last Entry's Term   */
    uint64_t term;
    uint32_t candidateID;
    uint64_t lastLogIndex;
    uint64_t lastLogTerm;

};

/* Class:       RequestVoteRPC_Response
 * Function:    Packing the Arguments of the Response of a RequestVote RPC */
struct RequestVoteRPC_Response
{
    /** Arguments
     * term:            currentTerm of this Server (used for candidate to update for itself)
     * voteGranted:     Whether Vote for the Candidate
     *
     ** Assistant Arguments
     * responsorID:              The Identity Number of Responser   */
    uint64_t term;
    bool voteGranted;

    uint32_t responsorID;
};

/* Class:       InstallSnapshotRPC
 * Function:    Packing the Arguments of an InstallSnapshot RPC
 * Bonus for Log Compaction */
struct InstallSnapshotRPC
{
    uint64_t term;
    uint32_t leaderID;
    uint64_t lastIncludedIndex;
    uint64_t lastIncludedTerm;
    uint64_t offset;                // to be reconsidered
    byte *data;
    uint64_t dataLength;
    bool done;
};

/* Class:       InstallSnapshotRPC_Response
 * Function:    Packing the Arguments of the Response of an InstallSnapshot RPC
 * Bonus for Log Compaction */
struct InstallSnapshotRPC_Response
{
    uint64_t term;

    uint32_t responsorID;
};

/* Class:       LogAppendRPC
 * Function:    Append the Command to the Cluster and Help Client to Locate the Leader of the Cluster
 * Assistant RPC */
struct LogAppendRPC
{
    char *command;
};

/* Class:       uniformRPC
 * Function:    The Uniform RPC Packing the Package and its Type */
struct uniformRPC
{
    enum
    {
        ARPC, ARPC_R,
        RRPC, RRPC_R,
        IRPC, IRPC_R,
        LRPC
    } type;
    union
    {
        struct AppendEntriesRPC arpc;
        struct AppendEntriesRPC_Response arpc_R;
        struct RequestVoteRPC rrpc;
        struct RequestVoteRPC_Response rrpc_R;
        struct InstallSnapshotRPC irpc;
        struct InstallSnapshotRPC_Response irpc_R;
        struct LogAppendRPC lrpc;
    } package;
};

struct packagePair
{
    uniformRPC package;
    uint32_t receiverID;
};

#endif // PACKAGE_HH
