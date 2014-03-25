/*
 * ExponentialDistribution.h
 */

#ifndef EXPONENTIALDISTRIBUTION_H_
#define EXPONENTIALDISTRIBUTION_H_

#include "IRandomDistribution.h"

class ExponentialDistribution: public IRandomDistribution {
protected:
    double mean;
public:
    ExponentialDistribution(double mean) :
            mean(mean) {
    }

    virtual double getNext();
};

#endif /* EXPONENTIALDISTRIBUTION_H_ */
