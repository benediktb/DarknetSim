/*
 * WeibullDistribution.h
 */

#ifndef WEIBULLDISTRIBUTION_H_
#define WEIBULLDISTRIBUTION_H_

#include "IRandomDistribution.h"

class WeibullDistribution: public IRandomDistribution {
protected:
    double a, b;
public:
    WeibullDistribution(double a, double b, int randomGenerator) :
            IRandomDistribution(randomGenerator), a(a), b(b) {
    }

    virtual double getNext();
};

#endif /* WEIBULLDISTRIBUTION_H_ */
