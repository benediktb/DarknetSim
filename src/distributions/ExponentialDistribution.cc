/*
 * ExponentialDistribution.cc
 */

#include "ExponentialDistribution.h"

double ExponentialDistribution::getNext() {
    return exponential(mean, randomGenerator);
}

