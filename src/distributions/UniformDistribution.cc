/*
 * UniformDistribution.cc
 */

#include "UniformDistribution.h"

double UniformDistribution::getNext() {
    return uniform(0, b);
}

