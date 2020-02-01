#include "Server.hh"

/* Function: Get the Term from the Package */
inline uint64_t getPackageTerm(const uniformRPC &package)
{
    uint64_t term;
    switch (package.type)
    {
        case uniformRPC::ARPC:
        {
            term = package.package.arpc.term;
            break;
        }
        case uniformRPC::ARPC_R:
        {
            term = package.package.arpc_R.term;
            break;
        }
        case uniformRPC::RRPC:
        {
            term = package.package.rrpc.term;
            break;
        }
        case uniformRPC::RRPC_R:
        {
            term = package.package.rrpc_R.term;
            break;
        }
        case uniformRPC::IRPC:
        {
            term = package.package.irpc.term;
            break;
        }
        case uniformRPC::IRPC_R:
        {
            term = package.package.irpc_R.term;
            break;
        }
        case uniformRPC::LRPC:
        {
            term = 0;
        }
    }

    return term;
}

Server::Server(uint32_t id, uint32_t serverNum)
    : currentTerm(0), voteFor(0), commitIndex(0), lastApplied(0),
      ID(id), leaderID(0), clusterSize(serverNum), voteForMe(0), isCrashed(false), status(FOLLOWER)
{

}

Server::~Server()
{
    for (auto i = log.begin(); i != log.end(); ++i)
    {
        delete i->command;
    }
    stop();
}

void Server::prepare()
{
    moveToThread(&newThread);
    connect(&newThread, &QThread::started, this, &Server::initialize);
    newThread.start();
}

void Server::run()
{
    electionTimeout->start(int(Settings::uniformRandom(Settings::serverTimeout_low, Settings::serverTimeout_high)));
    crashTimeout->start(int(Settings::exponentialRandom(1 / Settings::aveServerCrashInterval)));
}

void Server::stop()
{
    newThread.quit();
    newThread.wait();
}

uint32_t Server::getID()
{
    return ID;
}

void Server::ElectionTimeout()
{
    /** Work Flow:
     * (My Status)? ---- Candidate: Increment currentTerm and Vote for Self and Reset electionTimeout and Broadcast RRPC
     * ----------------- Follower: followerExalted()
     * ----------------- Leader: No Way */

    if (status == CANDIDATE)
    {
        ++currentTerm;
        voteForMe = 1;
        electionTimeout->start(int(Settings::uniformRandom(Settings::serverTimeout_low, Settings::serverTimeout_high)));
        emit broadcast(generateRRPC());
    }
    else if (status == FOLLOWER)
    {
        followerExalted();
    }
    else
    {
        // never reach here
    #ifdef DEBUG
        qDebug() << "ElectionTimeout(): leader" << ID << "election timeout (should never reach here)";
    #endif
    }
}

void Server::HeartbeatTimeout()
{
    if (status == LEADER)
    {
        uint64_t min = uint64_t(log.length() - 1);
        for (uint32_t i = 1; i <= clusterSize; ++i)
        {
            if (i == ID)
            {
                continue;
            }

            min = (matchIndex.at(int(i)) < min)?matchIndex.at(int(i)):min;

            if (nextIndex.at(int(i)) < uint64_t(log.length()))
            {
                emit sendPackage(generateARPC(nextIndex.at(int(i)), 1), i);
            }
            else if (nextIndex.at(int(i)) == uint64_t(log.length()))
            {
                emit sendPackage(generateHeartbeat(), i);
            }
            else
            {
                // never reach here
            #ifdef DEBUG
                qDebug() << "HeartbeatTimeout(): nextIndex " << i << " > log's length" << log.length() << "(should never reach here)";
            #endif
            }
        }

        if (log.length() != 0)
        {
            commitIndex = min;
            uint64_t count = 0;
            for (uint64_t i = uint64_t(log.length()) - 1; i > min; --i)
            {
                for (uint32_t j = 1; j <= clusterSize; ++j)
                {
                    if (matchIndex[int(j)] == i)
                    {
                        ++count;
                    }
                }
                if (count > clusterSize / 2)
                {
                    commitIndex = i;
                #ifdef DEBUG
                #ifdef DETAILED_DEBUG
                    qDebug() << "commitIndex:" << commitIndex;
                #endif
                #endif
                }
            }
        }

        if (heartbeatTimeout->interval() == Settings::leaderHeartbeatSpan && commitIndex >= uint64_t(log.length()) / 4 * 3)
        {
            heartbeatTimeout->start(Settings::leaderHeartbeatSpan);
        }
        emit reportCommit(commitIndex);

    }
    else if (status == CANDIDATE)
    {
        // never reach here
    #ifdef DEBUG
        qDebug() << "HeartbeatTimeout(): candidate" << ID << "heartbeat timeout (should never reach here)";
    #endif
    }
    else if (status == FOLLOWER)
    {
        // never reach here
    #ifdef DEBUG
        qDebug() << "HeartbeatTimeout(): follower " << ID << "heartbeat timeout (should never reach here)";
    #endif
    }
}

/* Lose All the Local Data */
void Server::CrashTimeout()
{

#ifdef DEBUG
    if (status == LEADER)
    {
        qDebug() << "Leader" << ID << "Crash !";
    }
    else
    {
        qDebug() << ID << "Crash";
    }
#endif

    isCrashed = true;

    recoverTimeout->stop();
    recoverTimeout->start(int(Settings::exponentialRandom(1 / Settings::aveServerRecoverInterval)));
    crashTimeout->stop();

    electionTimeout->stop();
    heartbeatTimeout->stop();
    disconnect(electionTimeout, &QTimer::timeout, this, &Server::ElectionTimeout);
    disconnect(heartbeatTimeout, &QTimer::timeout, this, &Server::HeartbeatTimeout);

    log.clear();
    nextIndex.clear();
    matchIndex.clear();

    emit sendCrash(ID);
}

void Server::RecoverTimeout()
{
#ifdef DEBUG
    qDebug() << ID << "Recover";
#endif

    status = FOLLOWER;

    isCrashed = false;

    commitIndex = 0;
    voteFor = 0;
    voteForMe = 0;
    currentTerm = 0;

    Entry nullEntry;
    nullEntry.term = 0;
    nullEntry.command = nullptr;
    log.append(nullEntry);

    for (int i = 0; i <= int(clusterSize); ++i)
    {
        nextIndex.append(1);
        matchIndex.append(0);
    }

    crashTimeout->stop();
    crashTimeout->start(int(Settings::exponentialRandom(1 / Settings::aveServerCrashInterval)));
    recoverTimeout->stop();

    electionTimeout->start(int(Settings::uniformRandom(Settings::serverTimeout_low, Settings::serverTimeout_high)));
    heartbeatTimeout->stop();
    connect(electionTimeout, &QTimer::timeout, this, &Server::ElectionTimeout);
    connect(heartbeatTimeout, &QTimer::timeout, this, &Server::HeartbeatTimeout);

    emit sendRecover(ID);
}

void Server::initialize()
{
    electionTimeout = new QTimer;
    heartbeatTimeout = new QTimer;
    crashTimeout = new QTimer;
    recoverTimeout = new QTimer;

    connect(electionTimeout, &QTimer::timeout, this, &Server::ElectionTimeout);
    connect(heartbeatTimeout, &QTimer::timeout, this, &Server::HeartbeatTimeout);
    connect(crashTimeout, &QTimer::timeout, this, &Server::CrashTimeout);
    connect(recoverTimeout, &QTimer::timeout, this, &Server::RecoverTimeout);

    Entry nullEntry;
    nullEntry.term = 0;
    nullEntry.command = nullptr;
    log.append(nullEntry);

    for (int i = 0; i <= int(clusterSize); ++i)
    {
        nextIndex.append(1);
        matchIndex.append(0);
    }

}

void Server::writeLog(Entry *entries, int entriesLength)
{
    for (int i = 0; i < entriesLength; ++i)
    {
        Entry tmp;
        tmp.command = new char[strlen(entries[i].command) + 1];
        strcpy(tmp.command, entries[i].command);
        delete entries[i].command;
        tmp.term = currentTerm;
        log.append(tmp);
    }
}

void Server::writeLog(char *command)
{
    Entry tmp;
    tmp.command = new char[strlen(command) + 1];
    strcpy(tmp.command, command);
    delete command;
    tmp.term = currentTerm;
    log.append(tmp);
}

void Server::receivePackage(const uniformRPC &package)
{
    if (isCrashed)
    {
        return;
    }

    if (status == LEADER)
    {
        leaderReceivePackage(package);
    }
    else if (status == CANDIDATE)
    {
        candidateReceivePackage(package);
    }
    else if (status == FOLLOWER)
    {
        followerReceivePackage(package);
    }
}

void Server::leaderReceivePackage(const uniformRPC &package)
{
    /** Work Flow:
     * (I'm a Qualified Leader)? ---- false: step down to follower and change currentTerm
     * ------------------------------ true: (ARPC Accepted)? ---- true: update the nextIndex[] and matchIndex[]
     *                                      --------------------- false: --nextIndex[] and retry   */

    if (package.type == uniformRPC::LRPC)
    {
        writeLog(package.package.lrpc.command);
        HeartbeatTimeout();

        emit claimLeader(ID);
        return;
    }

    uint64_t term = getPackageTerm(package);
    if (currentTerm < term)
    {
    #ifdef DEBUG
        if (package.type == uniformRPC::RRPC)
        {
            qDebug() << "Leader" << ID << "at term" << currentTerm << "Step Down Receive RRPC from" << package.package.rrpc.candidateID << "at Term" << term;
        }
        else if (package.type == uniformRPC::RRPC_R)
        {
            qDebug() << "Leader" << ID << "at term" << currentTerm << "Step Down Receive RRPC_R from" << package.package.rrpc_R.responsorID << "at Term" << term;
        }
        else if (package.type == uniformRPC::ARPC)
        {
            qDebug() << "Leader" << ID << "at term" << currentTerm << "Step Down Receive ARPC from" << package.package.arpc.leaderID << "at Term" << term;
        }
        else if (package.type == uniformRPC::ARPC_R)
        {
            qDebug() << "Leader" << ID << "at term" << currentTerm << "Step Down Receive ARPC_R from" << package.package.arpc_R.responsorID << "at Term" << term;
        }
    #endif

        currentTerm = term;
        leaderStepDown();
        return;
    }

    // Free the Memory
    if (package.type == uniformRPC::ARPC)
    {
        const AppendEntriesRPC &p = package.package.arpc;
        for (int i = 0; i < p.entriesLength; ++i)
        {
            delete p.entries[i].command;
        }
        delete package.package.arpc.entries;
    }

    if (package.type == uniformRPC::ARPC_R)
    {
        const AppendEntriesRPC_Response &p = package.package.arpc_R;
        if (!p.success)
        {
            if (currentTerm < p.term)
            {
            #ifdef DEBUG
                qDebug() << "\"Leader\"" << ID <<  "at Term" << currentTerm << "Step Down Receiving ARPC_R from" << p.responsorID << "at Term" << p.term;
            #endif
                leaderStepDown();
            }
            else
            {
                if (nextIndex[int(p.responsorID)] > 1)
                {
                    --nextIndex[int(p.responsorID)];
                }
                matchIndex[int(p.responsorID)] = 0;
                emit sendPackage(generateARPC(nextIndex[int(p.responsorID)], 1), p.responsorID);
            }
        }
        else
        {
            if (p.lastGrantedIndex == nextIndex[int(p.responsorID)])
            {
                matchIndex[int(p.responsorID)] = nextIndex[int(p.responsorID)];
                ++nextIndex[int(p.responsorID)];
            }
        }
    }
}
void Server::candidateReceivePackage(const uniformRPC &package)
{
    /** Work Flow:
     * (I'm a Advanced Candidate)? ---- false: convert to follower and change currentTerm
     * -------------------------------- true: (I'm Acknowledegd)? ---- true: convert to follower
     *                                        ------------------------ false: wait for more votes or election timeout or other winner */

    if (package.type == uniformRPC::LRPC)
    {
        if (leaderID == 0)
        {
            emit sendPackage(package, uint32_t(Settings::boundInt(1, int(clusterSize))));
        }
        else
        {
            emit sendPackage(package, leaderID);
        }
        return;
    }

    uint64_t term = getPackageTerm(package);
    if (currentTerm < term)
    {
    #ifdef DEBUG
    #ifdef DETAILED_DEBUG
        if (package.type == uniformRPC::RRPC)
        {
            qDebug() << "Candidate" << ID << "at Term" << currentTerm << "Step Down Receiving RRPC from" << package.package.rrpc.candidateID << "at term" << term;
        }
        else if (package.type == uniformRPC::RRPC_R)
        {
            qDebug() << "Candidate" << ID << "at Term" << currentTerm << "Step Down Receiving RRPC_R from" << package.package.rrpc_R.responsorID << "at term" << term;
        }
        else if (package.type == uniformRPC::ARPC)
        {
            qDebug() << "Candidate" << ID << "at Term" << currentTerm << "Step Down Receiving ARPC from" << package.package.arpc.leaderID << "at term" << term;
        }
        else if (package.type == uniformRPC::ARPC_R)
        {
            qDebug() << "Candidate" << ID << "at Term" << currentTerm << "Step Down Receiving ARPC_R from" << package.package.arpc_R.responsorID << "at term" << term;
        }
    #endif
    #endif

        currentTerm = term;
        candidateStepDown();



        return;
    }

    if (package.type == uniformRPC::RRPC_R)
    {
        const RequestVoteRPC_Response &p = package.package.rrpc_R;
        if (p.voteGranted)
        {
            ++voteForMe;

            if (voteForMe > clusterSize / 2)
            {
                candidateExalted();
            }
        }
    }
    // If a Server Claims to be Leader, Step Down if it's Term >= currentTerm
    // read package???
    else if (package.type == uniformRPC::ARPC)
    {
        if (currentTerm <= term)
        {
            currentTerm = term;

        #ifdef DEBUG
        #ifdef DETAILED_DEBUG
            qDebug() << "Candidate" << ID << "at Term" << currentTerm << "Step Down Receiving ARPC from" << package.package.arpc.leaderID << "at term" << term;
        #endif
        #endif

            candidateStepDown();
        }

        // Free the Memory
        AppendEntriesRPC p = package.package.arpc;
        for (int i = 0; i < p.entriesLength; ++i)
        {
            delete p.entries[i].command;
        }
        delete package.package.arpc.entries;
    }

#ifdef DEBUG
#ifdef DETAILED_DEBUG
    if (package.type == uniformRPC::RRPC)
    {
        qDebug() << "Candidate" << ID << "Receive RRPC from Candidate" << package.package.rrpc.candidateID;
    }
#endif
#endif

}
void Server::followerReceivePackage(const uniformRPC &package)
{
    /** Work Flow:
     * (From Advanced Server)? ---- false: reply false
     * ---------------------------- true: (Package's Type)? ---- RRPC: (Have Voted to Others)? ---- true: reply false
     *                                                                 ---------------------------- false: check which's more up-to-date
     *                                    ---------------------- ARPC: (Contain prevLog)? ---- false: reply false
     *                                                                 ----------------------- true: (Confilcting Entries)? ---- true: delete following entries
     *                                                                                               ------------------------ false: append any new entries */

    if (package.type == uniformRPC::LRPC)
    {
        if (leaderID == 0)
        {
            emit sendPackage(package, uint32_t(Settings::boundInt(1, int(clusterSize))));
        }
        else
        {
            emit sendPackage(package, leaderID);
        }
        return;
    }

    uint64_t term = getPackageTerm(package);

    if (package.type == uniformRPC::RRPC)
    {
        const RequestVoteRPC &p = package.package.rrpc;
        if (term < currentTerm)
        {
            emit sendPackage(generateRRPC_R(false), p.candidateID);
            return;
        }
        // Enter a New Term's Election
        else if (term > currentTerm)
        {
            currentTerm = term;
            voteFor = 0;
        }

        // Restart the Timer
        electionTimeout->start(int(Settings::uniformRandom(Settings::serverTimeout_low, Settings::serverTimeout_high)));

        if (voteFor != 0 || p.lastLogTerm < log.last().term || (p.lastLogTerm == log.last().term && p.lastLogIndex < uint64_t(log.length() - 1)))
        {
            emit sendPackage(generateRRPC_R(false), p.candidateID);
            return;
        }
        else
        {
            voteFor = p.candidateID;
            emit sendPackage(generateRRPC_R(true), p.candidateID);
        }
    }
    else if (package.type == uniformRPC::ARPC)
    {
        const AppendEntriesRPC &p = package.package.arpc;
        if (term < currentTerm)
        {
            emit sendPackage(generateARPC_R(false, 0), p.leaderID);

            // Free the Memory
            AppendEntriesRPC p = package.package.arpc;
            for (int i = 0; i < p.entriesLength; ++i)
            {
                delete p.entries[i].command;
            }
            delete package.package.arpc.entries;
            return;
        }

        // Validate Leader Exists, Stopping to Vote and Restart the Timer
        currentTerm = term;
        voteFor = p.leaderID;
        leaderID = p.leaderID;

        electionTimeout->start(int(Settings::uniformRandom(Settings::serverTimeout_low, Settings::serverTimeout_high)));

        if (commitIndex < p.leaderCommit)
        {
            commitIndex = (p.leaderCommit < p.prevLogIndex + uint64_t(p.entriesLength))?p.leaderCommit:p.prevLogIndex + uint64_t(p.entriesLength);
        }

        if (p.prevLogIndex < uint64_t(log.length()))
        {
            if (p.prevLogTerm == log.at(int(p.prevLogIndex)).term)
            {
                log.remove(int(p.prevLogIndex + 1), log.length() - int(p.prevLogIndex + 1));
                writeLog(p.entries, p.entriesLength);

                emit sendPackage(generateARPC_R(true, p.prevLogIndex + uint64_t(p.entriesLength)), p.leaderID);
            }
            else
            {
                log.remove(int(p.prevLogIndex), log.length() - int(p.prevLogIndex));
                emit sendPackage(generateARPC_R(false, 0), p.leaderID);
            }
        }
        else
        {
            emit sendPackage(generateARPC_R(false, 0), p.leaderID);
        }

        // Free the Memory
        delete p.entries;
    }
}

void Server::leaderStepDown()
{
    status = FOLLOWER;

    heartbeatTimeout->stop();
    electionTimeout->start(int(Settings::uniformRandom(Settings::serverTimeout_low, Settings::serverTimeout_high)));
}

void Server::candidateStepDown()
{
    status = FOLLOWER;

    electionTimeout->start(int(Settings::uniformRandom(Settings::serverTimeout_low, Settings::serverTimeout_high)));
}

void Server::candidateExalted()
{
    status = LEADER;

    emit claimLeader(ID);

#ifdef DEBUG
    qDebug() << "New Leader" << ID  <<  ";" << voteForMe << "Vote for Me;" << "Term:" << currentTerm;
#endif

    for (uint32_t i = 1; i <= clusterSize; ++i)
    {
        nextIndex[int(i)] = uint64_t(log.length());
        matchIndex[int(i)] = 0;
    }

    electionTimeout->stop();
    emit broadcast(generateHeartbeat());
    heartbeatTimeout->stop();
    heartbeatTimeout->start(3); //???
    leaderID = ID;
}

void Server::followerExalted()
{
#ifdef DEBUG
#ifdef DETAILED_DEBUG
    qDebug() << "Follower " << ID << " Exalted";
#endif
#endif

    status = CANDIDATE;

    ++currentTerm;
    voteForMe = 1;
    electionTimeout->start(int(Settings::uniformRandom(Settings::serverTimeout_low, Settings::serverTimeout_high)));
    emit broadcast(generateRRPC());
}

uniformRPC Server::generateHeartbeat()
{
    AppendEntriesRPC atmp;
    atmp.term = currentTerm;
    atmp.leaderID = ID;
    atmp.prevLogIndex = uint64_t(log.length()) - 1;
    atmp.prevLogTerm = log.last().term;
    atmp.entries = nullptr;
    atmp.entriesLength = 0;
    atmp.leaderCommit = commitIndex;

    uniformRPC tmp;
    tmp.type = uniformRPC::ARPC;
    tmp.package.arpc = atmp;

    return tmp;
}

uniformRPC Server::generateARPC(uint64_t startIndex, int entriesLength)
{
    AppendEntriesRPC atmp;
    atmp.term = currentTerm;
    atmp.leaderID = ID;
    atmp.prevLogIndex = startIndex - 1;
    atmp.prevLogTerm = log.at(int(startIndex - 1)).term;
    atmp.entries = new Entry[entriesLength];
    for (int i = 0; i < entriesLength; ++i)
    {
        atmp.entries[i].term = (log.data() + startIndex + i)->term;

        atmp.entries[i].command = new char[strlen((log.data() + startIndex + i)->command) + 1];
        strcpy(atmp.entries[i].command, (log.data() + startIndex + i)->command);
    }
    atmp.entriesLength = entriesLength;
    atmp.leaderCommit = commitIndex;

    uniformRPC tmp;
    tmp.type = uniformRPC::ARPC;
    tmp.package.arpc = atmp;

    return tmp;
}

uniformRPC Server::generateARPC_R(bool accepted, uint64_t lastGrantedIndex)
{
    AppendEntriesRPC_Response artmp;
    artmp.term = currentTerm;
    artmp.success = accepted;
    artmp.responsorID = ID;
    artmp.lastGrantedIndex = lastGrantedIndex;

    uniformRPC tmp;
    tmp.type = uniformRPC::ARPC_R;
    tmp.package.arpc_R = artmp;

    return tmp;
}

uniformRPC Server::generateRRPC()
{
    RequestVoteRPC rtmp;
    rtmp.term = currentTerm;
    rtmp.candidateID = ID;
    rtmp.lastLogIndex = uint64_t(log.length()) - 1;
    rtmp.lastLogTerm = log.last().term;

    uniformRPC tmp;
    tmp.type = uniformRPC::RRPC;
    tmp.package.rrpc = rtmp;

    return tmp;
}

uniformRPC Server::generateRRPC_R(bool accepted)
{
    RequestVoteRPC_Response rrtmp;
    rrtmp.term = currentTerm;
    rrtmp.voteGranted = accepted;
    rrtmp.responsorID = ID;

    uniformRPC tmp;
    tmp.type = uniformRPC::RRPC_R;
    tmp.package.rrpc_R = rrtmp;

    return tmp;
}

uniformRPC Server::generateIRPC()
{
    return uniformRPC();
}

uniformRPC Server::generateIRPC_R(bool accepted)
{
    accepted = true;
    return uniformRPC();
}
