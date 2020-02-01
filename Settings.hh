#ifndef SETTINGS_HH
#define SETTINGS_HH

/* Debug Switch */
#define DEBUG
//#define DETAILED_DEBUG

#include <QVector>
#include <QMap>
#include <random>

#include "Package.hh"

/* Class:       Settings
 * Function:    Store Global Settings and Provide General Functions */
class Settings
{
    /** Variables:
     * packageLossRate:             The simulNet's Loss Rate of Package
     * aveServerCrashInterval:      The Expectation of the Interval of a Server Crashes (unit: ms)
     * aveServerRecoverInterval:    The Expectation of the Interval of a Server Recovers from Crashes (unit: ms)
     * broadcastDelay:              The Delay of simulNet which Conform to the Normal Distribution
     *                              with 95% Confidence Interval set between broadcastDelay_low and broadcastDelay_high
     * serverTimeout:               The Election Timeout of a Server which Conform to the Uniform Distribution
     *                              with Interval between serverTimeout_low and serverTimeout_high
     * leaderHeartbeatSpan:         The Fixed Time to Generate Heartbeat or ARPC to all the Servers and to Update the commitIndex
     *
     ** Procedures:
     * normalRandom_95:             Generate a Real Number which Conform to the Normal Distribution with Confidence Coefficient Set to 95%
     * normalRandom_68:             Generate a Real Number which Conform to the Normal Distribution with Confidence Coefficient Set to 68%
     * exponentialRandom:           Generate a Real Number which Conform to the Exponential Distribution with set Lambda
     * uniformRandom:               Generate a Real Number which Conform to the Uniform Distribution between low and high
     * trueORfalse:                 Return True at the set Probalility, or Return False Otherwise
     * boundInt:                    Return a Integer which Conform to the Uniform Distribution between low and high -> [low, high]
     * makePair:                    Make a packagePair with Given package and receiverID    */
public:
    static double packageLossRate;
    static double aveServerCrashInterval;
    static double aveServerRecoverInterval;
    static double broadcastDelay_low;
    static double broadcastDelay_high;
    static double serverTimeout_low;
    static double serverTimeout_high;
    static int leaderHeartbeatSpan;
    //static QVector<QMap<uint32_t, uint32_t>> blockedDataLink;

    static double normalRandom_95(double low, double high);
    static double normalRandom_68(double low, double high);
    static double exponentialRandom(double lambda);
    static double uniformRandom(double low, double high);
    static bool trueORfalse(double probablity);
    static int boundInt(int low, int high);
    static packagePair makePair(const uniformRPC &package, uint32_t receiverID);
};

#endif // SETTINGS_HH
