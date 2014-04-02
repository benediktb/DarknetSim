/*
 * IRandomDistribution.h
 */

#ifndef IRANDOMDISTRIBUTION_H_
#define IRANDOMDISTRIBUTION_H_

#include <distrib.h>

/**
 * Random distributions for churn times. ChurnController assumes result is given
 * in minutes.
 */
class IRandomDistribution {
protected:
    const int randomGenerator;
public:
    IRandomDistribution(int randomGenerator) :
            randomGenerator(randomGenerator) {
    }
    virtual ~IRandomDistribution() {
    }
    virtual double getNext() = 0;
};

#endif /* IRANDOMDISTRIBUTION_H_ */
