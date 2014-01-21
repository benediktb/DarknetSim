/*
 * PoissonDistribution.cc
 */

#include "PoissonDistribution.h"

double PoissonDistribution::getNext() {
    return poisson(lambda);
}

