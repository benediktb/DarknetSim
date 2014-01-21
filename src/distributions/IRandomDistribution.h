/*
 * IRandomDistribution.h
 */

#ifndef IRANDOMDISTRIBUTION_H_
#define IRANDOMDISTRIBUTION_H_

#include <distrib.h>

class IRandomDistribution {
public:
    virtual double getNext() = 0;
};

#endif /* IRANDOMDISTRIBUTION_H_ */
