/*
 * LognormalDistribution.h
 */

#ifndef LOGNORMALDISTRIBUTION_H_
#define LOGNORMALDISTRIBUTION_H_

#include "IRandomDistribution.h"

class LognormalDistribution: public IRandomDistribution {
protected:
    double mean, variance;
public:
    LognormalDistribution(double mean, double variance, int randomGenerator) :
            IRandomDistribution(randomGenerator), mean(mean), variance(variance) {
    }

    virtual double getNext();
};

#endif /* LOGNORMALDISTRIBUTION_H_ */
