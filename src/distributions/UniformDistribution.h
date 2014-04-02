/*
 * UniformDistribution.h
 */

#ifndef UNIFORMDISTRIBUTION_H_
#define UNIFORMDISTRIBUTION_H_

#include "IRandomDistribution.h"

class UniformDistribution: public IRandomDistribution {
protected:
    double b;
public:
    UniformDistribution(double b, int randomGenerator) :
            IRandomDistribution(randomGenerator), b(b) {
    }

    virtual double getNext();
};

#endif /* UNIFORMDISTRIBUTION_H_ */
