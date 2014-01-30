/*
 * LognormalDistribution.cc
 */

#include "LognormalDistribution.h"

double LognormalDistribution::getNext() {
    return lognormal(mean, variance);
}

