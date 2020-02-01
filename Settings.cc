#include "Settings.hh"

double Settings::packageLossRate = 0.5 / 100.0;
double Settings::aveServerCrashInterval = 10.0 * 60 * 1000;
double Settings::aveServerRecoverInterval = 0.5 * 60 * 1000;
double Settings::broadcastDelay_low = 1;
double Settings::broadcastDelay_high = 20;
double Settings::serverTimeout_low = 400;
double Settings::serverTimeout_high = 1000;
int Settings::leaderHeartbeatSpan = 30;

double Settings::normalRandom_95(double low, double high)
{
    double mu = 0;
    double sigma = 1;

    if (high > low)
    {
        mu = (high + low) / 2;
        sigma = (high - low) / 4;
    }

    std::normal_distribution<double> norm(mu, sigma);
    std::random_device seed;
    std::default_random_engine engine(seed());

    return norm(engine);
}

double Settings::normalRandom_68(double low, double high)
{
    double mu = (high + low) / 2;
    double sigma = (high - low) / 2;
    std::normal_distribution<double> norm(mu, sigma);
    std::random_device seed;
    std::default_random_engine engine(seed());

    return norm(engine);
}

double Settings::exponentialRandom(double lambda)
{
    std::exponential_distribution<double> exp(lambda);
    std::random_device seed;
    std::default_random_engine engine(seed());

    return exp(engine);
}

double Settings::uniformRandom(double low, double high)
{
    std::uniform_real_distribution<double> uniform(low, high);
    std::random_device seed;
    std::default_random_engine engine(seed());

    return uniform(engine);
}

bool Settings::trueORfalse(double probablity)
{
    if (probablity >= 0 && probablity <= 1)
    {
        std::uniform_real_distribution<double> uniform(0.0, 1.0);
        std::random_device seed;
        std::default_random_engine engine(seed());

        return uniform(engine) <= probablity;
    }

    return true;
}

int Settings::boundInt(int low, int high)
{
    std::uniform_int_distribution<int> uniform(low - 1, high);
    std::random_device seed;
    std::default_random_engine engine(seed());

    return uniform(engine);
}

packagePair Settings::makePair(const uniformRPC &package, uint32_t receiverID)
{
    packagePair tmp;
    tmp.package = package;
    tmp.receiverID = receiverID;

    return tmp;
}
