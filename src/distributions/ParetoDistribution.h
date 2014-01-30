/*
 * ParetoDistribution.h
 */

#ifndef PARETODISTRIBUTION_H_
#define PARETODISTRIBUTION_H_

#include "IRandomDistribution.h"

class ParetoDistribution: public IRandomDistribution {
protected:
    double a, b, c;
public:
    ParetoDistribution(double a, double b, double c): a(a), b(b), c(c) {}

    virtual double getNext();
};

#endif /* PARETODISTRIBUTION_H_ */
