/*
 * PoissonDistribution.h
 */

#ifndef POISSONDISTRIBUTION_H_
#define POISSONDISTRIBUTION_H_

#include "IRandomDistribution.h"

class PoissonDistribution: public IRandomDistribution {
protected:
    double lambda;
public:
    PoissonDistribution(double lambda, int randomGenerator) :
        IRandomDistribution(randomGenerator), lambda(lambda) {
    }

    virtual double getNext();
};

#endif /* POISSONDISTRIBUTION_H_ */
