/*
 * RandomDistributionFactory.h
 */

#ifndef RANDOMDISTRIBUTIONFACTORY_H_
#define RANDOMDISTRIBUTIONFACTORY_H_

#include "IRandomDistribution.h"

class RandomDistributionFactory {
public:
    static IRandomDistribution* getDistribution(std::string distributionName,
            cModule* module, std::string parPrefix);
};

#endif /* RANDOMDISTRIBUTIONFACTORY_H_ */
